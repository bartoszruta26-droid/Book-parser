#!/usr/bin/env python3
"""
Book Parser WebUI Backend Server
Flask API server for managing the book parsing pipeline
"""

import os
import json
import subprocess
import threading
import time
from datetime import datetime
from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS
import logging

# Konfiguracja
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LOGS_DIR = os.path.join(SCRIPT_DIR, 'logs')
STATE_FILE = os.path.join(SCRIPT_DIR, '.pipeline_state.json')

# Upewnij się, że katalogi istnieją
os.makedirs(LOGS_DIR, exist_ok=True)
for dir_name in ['input', 'chunk', 'rewriten', 'finish', 'epub_input', 'converted']:
    os.makedirs(os.path.join(SCRIPT_DIR, dir_name), exist_ok=True)

# Konfiguracja logowania
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

app = Flask(__name__, static_folder=SCRIPT_DIR)
CORS(app)

# Globalny stan pipeline
pipeline_state = {
    'running': False,
    'progress': 0,
    'status': 'idle',
    'pending': 0,
    'processing': 0,
    'completed': 0,
    'errors': 0,
    'files': [],
    'logs': [],
    'config': {}
}

state_lock = threading.Lock()
pipeline_thread = None


def load_state():
    """Załaduj stan z pliku"""
    global pipeline_state
    if os.path.exists(STATE_FILE):
        try:
            with open(STATE_FILE, 'r') as f:
                saved_state = json.load(f)
                pipeline_state.update(saved_state)
        except Exception as e:
            logger.error(f"Błąd ładowania stanu: {e}")


def save_state():
    """Zapisz stan do pliku"""
    with state_lock:
        try:
            with open(STATE_FILE, 'w') as f:
                json.dump(pipeline_state, f, indent=2)
        except Exception as e:
            logger.error(f"Błąd zapisu stanu: {e}")


def add_log(message, level='info'):
    """Dodaj wpis do logów"""
    timestamp = datetime.now().strftime('%H:%M:%S')
    log_entry = {
        'timestamp': timestamp,
        'message': message,
        'level': level
    }
    with state_lock:
        pipeline_state['logs'].append(log_entry)
        # Zachowaj ostatnie 500 logów
        if len(pipeline_state['logs']) > 500:
            pipeline_state['logs'] = pipeline_state['logs'][-500:]
    logger.info(f"[{level.upper()}] {message}")


def scan_input_directory(input_dir):
    """Skanuj katalog wejściowy w poszukiwaniu plików"""
    files = []
    if os.path.exists(input_dir):
        for filename in os.listdir(input_dir):
            filepath = os.path.join(input_dir, filename)
            if os.path.isfile(filepath):
                stat = os.stat(filepath)
                files.append({
                    'name': filename,
                    'size': stat.st_size,
                    'modified': datetime.fromtimestamp(stat.st_mtime).strftime('%Y-%m-%d %H:%M'),
                    'status': 'pending',
                    'path': filepath
                })
    return files


def run_pipeline_process(config):
    """Uruchom pipeline jako proces wątku"""
    global pipeline_state
    
    try:
        add_log("Rozpoczynanie pipeline...", "info")
        
        with state_lock:
            pipeline_state['running'] = True
            pipeline_state['status'] = 'running'
            pipeline_state['progress'] = 0
        
        # Przygotuj komendę
        cmd = [
            os.path.join(SCRIPT_DIR, 'run_pipeline.sh'),
            '--input', config.get('inputDir', './input'),
            '--chunk-dir', config.get('chunkDir', './chunk'),
            '--rewriten', config.get('rewritenDir', './rewriten'),
            '--output', config.get('outputDir', './finish'),
            '--model', config.get('model', 'qwen2.5-coder:7b'),
            '--style', config.get('style', 'technical'),
            '--length', config.get('length', 'medium'),
            '--lang', config.get('language', 'pl'),
            '--max-chunks', str(config.get('maxChunks', 5))
        ]
        
        if config.get('expand', False):
            cmd.append('--expand')
        
        add_log(f"Uruchamianie: {' '.join(cmd)}", "debug")
        
        # Uruchom proces
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            cwd=SCRIPT_DIR
        )
        
        # Czytaj output na bieżąco
        progress = 0
        while process.poll() is None:
            line = process.stdout.readline()
            if line:
                add_log(line.strip(), 'info')
                # Symuluj postęp na podstawie logów
                with state_lock:
                    progress = min(progress + 2, 95)
                    pipeline_state['progress'] = progress
        
        # Sprawdź wynik
        return_code = process.returncode
        if return_code == 0:
            add_log("Pipeline zakończony sukcesem!", "success")
            with state_lock:
                pipeline_state['progress'] = 100
                pipeline_state['completed'] += 1
        else:
            add_log(f"Pipeline zakończony z kodem błędu: {return_code}", "error")
            with state_lock:
                pipeline_state['errors'] += 1
        
    except Exception as e:
        add_log(f"Błąd pipeline: {str(e)}", "error")
        logger.exception("Exception in pipeline thread")
    
    finally:
        with state_lock:
            pipeline_state['running'] = False
            pipeline_state['status'] = 'idle'
        save_state()


@app.route('/')
def index():
    """Obsłuż plik HTML"""
    return send_from_directory(SCRIPT_DIR, 'webui.html')


@app.route('/api/status')
def get_status():
    """Pobierz aktualny status pipeline"""
    with state_lock:
        # Przelicz pliki
        input_dir = pipeline_state['config'].get('inputDir', './input')
        files = scan_input_directory(input_dir)
        
        pending = sum(1 for f in files if f['status'] == 'pending')
        processing = sum(1 for f in files if f['status'] == 'processing')
        completed = sum(1 for f in files if f['status'] == 'completed')
        
        return jsonify({
            'running': pipeline_state['running'],
            'progress': pipeline_state['progress'],
            'status': pipeline_state['status'],
            'pending': pending,
            'processing': processing,
            'completed': completed,
            'errors': pipeline_state['errors'],
            'files': files,
            'logs': pipeline_state['logs'][-100:],  # Ostatnie 100 logów
            'config': pipeline_state['config']
        })


@app.route('/api/config', methods=['GET'])
def get_config():
    """Pobierz aktualną konfigurację"""
    return jsonify(pipeline_state['config'])


@app.route('/api/config', methods=['POST'])
def set_config():
    """Ustaw konfigurację"""
    config = request.json
    with state_lock:
        pipeline_state['config'] = config
    save_state()
    add_log("Konfiguracja zaktualizowana", "success")
    return jsonify({'status': 'ok'})


@app.route('/api/start', methods=['POST'])
def start_pipeline():
    """Uruchom pipeline"""
    global pipeline_thread
    
    if pipeline_state['running']:
        return jsonify({'error': 'Pipeline już działa'}), 400
    
    config = request.json or {}
    with state_lock:
        pipeline_state['config'].update(config)
        pipeline_state['logs'] = []
        pipeline_state['progress'] = 0
    
    # Skanuj pliki wejściowe
    input_dir = pipeline_state['config'].get('inputDir', './input')
    files = scan_input_directory(input_dir)
    
    if not files:
        add_log("Brak plików w katalogu wejściowym!", "warning")
        return jsonify({'error': 'Brak plików do przetworzenia'}), 400
    
    with state_lock:
        pipeline_state['files'] = files
        pipeline_state['pending'] = len(files)
    
    # Uruchom w wątku
    pipeline_thread = threading.Thread(target=run_pipeline_process, args=(pipeline_state['config'],))
    pipeline_thread.daemon = True
    pipeline_thread.start()
    
    add_log(f"Znaleziono {len(files)} plików do przetworzenia", "info")
    return jsonify({'status': 'started', 'files_count': len(files)})


@app.route('/api/stop', methods=['POST'])
def stop_pipeline():
    """Zatrzymaj pipeline"""
    global pipeline_thread
    
    if not pipeline_state['running']:
        return jsonify({'error': 'Pipeline nie działa'}), 400
    
    # W przypadku bash script, musimy zabić proces
    # Dla uproszczenia, ustawiamy flagę zatrzymania
    with state_lock:
        pipeline_state['status'] = 'stopping'
    
    add_log("Zatrzymywanie pipeline...", "warning")
    return jsonify({'status': 'stopping'})


@app.route('/api/ollama/check')
def check_ollama():
    """Sprawdź dostępność Ollama"""
    try:
        result = subprocess.run(
            ['curl', '-s', 'http://localhost:11434/api/tags'],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.returncode == 0:
            try:
                data = json.loads(result.stdout)
                models = data.get('models', [])
                model_names = [m['name'] for m in models]
                add_log(f"Ollama dostępna. Modele: {', '.join(model_names)}", "success")
                return jsonify({
                    'available': True,
                    'models': model_names
                })
            except json.JSONDecodeError:
                pass
        
        add_log("Ollama odpowiada, ale nie można odczytać modeli", "warning")
        return jsonify({
            'available': True,
            'models': []
        })
        
    except subprocess.TimeoutExpired:
        add_log("Timeout podczas łączenia z Ollama", "error")
        return jsonify({'available': False, 'error': 'Timeout'}), 500
    except Exception as e:
        add_log(f"Nie można połączyć z Ollama: {str(e)}", "error")
        return jsonify({'available': False, 'error': str(e)}), 500


@app.route('/api/logs')
def get_logs():
    """Pobierz logi"""
    with state_lock:
        return jsonify(pipeline_state['logs'][-200:])


@app.route('/api/files')
def get_files():
    """Pobierz listę plików"""
    config = pipeline_state['config']
    input_dir = config.get('inputDir', './input')
    files = scan_input_directory(input_dir)
    return jsonify(files)


@app.route('/api/directories')
def get_directories():
    """Pobierz informacje o katalogach"""
    dirs = {}
    for dir_name in ['input', 'chunk', 'rewriten', 'finish', 'epub_input', 'converted']:
        dir_path = os.path.join(SCRIPT_DIR, dir_name)
        exists = os.path.exists(dir_path)
        writable = os.access(dir_path, os.W_OK) if exists else False
        file_count = len(os.listdir(dir_path)) if exists else 0
        
        dirs[dir_name] = {
            'path': dir_path,
            'exists': exists,
            'writable': writable,
            'file_count': file_count
        }
    
    return jsonify(dirs)


if __name__ == '__main__':
    load_state()
    add_log("Serwer WebUI uruchomiony", "success")
    print("\n" + "="*50)
    print("📚 Book Parser WebUI Server")
    print("="*50)
    print(f"📁 Katalog roboczy: {SCRIPT_DIR}")
    print(f"🌐 Adres: http://localhost:5000")
    print(f"📝 Logi: {LOGS_DIR}")
    print("="*50 + "\n")
    
    app.run(host='0.0.0.0', port=5000, debug=False, threaded=True)

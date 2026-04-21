# Book Parser WebUI

Działające Web UI do zarządzania pipeline'em przetwarzania książek.

## Wymagania

- Python 3.x
- Flask
- Flask-CORS

## Instalacja zależności

```bash
pip install flask flask-cors
```

## Uruchomienie serwera

```bash
cd /workspace
python server.py
```

Serwer będzie dostępny pod adresem: **http://localhost:5000**

## Funkcje WebUI

### Panel konfiguracji
- Katalogi (wejściowy, chunki, przepisane, wyjściowy)
- Model AI (Ollama)
- Styl generowania
- Długość outputu
- Język
- Maksymalna liczba chunków
- Opcja ekspansji treści przez LLM

### Status systemu
- Liczniki: oczekujące, w trakcie, zakończone, błędy
- Pasek postępu
- Lista plików do przetworzenia

### Akcje
- ▶️ Uruchom Pipeline - uruchamia przetwarzanie
- 🔄 Odśwież status - ręczne odświeżenie
- ✅ Sprawdź Ollama - weryfikacja dostępności modelu
- ⏹️ Zatrzymaj - zatrzymuje przetwarzanie

### Logi na żywo
- Podgląd logów w czasie rzeczywistym
- Kolorowanie według poziomu (info, success, warning, error)

## API Endpoints

| Endpoint | Metoda | Opis |
|----------|--------|------|
| `/` | GET | Strona HTML WebUI |
| `/api/status` | GET | Pobierz aktualny status pipeline |
| `/api/config` | GET | Pobierz konfigurację |
| `/api/config` | POST | Ustaw konfigurację |
| `/api/start` | POST | Uruchom pipeline |
| `/api/stop` | POST | Zatrzymaj pipeline |
| `/api/ollama/check` | GET | Sprawdź dostępność Ollama |
| `/api/logs` | GET | Pobierz logi |
| `/api/files` | GET | Pobierz listę plików |
| `/api/directories` | GET | Informacje o katalogach |

## Struktura plików

```
/workspace/
├── server.py          # Serwer Flask API
├── webui.html         # Frontend HTML/CSS/JS
├── input/             # Pliki wejściowe
├── chunk/             # Chunki
├── rewriten/          # Przepisane chunki
├── finish/            # Wyniki końcowe
└── logs/              # Logi serwera
```

## Auto-refresh

WebUI automatycznie odświeża status co 5 sekund podczas działania pipeline.

## Uwagi

- Wszystkie katalogy są tworzone automatycznie przy pierwszym uruchomieniu
- Stan pipeline jest zapisywany w pliku `.pipeline_state.json`
- Logi są przechowywane w pamięci (ostatnie 500 wpisów)

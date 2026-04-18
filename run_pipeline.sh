#!/bin/bash

# Pipeline przetwarzania dokumentów i wysyłki do mempalace
# Autor: Book-Parser Project
# Wymagania: chunker, mempalace_client, curl

set -e

# Konfiguracja
INPUT_DIR="${INPUT_DIR:-./input}"
CHUNK_DIR="${CHUNK_DIR:-./chunk}"
LOGS_DIR="${LOGS_DIR:-./logs}"
MEMPALACE_HOST="${MEMPALACE_HOST:-localhost}"
MEMPALACE_PORT="${MEMPALACE_PORT:-8080}"
VERBOSE="${VERBOSE:-false}"

# Kolory dla outputu
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo "========================================"
    echo "  Book-Parser Pipeline"
    echo "  Integracja z Mempalace"
    echo "========================================"
    echo ""
}

show_help() {
    echo "Użycie: $0 [opcje]"
    echo ""
    echo "Opcje:"
    echo "  -i, --input DIR       Katalog wejściowy z dokumentami (domyślnie: ./input)"
    echo "  -o, --output DIR      Katalog wyjściowy dla chunków (domyślnie: ./chunk)"
    echo "  -l, --logs DIR        Katalog logów (domyślnie: ./logs)"
    echo "  -H, --host HOST       Host mempalace (domyślnie: localhost)"
    echo "  -p, --port PORT       Port mempalace (domyślnie: 8080)"
    echo "  -c, --clean           Wyczyść katalog chunków przed rozpoczęciem"
    echo "  -s, --skip-upload     Tylko chunkowanie, bez wysyłki do mempalace"
    echo "  -w, --wait SEC        Czas oczekiwania między retry (sekundy, domyślnie: 5)"
    echo "  -v, --verbose         Tryb szczegółowy"
    echo "  -h, --help            Wyświetl tę pomoc"
    echo ""
    echo "Zmienne środowiskowe:"
    echo "  INPUT_DIR             Katalog wejściowy"
    echo "  CHUNK_DIR             Katalog wyjściowy dla chunków"
    echo "  LOGS_DIR              Katalog logów"
    echo "  MEMPALACE_HOST        Host mempalace"
    echo "  MEMPALACE_PORT        Port mempalace"
    echo "  VERBOSE               Tryb szczegółowy (true/false)"
    echo ""
    echo "Przykłady:"
    echo "  $0 -i ./input -o ./chunk -H localhost -p 8080"
    echo "  $0 --clean --verbose"
    echo "  MEMPALACE_HOST=192.168.1.100 $0 --skip-upload"
}

check_dependencies() {
    log_info "Sprawdzanie zależności..."
    
    local missing=0
    
    if ! command -v ./chunker &> /dev/null; then
        log_error "Brak chunker. Uruchom 'make chunker'"
        missing=1
    fi
    
    if ! command -v ./mempalace_client &> /dev/null; then
        log_error "Brak mempalace_client. Uruchom 'make mempalace'"
        missing=1
    fi
    
    if [ $missing -eq 1 ]; then
        log_error "Brakuje zależności. Skompiluj projekt poleceniem 'make all'"
        exit 1
    fi
    
    log_success "Wszystkie zależności dostępne"
}

wait_for_mempalace() {
    local max_retries=${1:-10}
    local wait_time=${2:-5}
    
    log_info "Oczekiwanie na dostępność mempalace (${max_retries} prób, ${wait_time}s między próbami)..."
    
    for i in $(seq 1 $max_retries); do
        if curl -s -o /dev/null -w "%{http_code}" "http://${MEMPALACE_HOST}:${MEMPALACE_PORT}/api/v1/health" | grep -q "200"; then
            log_success "mempalace dostępny na ${MEMPALACE_HOST}:${MEMPALACE_PORT}"
            return 0
        fi
        
        log_warning "Próba $i/$max_retries nieudana, czekam ${wait_time}s..."
        sleep $wait_time
    done
    
    log_error "mempalace nie jest dostępny po $max_retries próbach"
    return 1
}

run_chunking() {
    log_info "Rozpoczynanie procesu chunkowania..."
    log_info "Katalog wejściowy: $INPUT_DIR"
    log_info "Katalog wyjściowy: $CHUNK_DIR"
    
    local chunker_args="-i $INPUT_DIR -o $CHUNK_DIR -l $LOGS_DIR"
    
    if [ "$VERBOSE" = "true" ]; then
        chunker_args="$chunker_args -v"
    fi
    
    log_info "Uruchamianie chunkera: ./chunker $chunker_args"
    
    if ./chunker $chunker_args; then
        log_success "Chunkowanie zakończone pomyślnie"
        return 0
    else
        log_error "Błąd podczas chunkowania"
        return 1
    fi
}

run_upload() {
    log_info "Rozpoczynanie wysyłki chunków do mempalace..."
    log_info "Host: $MEMPALACE_HOST:$MEMPALACE_PORT"
    log_info "Katalog chunków: $CHUNK_DIR"
    
    local upload_args="-i $CHUNK_DIR -H $MEMPALACE_HOST -p $MEMPALACE_PORT"
    
    if [ "$VERBOSE" = "true" ]; then
        upload_args="$upload_args -v"
    fi
    
    log_info "Uruchamianie mempalace_client: ./mempalace_client $upload_args"
    
    if ./mempalace_client $upload_args; then
        log_success "Wysyłka chunków zakończona pomyślnie"
        return 0
    else
        log_error "Błąd podczas wysyłki chunków"
        return 1
    fi
}

count_chunks() {
    local count=$(find "$CHUNK_DIR" -name "*.json" -type f 2>/dev/null | wc -l)
    echo $count
}

# Parsowanie argumentów
CLEAN=false
SKIP_UPLOAD=false
WAIT_TIME=5

while [[ $# -gt 0 ]]; do
    case $1 in
        -i|--input)
            INPUT_DIR="$2"
            shift 2
            ;;
        -o|--output)
            CHUNK_DIR="$2"
            shift 2
            ;;
        -l|--logs)
            LOGS_DIR="$2"
            shift 2
            ;;
        -H|--host)
            MEMPALACE_HOST="$2"
            shift 2
            ;;
        -p|--port)
            MEMPALACE_PORT="$2"
            shift 2
            ;;
        -w|--wait)
            WAIT_TIME="$2"
            shift 2
            ;;
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -s|--skip-upload)
            SKIP_UPLOAD=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            log_error "Nieznana opcja: $1"
            show_help
            exit 1
            ;;
    esac
done

# Główny przepływ pracy
main() {
    print_header
    
    # Sprawdzenie zależności
    check_dependencies
    
    # Tworzenie katalogów
    mkdir -p "$CHUNK_DIR" "$LOGS_DIR"
    
    # Czyszczenie jeśli wymagane
    if [ "$CLEAN" = "true" ]; then
        log_info "Czyszczenie katalogu chunków..."
        rm -rf "$CHUNK_DIR"/*
        log_success "Katalog wyczyszczony"
    fi
    
    # Liczenie plików wejściowych
    local input_count=$(find "$INPUT_DIR" -type f \( -name "*.txt" -o -name "*.md" -o -name "*.json" -o -name "*.pdf" -o -name "*.doc" -o -name "*.docx" \) 2>/dev/null | wc -l)
    log_info "Znaleziono $input_count plików do przetworzenia w $INPUT_DIR"
    
    if [ "$input_count" -eq 0 ]; then
        log_warning "Brak plików do przetworzenia"
        exit 0
    fi
    
    # Chunkowanie
    if ! run_chunking; then
        log_error "Proces chunkowania nie powiódł się"
        exit 1
    fi
    
    # Liczenie utworzonych chunków
    local chunk_count=$(count_chunks)
    log_success "Utworzono $chunk_count chunków"
    
    # Wysyłka do mempalace (jeśli nie pominięto)
    if [ "$SKIP_UPLOAD" = "false" ]; then
        if wait_for_mempalace 10 $WAIT_TIME; then
            if ! run_upload; then
                log_error "Proces wysyłki nie powiódł się"
                exit 1
            fi
        else
            log_warning "mempalace niedostępny - pomijam wysyłkę"
            log_info "Możesz uruchomić wysyłkę później: ./mempalace_client -i $CHUNK_DIR -H $MEMPALACE_HOST -p $MEMPALACE_PORT"
        fi
    fi
    
    echo ""
    log_success "=== Pipeline zakończony pomyślnie ==="
    echo ""
    echo "Podsumowanie:"
    echo "  - Przetworzono plików: $input_count"
    echo "  - Utworzono chunków: $chunk_count"
    echo "  - Katalog chunków: $CHUNK_DIR"
    echo "  - Logi: $LOGS_DIR"
    
    if [ "$SKIP_UPLOAD" = "false" ]; then
        echo "  - Mempalace: $MEMPALACE_HOST:$MEMPALACE_PORT"
    fi
    
    echo ""
}

main

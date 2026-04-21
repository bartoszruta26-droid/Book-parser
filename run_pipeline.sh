#!/bin/bash
# Skrypt uruchamiający pipeline z ekspansją treści przez Ollama LLM

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Kolory dla outputu
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Wartości domyślne
INPUT_DIR="./input"
CHUNK_DIR="./chunk"
REWRITEN_DIR="./rewriten"
OUTPUT_DIR="./finish"
MODEL="qwen2.5-coder:7b"
STYLE="technical"
LENGTH="medium"
LANGUAGE="pl"
MAX_CHUNKS=5
EXPAND=false
CHECK_OLLAMA=false

print_help() {
    echo "=== Pipeline z Ekspansją Treści przez Ollama ==="
    echo ""
    echo "Użycie: $0 [opcje]"
    echo ""
    echo "Opcje:"
    echo "  -i, --input DIR       Katalog z dokumentami wejściowymi (domyślnie: ./input)"
    echo "  -c, --chunk-dir DIR   Katalog na chunki (domyślnie: ./chunk)"
    echo "  -r, --rewriten DIR    Katalog na przepisane chunki (domyślnie: ./rewriten)"
    echo "  -o, --output DIR      Katalog wyjściowy (domyślnie: ./finish)"
    echo "  -m, --model MODEL     Model Ollama (domyślnie: qwen2.5-coder:7b)"
    echo "  -s, --style STYLE     Styl: academic, journalistic, technical, creative, business, casual"
    echo "  -l, --length LENGTH   Długość: short, medium, long, very_long"
    echo "  --lang LANGUAGE       Język outputu (domyślnie: pl)"
    echo "  -n, --max-chunks N    Maksymalna liczba chunków do ekspansji (domyślnie: 5)"
    echo "  -e, --expand          Włącz ekspansję treści przez LLM"
    echo "  --check               Sprawdź dostępność Ollama i zakończ"
    echo "  -h, --help            Wyświetl pomoc"
    echo ""
    echo "Przykłady:"
    echo "  $0 --check"
    echo "  $0 -i ./input -e -s creative -l long"
    echo "  $0 -e -m llama3.2 --lang en"
}

# Parsowanie argumentów
while [[ $# -gt 0 ]]; do
    case $1 in
        -i|--input)
            INPUT_DIR="$2"
            shift 2
            ;;
        -c|--chunk-dir)
            CHUNK_DIR="$2"
            shift 2
            ;;
        -r|--rewriten)
            REWRITEN_DIR="$2"
            shift 2
            ;;
        -o|--output)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        -m|--model)
            MODEL="$2"
            shift 2
            ;;
        -s|--style)
            STYLE="$2"
            shift 2
            ;;
        -l|--length)
            LENGTH="$2"
            shift 2
            ;;
        --lang)
            LANGUAGE="$2"
            shift 2
            ;;
        -n|--max-chunks)
            MAX_CHUNKS="$2"
            shift 2
            ;;
        -e|--expand)
            EXPAND=true
            shift
            ;;
        --check)
            CHECK_OLLAMA=true
            shift
            ;;
        -h|--help)
            print_help
            exit 0
            ;;
        *)
            echo -e "${RED}Nieznana opcja: $1${NC}"
            print_help
            exit 1
            ;;
    esac
done

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Pipeline z Ekspansją Treści Ollama   ${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Sprawdzenie dostępności Ollama
check_ollama() {
    echo -e "${YELLOW}Sprawdzanie dostępności Ollama...${NC}"
    
    if command -v curl &> /dev/null; then
        RESPONSE=$(curl -s -w "%{http_code}" -o /tmp/ollama_response.json http://localhost:11434/api/tags 2>/dev/null || echo "000")
        
        if [[ "$RESPONSE" == "200" ]]; then
            echo -e "${GREEN}✓ Ollama jest dostępne!${NC}"
            
            # Pobierz listę modeli
            MODELS=$(cat /tmp/ollama_response.json | grep -o '"name":"[^"]*"' | cut -d'"' -f4 || echo "")
            
            if [[ -n "$MODELS" ]]; then
                echo -e "${GREEN}Dostępne modele:${NC}"
                echo "$MODELS" | while read model; do
                    if [[ "$model" == *"$MODEL"* ]] || [[ "$MODEL" == *"$model"* ]]; then
                        echo -e "  ${GREEN}✓ $model${NC}"
                    else
                        echo "  - $model"
                    fi
                done
            else
                echo -e "${YELLOW}Brak zainstalowanych modeli${NC}"
            fi
            
            return 0
        else
            echo -e "${RED}✗ Ollama nie odpowiada na porcie 11434${NC}"
            return 1
        fi
    else
        echo -e "${RED}✗ curl nie jest zainstalowany${NC}"
        return 1
    fi
}

# Tryb sprawdzania
if [[ "$CHECK_OLLAMA" == true ]]; then
    check_ollama
    exit $?
fi

# Tworzenie katalogów
echo -e "${YELLOW}Przygotowanie katalogów...${NC}"
mkdir -p "$CHUNK_DIR" "$REWRITEN_DIR" "$OUTPUT_DIR"

# Kompilacja narzędzi
echo -e "${YELLOW}Kompilacja narzędzi...${NC}"
make all > /dev/null 2>&1 || {
    echo -e "${RED}Błąd kompilacji!${NC}"
    exit 1
}
echo -e "${GREEN}✓ Narzędzia skompilowane${NC}"
echo ""

# Krok 1: Chunking dokumentów
echo -e "${BLUE}=== KROK 1: Chunking dokumentów ===${NC}"

if [[ ! -d "$INPUT_DIR" ]] || [[ -z "$(ls -A "$INPUT_DIR" 2>/dev/null)" ]]; then
    echo -e "${YELLOW}Katalog input jest pusty. Tworzę przykładowy dokument...${NC}"
    mkdir -p "$INPUT_DIR"
    cat > "$INPUT_DIR/sample_document.txt" << 'EOF'
# Wprowadzenie do Sztucznej Inteligencji

## Historia AI

Sztuczna inteligencja to dziedzina nauki zajmująca się tworzeniem systemów komputerowych zdolnych do wykonywania zadań wymagających ludzkiej inteligencji. Pierwsze koncepcje AI pojawiły się już w starożytności, ale正式ne narodziny tej dziedziny przypadają na lata 50. XX wieku.

### Wczesne lata

W 1950 roku Alan Turing opublikował przełomowy artykuł "Computing Machinery and Intelligence", w którym zaproponował test Turinga jako miarę inteligencji maszyny. To właśnie Turing zasugerował pytanie: "Czy maszyny mogą myśleć?"

### Narodziny terminu

Termin "sztuczna inteligencja" został coined w 1956 roku podczas konferencji w Dartmouth, zorganizowanej przez Johna McCarthy'ego, Marvina Minsky'ego, Nathaniela Rochestera i Claude'a Shannona.

## Główne podejścia

### AI Symboliczne

Podejście symboliczne, zwane też GOFAI (Good Old-Fashioned AI), zakładało reprezentację wiedzy za pomocą symboli i reguł logicznych. Systemy eksperckie lat 70. i 80. były przykładem tego podejścia.

### Uczenie Maszynowe

Uczenie maszynowe to podejście statystyczne, gdzie systemy uczą się na podstawie danych zamiast być programowane explicite. Deep learning, czyli uczenie głębokie, jest podzbiorem uczenia maszynowego wykorzystującym sieci neuronowe.

## Zastosowania współczesne

### Przetwarzanie Języka Naturalnego

Modele językowe takie jak GPT, BERT czy Qwen revolucionizują sposób interakcji człowiek-komputer. Chatboty, tłumaczenia automatyczne i analiza sentymentu to tylko niektóre zastosowania.

### Wizja Komputerowa

Systemy rozpoznawania obrazów znajdują zastosowanie w medycynie, motoryzacji (samochody autonomiczne), bezpieczeństwie i rozrywce.

### Robotyka

AI umożliwia robotom adaptację do nowych środowisk i zadań, od robotów przemysłowych po humanoidy.

## Wyzwania i przyszłość

### Wyzwania etyczne

Rozwój AI rodzi pytania o prywatność, bias w algorytmach, wpływ na rynek pracy i autonomię systemów decyzyjnych.

### Przyszłość

Eksperci przewidują dalszy rozwój AGI (Artificial General Intelligence), bardziej zaawansowanych interfejsów mózg-komputer oraz integracji AI z codziennością.
EOF
    echo -e "${GREEN}✓ Utworzono przykładowy dokument${NC}"
fi

echo -e "${YELLOW}Uruchamianie chunkera...${NC}"
./chunker -i "$INPUT_DIR" -o "$CHUNK_DIR" -v

CHUNK_COUNT=$(ls -1 "$CHUNK_DIR"/*.json 2>/dev/null | wc -l)
echo -e "${GREEN}✓ Utworzono $CHUNK_COUNT chunków${NC}"
echo ""

# Krok 2: Wysyłka do Mempalace (opcjonalna)
echo -e "${BLUE}=== KROK 2: Integracja z Mempalace (opcjonalna) ===${NC}"
echo -e "${YELLOW}Sprawdzanie dostępności mempalace...${NC}"

if ./mempalace_client --check 2>/dev/null; then
    echo -e "${GREEN}✓ Mempalace dostępne, wysyłanie chunków...${NC}"
    ./mempalace_client -i "$CHUNK_DIR" 2>&1 | tail -5
else
    echo -e "${YELLOW}⊘ Mempalace niedostępne - pomijam ten krok${NC}"
fi
echo ""

# Krok 3: Ekspansja treści przez LLM
if [[ "$EXPAND" == true ]]; then
    echo -e "${BLUE}=== KROK 3: Ekspansja treści przez LLM ===${NC}"
    
    # Sprawdź Ollama
    if ! check_ollama; then
        echo -e "${RED}✗ Ollama nie jest dostępne!${NC}"
        echo -e "${YELLOW}Uruchom Ollama poleceniem: ollama serve${NC}"
        echo -e "${YELLOW}Następnie pobierz model: ollama pull $MODEL${NC}"
        exit 1
    fi
    
    # Sprawdź czy model jest dostępny
    if ! curl -s http://localhost:11434/api/tags | grep -q "\"name\":\"$MODEL\""; then
        echo -e "${YELLOW}Model '$MODEL' nie jest dostępny. Pobieranie...${NC}"
        echo -e "${YELLOW}To może potrwać kilka minut w zależności od rozmiaru modelu.${NC}"
        
        if command -v ollama &> /dev/null; then
            ollama pull "$MODEL"
        else
            echo -e "${RED}✗ Klient ollama nie jest zainstalowany${NC}"
            echo -e "${YELLOW}Możesz kontynuować bez ekspansji lub zainstalować Ollama${NC}"
            EXPAND=false
        fi
    fi
    
    if [[ "$EXPAND" == true ]]; then
        echo -e "${YELLOW}Uruchamianie ekspandera treści...${NC}"
        echo -e "  Model: ${GREEN}$MODEL${NC}"
        echo -e "  Styl: ${GREEN}$STYLE${NC}"
        echo -e "  Długość: ${GREEN}$LENGTH${NC}"
        echo -e "  Język: ${GREEN}$LANGUAGE${NC}"
        echo -e "  Max chunków: ${GREEN}$MAX_CHUNKS${NC}"
        echo ""
        
        TIMESTAMP=$(date +%Y%m%d_%H%M%S)
        REWRITEN_OUTPUT="$REWRITEN_DIR/rewritten_${TIMESTAMP}.json"
        TEXT_OUTPUT="$REWRITEN_DIR/rewritten_${TIMESTAMP}.txt"
        FINAL_OUTPUT="$OUTPUT_DIR/final_${TIMESTAMP}.json"
        
        # Ekspansja z wybranymi parametrami - zapis do /rewriten
        ./ollama_expander \
            -i "$CHUNK_DIR" \
            -o "$REWRITEN_OUTPUT" \
            -m "$MODEL" \
            -s "$STYLE" \
            -l "$LENGTH" \
            --lang "$LANGUAGE" \
            --max-chunks "$MAX_CHUNKS" \
            -v
        
        # Konwersja do formatu tekstowego - zapis do /rewriten
        ./ollama_expander \
            -i "$CHUNK_DIR" \
            -o "$TEXT_OUTPUT" \
            -t \
            -m "$MODEL" \
            -s "$STYLE" \
            -l "$LENGTH" \
            --lang "$LANGUAGE" \
            --max-chunks "$MAX_CHUNKS" \
            2>/dev/null
        
        echo ""
        echo -e "${GREEN}✓ Przepisywanie zakończone sukcesem!${NC}"
        echo -e "${YELLOW}Wyniki zapisano w:${NC}"
        echo -e "  JSON: $REWRITEN_OUTPUT"
        echo -e "  TXT:  $TEXT_OUTPUT"
        
        # Krok 4: Przeniesienie wyników do /finish
        echo ""
        echo -e "${BLUE}=== KROK 4: Zapis do katalogu finish ===${NC}"
        cp "$REWRITEN_OUTPUT" "$FINAL_OUTPUT"
        cp "$TEXT_OUTPUT" "$OUTPUT_DIR/final_${TIMESTAMP}.txt"
        echo -e "${GREEN}✓ Wyniki przeniesione do katalogu finish${NC}"
        echo -e "${YELLOW}Finalne pliki w:${NC}"
        echo -e "  JSON: $FINAL_OUTPUT"
        echo -e "  TXT:  $OUTPUT_DIR/final_${TIMESTAMP}.txt"
    fi
else
    echo -e "${BLUE}=== KROK 3: Pominięto ekspansję ===${NC}"
    echo -e "${YELLOW}Użyj flagi -e lub --expand aby włączyć ekspansję przez LLM${NC}"
fi

echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}  Pipeline zakończone sukcesnie!       ${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "${YELLOW}Podsumowanie:${NC}"
echo -e "  Dokumenty wejściowe: $INPUT_DIR"
echo -e "  Chunki: $CHUNK_DIR ($CHUNK_COUNT plików)"
echo -e "  Przepisane: $REWRITEN_DIR"
echo -e "  Finalne: $OUTPUT_DIR"

if [[ "$EXPAND" == true ]]; then
    echo -e "  Expander: ${GREEN}aktywny${NC} (model: $MODEL)"
fi

echo ""
echo -e "${YELLOW}Następne kroki:${NC}"
echo -e "  1. Przeglądaj wyniki w katalogu $OUTPUT_DIR (finish)"
echo -e "  2. Sprawdź przepisane chunki w $REWRITEN_DIR"
echo -e "  3. Uruchom ponownie z innymi parametrami stylu/długości"
echo -e "  4. Skonfiguruj mempalace dla trwałego przechowywania chunków"

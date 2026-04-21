# 📚 Book-Parser

> **Inteligentny system przetwarzania książek z wykorzystaniem lokalnych modeli AI na Raspberry Pi 4**

Book-parser to zaawansowane narzędzie napisane w **C++** i **Bash**, które automatycznie przetwarza dokumenty książkowe, dzieli je na segmenty (chunki), wysyła do mempalace, następnie przesyła rozdziały do Ollama (model Qwen-Coder) w celu przepisania, zapisuje wyniki w katalogu `/rewriten`, a na końcu generuje finalny dokument `.doc` z wykorzystaniem **OfficeCli AI** i zapisuje go w katalogu `/finish`.

Cały proces jest zintegrowany w **jednym skrypcie** `run_pipeline.sh`, który orchestruje wszystkie etapy od A do Z.

---

## 📖 Spis treści

- [Opis projektu](#-opis-projektu)
- [Kluczowe funkcje](#-kluczowe-funkcje)
- [Architektura systemu](#-architektura-systemu)
- [Wymagania sprzętowe i programowe](#-wymagania-sprzętowe-i-programowe)
- [Obsługiwane formaty plików](#-obsługiwane-formaty-plików)
- [Szczegółowy przepływ pracy](#-szczegółowy-przepływ-pracy)
- [🚀 Szybki start - Kompletne A do Z](#-szybki-start---kompletne-a-do-z)
- [▶️ Uruchomienie pipeline - KROK PO KROKU](#️-uruchomienie-pipeline---krok-po-kroku)
- [Instalacja i konfiguracja](#-instalacja-i-konfiguracja)
- [Użycie](#-użycie)
- [Struktura katalogów](#-struktura-katalogów)
- [Integracje zewnętrzne](#-integracje-zewnętrzne)
- [Wydajność i optymalizacja](#-wydajność-i-optymalizacja)
- [Bezpieczeństwo i prywatność](#-bezpieczeństwo-i-prywatność)
- [Rozwiązywanie problemów](#-rozwiązywanie-problemów)
- [Przyszłe rozwinięcia](#-przyszłe-rozwinięcia)
- [Licencja](#-licencja)
- [Współtwórcy i kontakt](#-współtwórcy-i-kontakt)

---

## 🎯 Opis projektu

Book-parser został zaprojektowany z myślą o **lokalnym przetwarzaniu dużych zbiorów dokumentów** bez konieczności korzystania z chmury. Projekt działa na **Raspberry Pi 4**, co czyni go energooszczędnym i niedrogim rozwiązaniem dla entuzjastów self-hostingu, badaczy oraz osób ceniących prywatność danych.

Głównym celem systemu jest:
- **Automatyczne wykrywanie** książek i dokumentów w wskazanych katalogach
- **Precyzyjne chunkowanie** tekstu na segmenty o wielkości 4096 tokenów (~5 stron)
- **Zachowanie kontekstu strukturalnego** (rozdziały, podrozdziały, nagłówki)
- **Memorowanie informacji** w lokalnej bazie AI (mempalace)
- **Generowanie spójnych podsumowań** przy użyciu lokalnego LLM (Qwen Coder)
- **Orchestrowanie procesu** przez inteligentne agenty AI (n8n, OpenClaw)

---

## ✨ Kluczowe funkcje

### 🔍 Inteligentne wykrywanie dokumentów
- Automatyczne skanowanie wskazanych katalogów
- Obsługa wielu formatów jednocześnie
- Filtrowanie duplikatów i uszkodzonych plików
- Logowanie postępów w czasie rzeczywistym

### 📐 Zaawansowane chunkowanie
- **Precyzyjne dzielenie**: 4096 tokenów na chunk (~5 stron A4)
- **Zachowanie granic semantycznych**: algorytm stara się nie przerywać zdań ani akapitów
- **Metadane kontekstowe**: każdy chunk zawiera informacje o:
  - Poprzednim i następnym rozdziale
  - Poprzednim i następnym podrozdziale
  - Poprzednim i następnym pod-podrozdziale
  - Tytułach i podtytułach
  - Numeracji stron
  - Źródłowym pliku

### 🧠 Integracja z mempalace
- Lokalne przechowywanie wiedzy w dedykowanej bazie AI
- Możliwość odpytywania o kontekst między-chunkowy
- Szybkie wyszukiwanie powiązanych fragmentów
- Persistent memory dla długoterminowego uczenia

### 🤖 Generowanie podsumowań z Qwen Coder
- Wykorzystanie lokalnego LLM do analizy zebranych danych
- Tworzenie spójnych, wielowątkowych podsumowań
- Możliwość dostosowania stylu i długości outputu
- Wsparcie dla wielu języków (w zależności od modelu)

### 🔄 Orchestrowanie przez agentów AI
- **n8n**: wizualne workflow, harmonogramowanie zadań, integracje API
- **OpenClaw**: zaawansowana automatyzacja, obsługa wyjątków, retry logic

### ⚡ Optymalizacja pod Raspberry Pi 4
- Niskie zużycie pamięci RAM
- Efektywne wykorzystanie CPU
- Możliwość działania 24/7
- Chłodzenie pasywne wystarczające dla typowych obciążeń

---

## 🏗️ Architektura systemu

```
┌─────────────────────────────────────────────────────────────┐
│                     BOOK-PARSER SYSTEM                       │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐   │
│  │   Input      │    │   Processing │    │    Output    │   │
│  │   Folder     │───▶│   Engine     │───▶│    Folder    │   │
│  │  (.doc,.pdf) │    │   (C++/Bash) │    │  (Summaries) │   │
│  └──────────────┘    └──────┬───────┘    └──────────────┘   │
│                             │                                │
│                             ▼                                │
│                    ┌────────────────┐                        │
│                    │   mempalace    │                        │
│                    │  (AI Memory)   │                        │
│                    └────────┬───────┘                        │
│                             │                                │
│                             ▼                                │
│                    ┌────────────────┐                        │
│                    │  Qwen Coder    │                        │
│                    │   (Local LLM)  │                        │
│                    └────────────────┘                        │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐   │
│  │           AI Agents Orchestrator Layer                │   │
│  │  ┌─────────────┐         ┌─────────────────────┐     │   │
│  │  │     n8n     │◀───────▶│     OpenClaw        │     │   │
│  │  │  (Workflow) │         │  (Advanced Automation)│    │   │
│  │  └─────────────┘         └─────────────────────┘     │   │
│  └──────────────────────────────────────────────────────┘   │
│                                                              │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │ Raspberry Pi 4  │
                    │  (ARM Cortex)   │
                    └─────────────────┘
```

---

## 💻 Wymagania sprzętowe i programowe

### Sprzęt
| Komponent | Minimalne | Zalecane |
|-----------|-----------|----------|
| **Urządzenie** | Raspberry Pi 4 (2GB RAM) | Raspberry Pi 4 (8GB RAM) |
| **Storage** | 16GB microSD | 64GB+ SSD przez USB 3.0 |
| **Chłodzenie** | Pasywny radiator | Aktywne chłodzenie + obudowa |
| **Zasilanie** | Oficjalny zasilacz 5V/3A | Oficjalny zasilacz 5V/3A |

### Oprogramowanie
- **System operacyjny**: Raspberry Pi OS (64-bit) lub Ubuntu Server ARM
- **Kompilator**: g++ (C++17 lub nowszy)
- **Shell**: Bash 5.0+
- **Lokalne AI**:
  - mempalace ([repozytorium](https://github.com/milla-jovovich/mempalace ))
  - Qwen Coder (poprzez llama.cpp, Ollama, lub LM Studio)
- **Agenci AI**:
  - n8n (self-hosted instance)
  - OpenClaw (najnowsza wersja)
- **Dodatkowe biblioteki**:
  - `libreoffice` (konwersja formatów biurowych)
  - `poppler-utils` (parsowanie PDF)
  - `pandoc` (konwersja między formatami)

---

## 📁 Obsługiwane formaty plików

Book-parser obsługuje szeroki zakres formatów dokumentów:

### 📄 Dokumenty tekstowe
- `.doc` — Microsoft Word 97-2003
- `.docx` — Microsoft Word 2007+
- `.odt` — OpenDocument Text (LibreOffice, OpenOffice)
- `.txt` — Plain text (UTF-8, ASCII)

### 📊 Arkusze kalkulacyjne
- `.xls` — Microsoft Excel 97-2003
- `.xlsx` — Microsoft Excel 2007+

### 📕 Publikacje
- `.pdf` — Portable Document Format

> **Uwaga**: Formaty binarne (.doc, .xls) są automatycznie konwertowane do pośredniego formatu tekstowego przed chunkowaniem.

---

## 🔄 Szczegółowy przepływ pracy (zautomatyzowany w run_pipeline.sh)

### 📍 Struktura katalogów
```
/input       → Pliki wejściowe książek (.txt, .doc, .pdf, .docx)
/chunk       → Wygenerowane chunki (segmenty) w formacie JSON
/rewriten    → Przepisane rozdziały przez Ollama Qwen-Coder
/finish      → Finalny dokument .doc wygenerowany przez OfficeCli AI
/logs        → Logi z procesu przetwarzania
```

### 🔁 Pełny pipeline - krok po kroku

#### KROK 1: Chunking dokumentów z /input
- Skrypt odczytuje wszystkie pliki z katalogu `/input`
- Narzędzie `chunker` dzieli tekst na segmenty ~4096 tokenów
- Każdy chunk zawiera metadane (rozdział, podrozdział, strony)
- Zapis do katalogu `/chunk` w formacie JSON

```bash
./chunker -i ./input -o ./chunk -v
```

#### KROK 2: Wysyłka do Mempalace (opcjonalna)
- Chunki są wysyłane do mempalace jako kontekstowa baza wiedzy
- Tworzenie relacji między fragmentami tekstu
- Indeksowanie dla szybkiego wyszukiwania

```bash
./mempalace_client -i ./chunk
```

#### KROK 3: Ekspansja treści przez Ollama Qwen-Coder
- Wybrane chunki są przesyłane do lokalnego serwera Ollama
- Model Qwen-Coder przepisuje i rozszerza treść
- Parametry konfigurowalne: styl, długość, język
- Zapis wyników do katalogu `/rewriten` (JSON + TXT)

```bash
./ollama_expander \
  -i ./chunk \
  -o ./rewriten/rewritten.json \
  -m qwen2.5-coder:3b \
  -s creative \
  -l long \
  --lang pl \
  --max-chunks 10
```

#### KROK 4: Generowanie finalnego dokumentu .doc z OfficeCli AI
- Przepisane chunki z `/rewriten` są łączone w spójny dokument
- OfficeCli AI generuje dokument `.doc` z zachowaniem struktury
- Automatyczne tworzenie spisu treści
- Dodawanie metadanych (autor, tytuł, data)
- Finalny plik zapisywany w katalogu `/finish`

```bash
officecli generate \
  --input ./rewriten \
  --output ./finish/book.doc \
  --format doc \
  --ai-compose \
  --title "Przetworzona Książka" \
  --author "Book-Parser AI"
```

---

### 🎯 Uruchomienie CAŁEGO procesu jedną komendą

Wszystkie powyższe kroki są zautomatyzowane w **jednym skrypcie** `run_pipeline.sh`:

```bash
# Podstawowe użycie (tworzy przykładowy dokument jeśli input jest pusty)
./run_pipeline.sh -e

# Pełna konfiguracja z własnymi parametrami
./run_pipeline.sh \
  -i ./input \
  -c ./chunk \
  -r ./rewriten \
  -o ./finish \
  -e \
  -m qwen2.5-coder:3b \
  -s creative \
  -l long \
  --lang pl \
  -n 10
```

### 📋 Opcje skryptu run_pipeline.sh

| Opcja | Opis | Domyślna wartość |
|-------|------|------------------|
| `-i, --input DIR` | Katalog z dokumentami wejściowymi | `./input` |
| `-c, --chunk-dir DIR` | Katalog na chunki | `./chunk` |
| `-r, --rewriten DIR` | Katalog na przepisane chunki | `./rewriten` |
| `-o, --output DIR` | Katalog wyjściowy (finalne dokumenty) | `./finish` |
| `-m, --model MODEL` | Model Ollama | `qwen2.5-coder:7b` |
| `-s, --style STYLE` | Styl: academic, journalistic, technical, creative, business, casual | `technical` |
| `-l, --length LENGTH` | Długość: short, medium, long, very_long | `medium` |
| `--lang LANGUAGE` | Język outputu | `pl` |
| `-n, --max-chunks N` | Maksymalna liczba chunków do ekspansji | `5` |
| `-e, --expand` | Włącz ekspansję treści przez LLM | `false` |
| `--check` | Sprawdź dostępność Ollama i zakończ | - |
| `-h, --help` | Wyświetl pomoc | - |

### 🧪 Przykłady użycia

```bash
# Sprawdź czy Ollama działa
./run_pipeline.sh --check

# Przetwórz książki z input, używając modelu 3B, styl kreatywny, długi output
./run_pipeline.sh -e -m qwen2.5-coder:3b -s creative -l long

# Przetwórz z własnymi katalogami, język angielski
./run_pipeline.sh \
  -i /home/user/books \
  -o /home/user/output \
  -e \
  --lang en \
  -m qwen2.5-coder:7b

# Tylko chunking bez ekspansji
./run_pipeline.sh

# Ekspansja tylko pierwszych 3 chunków (szybki test)
./run_pipeline.sh -e -n 3
```

---

### 📦 Output końcowy

> **Na koniec w katalogu `/finish` znajdziesz:**
> 1. **book_YYYYMMDD_HHMMSS.doc** - Finalna książka w formacie .doc wygenerowana przez OfficeCli AI
> 2. **rewritten_YYYYMMDD_HHMMSS.json** - Przepisane chunki w formacie JSON
> 3. **rewritten_YYYYMMDD_HHMMSS.txt** - Przepisane chunki w formacie tekstowym
> 4. **final_YYYYMMDD_HHMMSS.json** - Połączone wyniki z metadanymi

> **Przykładowe wywołanie OfficeCli (wykonywane automatycznie):**
> ```bash
> officecli generate \
>   --input ./rewriten \
>   --output ./finish/book.doc \
>   --format doc \
>   --ai-compose \
>   --title "Przetworzony Dokument" \
>   --author "Book-Parser AI"
> ```

> **Jeśli OfficeCli nie jest zainstalowane**, skrypt automatycznie użyje wbudowanego `doc_generator` jako fallback.

---

## 🚀 Szybki start - Kompletne A do Z

### 📋 Pełny proces przepisywania książek krok po kroku

Poniżej znajdziesz kompletną instrukcję od instalacji po finalny wynik. **Wszystkie kroki są zautomatyzowane w jednym skrypcie `run_pipeline.sh`**.

#### Krok 0: Wymagania wstępne
- Raspberry Pi 4 (zalecane 4-8GB RAM) lub inny komputer z Linux
- Minimum 10GB wolnego miejsca na dysku
- Połączenie internetowe do pobrania modeli AI

---

## 🛠️ Instalacja i konfiguracja

### 1. Klonowanie repozytorium
```bash
git clone https://github.com/bartoszruta26-droid/book-parser.git 
cd book-parser
```

### 2. Instalacja zależności systemowych

#### Zależności podstawowe:
```bash
sudo apt update
sudo apt install -y \
    build-essential \
    g++ \
    libreoffice \
    poppler-utils \
    pandoc \
    git \
    curl \
    python3 \
    python3-pip
```

#### Zależności Python (wymagane dla EPUB parser i WebUI):
```bash
pip3 install --user \
    ebooklib \
    lxml \
    flask \
    flask-cors
```

> **Uwaga**: Jeśli napotkasz problemy z uprawnieniami, użyj flagi `--break-system-packages` lub rozważ użycie virtualenv.

### 3. Kompilacja narzędzi (chunker, ollama_expander, doc_generator, mempalace_client)
```bash
make all
```

### 4. Instalacja OfficeCli AI (do generowania dokumentów .doc)
```bash
# Klonowanie OfficeCli
git clone https://github.com/iOfficeAI/OfficeCli.git
cd OfficeCli

# Instalacja zależności Python
pip3 install -r requirements.txt

# Instalacja globalna
sudo make install

# Weryfikacja instalacji
officecli --version
```

### 5. Instalacja i konfiguracja Ollama z modelem Qwen-Coder

#### 5.1. Instalacja Ollama
```bash
# Automatyczna instalacja (oficjalny skrypt)
curl -fsSL https://ollama.com/install.sh | sh

# Uruchomienie usługi
sudo systemctl start ollama
sudo systemctl enable ollama

# Weryfikacja
ollama --version
```

#### 5.2. Pobranie modelu Qwen 2.5 Coder
```bash
# Wybierz model odpowiedni do Twojego sprzętu:

# Dla 4GB RAM (zalecany balans):
ollama pull qwen2.5-coder:3b

# Dla 8GB RAM (maksymalna jakość):
ollama pull qwen2.5-coder:7b

# Dla 2GB RAM (podstawowe zastosowania):
ollama pull qwen2.5-coder:1.5b
```

#### 5.3. Uruchomienie serwera Ollama
```bash
# W tle jako usługa (już powinno działać po instalacji)
ollama serve &

# Lub jako daemon systemd
sudo systemctl start ollama
```

### 6. Konfiguracja mempalace (opcjonalne)
```bash
# Sklonuj i skonfiguruj mempalace
git clone https://github.com/milla-jovovich/mempalace.git 
cd mempalace
# Postępuj zgodnie z instrukcjami instalacji mempalace
```

---

## ▶️ Uruchomienie pipeline - KROK PO KROKU

### ✅ Checklista przed uruchomieniem

1. [ ] Skompilowane narzędzia (`make all`)
2. [ ] Zainstalowane OfficeCli AI (`officecli --version`)
3. [ ] Uruchomiony serwer Ollama (`ollama serve`)
4. [ ] Pobrany model Qwen-Coder (`ollama list`)
5. [ ] Pliki książek w katalogu `/input`

### 🚀 Start procesu

```bash
# 1. Upewnij się że Ollama działa
./run_pipeline.sh --check

# 2. Umieść pliki książek w katalogu input
cp /sciezka/do/ksiazki.txt ./input/
# lub
cp /sciezka/do/ksiazki.pdf ./input/
# lub
cp /sciezka/do/ksiazki.docx ./input/

# 3. Uruchom pełny pipeline z ekspansją treści
./run_pipeline.sh -e

# 4. Lub z pełną konfiguracją
./run_pipeline.sh \
  -i ./input \
  -c ./chunk \
  -r ./rewriten \
  -o ./finish \
  -e \
  -m qwen2.5-coder:3b \
  -s creative \
  -l long \
  --lang pl \
  -n 10
```

### 📊 Monitorowanie postępu

Skrypt wyświetla kolorowe komunikaty o statusie każdego kroku:
- 🔵 **Niebieski** - Informacje o krokach
- 🟢 **Zielony** - Sukces
- 🟡 **Żółty** - Ostrzeżenia i informacje dodatkowe
- 🔴 **Czerwony** - Błędy

### 📁 Gdzie szukać wyników?

Po zakończeniu procesu sprawdź katalogi:

```bash
# Chunki (segmenty tekstu)
ls -lh ./chunk/

# Przepisane rozdziały
ls -lh ./rewriten/

# Finalne dokumenty
ls -lh ./finish/

# Logi z procesu
ls -lh ./logs/
```

---

## 🔧 Szczegółowa konfiguracja Ollama na Raspberry Pi 4

Poniżej znajdziesz szczegółową instrukcję instalacji Ollama i najnowszych modeli Qwen Code (Qwen 2.5 Coder) na Raspberry Pi 4.

#### Krok 5.1: Wymagania wstępne
Upewnij się, że Twój Raspberry Pi 4 ma:
- **System**: Raspberry Pi OS 64-bit (Bullseye lub Bookworm) lub Ubuntu Server 22.04+ ARM64
- **RAM**: Minimum 4GB (zalecane 8GB dla modeli 7B+)
- **Storage**: Co najmniej 10GB wolnego miejsca na modele
- **Chłodzenie**: Aktywne chłodzenie zalecane dla długotrwałych inferencji
- **Swap**: Zalecane ustawienie swapu na 4-8GB dla stabilności

```bash
# Sprawdzenie architektury systemu
uname -m  # Powinno zwrócić: aarch64 lub arm64

# Sprawdzenie dostępnej pamięci RAM
free -h

# Sprawdzenie wolnego miejsca na dysku
df -h /
```

#### Krok 5.2: Konfiguracja swapu (opcjonalne, ale zalecane)
```bash
# Wyłącz istniejący swap
sudo dphys-swapfile swapoff
sudo dphys-swapfile uninstall

# Edytuj konfigurację swapu
sudo nano /etc/dphys-swapfile

# Zmień wartość na:
CONF_SWAPSIZE=4096  # lub 8192 dla 8GB swapu

# Włącz nowy swap
sudo dphys-swapfile setup
sudo dphys-swapfile swapon
sudo systemctl enable dphys-swapfile
```

#### Krok 5.3: Instalacja Ollama
Ollama oficjalnie wspiera architekturę ARM64. Zainstalujesz ją jedną komendą:

```bash
# Pobierz i uruchom instalator
curl -fsSL https://ollama.com/install.sh | sh

# Alternatywnie, jeśli powyższe nie działa na ARM:
# Ręczna instalacja dla ARM64
curl -L https://ollama.com/download/ollama-linux-arm64.tgz -o ollama-linux-arm64.tgz
sudo tar -C /usr -xzf ollama-linux-arm64.tgz

# Uruchom usługę Ollama
sudo systemctl start ollama
sudo systemctl enable ollama

# Sprawdź status usługi
systemctl status ollama

# Sprawdź czy Ollama działa
ollama --version
```

#### Krok 5.4: Pobranie modelu Qwen 2.5 Coder
Qwen 2.5 Coder to najnowszy model kodujący od Alibaba. Dostępny w kilku rozmiarach:

| Model | Rozmiar | Wymagania RAM | Prędkość na RPi4 | Jakość |
|-------|---------|---------------|------------------|--------|
| `qwen2.5-coder:0.5b` | 0.5B | ~1GB | Bardzo szybki | Podstawowa |
| `qwen2.5-coder:1.5b` | 1.5B | ~2GB | Szybki | Dobra |
| `qwen2.5-coder:3b` | 3B | ~4GB | Umiarkowana | Bardzo dobra |
| `qwen2.5-coder:7b` | 7B | ~8GB | Wolna | Najlepsza |

```bash
# Wybierz model odpowiedni do Twojego sprzętu:

# Dla Raspberry Pi 4 z 4GB RAM (zalecany balans):
ollama pull qwen2.5-coder:3b

# Dla Raspberry Pi 4 z 8GB RAM (maksymalna jakość):
ollama pull qwen2.5-coder:7b

# Dla Raspberry Pi 4 z 2GB RAM (podstawowe zastosowania):
ollama pull qwen2.5-coder:1.5b

# Wersja podstawowa (najszybsza):
ollama pull qwen2.5-coder:0.5b
```

#### Krok 5.5: Testowanie modelu
```bash
# Uruchom interaktywny czat z modelem
ollama run qwen2.5-coder:3b "Napisz funkcję w Pythonie do sortowania bąbelkowego"

# Sprawdź dostępne modele
ollama list

# Sprawdź szczegóły modelu
ollama show qwen2.5-coder:3b
```

#### Krok 5.6: Konfiguracja wydajności dla Raspberry Pi 4
Dla optymalnej wydajności na RPi4, utwórz plik konfiguracyjny:

```bash
# Stwórz plik środowiskowy dla Ollama
sudo mkdir -p /etc/systemd/system/ollama.service.d
sudo nano /etc/systemd/system/ollama.service.d/environment.conf
```

Dodaj następującą zawartość:
```ini
[Service]
Environment="OLLAMA_NUM_PARALLEL=2"
Environment="OLLAMA_MAX_LOADED_MODELS=1"
Environment="OLLAMA_CONTEXT_LENGTH=4096"
# Ograniczenie zużycia pamięci dla stabilności
Environment="OLLAMA_KEEP_ALIVE=5m"
```

Zrestartuj usługę:
```bash
sudo systemctl daemon-reload
sudo systemctl restart ollama
```

#### Krok 5.7: Weryfikacja działania
```bash
# Sprawdź czy usługa działa
curl http://localhost:11434/api/tags

# Przykładowe zapytanie API
curl http://localhost:11434/api/generate -d '{
  "model": "qwen2.5-coder:3b",
  "prompt": "Witaj! Jak się masz?",
  "stream": false
}'
```

#### Krok 5.8: Rozwiązywanie typowych problemów

**Problem: Ollama nie startuje na ARM**
```bash
# Sprawdź logi
journalctl -u ollama -f

# Upewnij się, że masz bibliotekę libc6
sudo apt update
sudo apt install -y libc6 libstdc++6
```

**Problem: Brak pamięci podczas ładowania modelu**
```bash
# Zwiększ swap
sudo systemctl stop ollama
sudo nano /etc/dphys-swapfile
# Zmień CONF_SWAPSIZE=8192
sudo dphys-swapfile setup
sudo dphys-swapfile swapon
sudo systemctl start ollama
```

**Problem: Wolna inferencja**
- Użyj mniejszego modelu (np. `qwen2.5-coder:1.5b` zamiast `7b`)
- Zamknij inne aplikacje zużywające RAM
- Upewnij się, że CPU nie jest thermal throttled: `vcgencmd get_throttled`

#### Krok 5.9: Automatyczne uruchamianie przy starcie systemu
Ollama powinna być już skonfigurowana jako usługa systemowa. Sprawdź:

```bash
# Sprawdź czy usługa jest włączona
systemctl is-enabled ollama

# Jeśli nie, włącz ją
sudo systemctl enable ollama

# Restart po rebootzie
sudo reboot
# Po restarcie sprawdź:
systemctl status ollama
```

#### Krok 5.10: Integracja z book-parser
Po skonfigurowaniu Ollama, zaktualizuj plik konfiguracyjny book-parser:

```bash
nano config.json
```

Upewnij się, że zawiera odpowiednie wpisy:
```json
{
  "llm_model": "qwen2.5-coder:3b",
  "llm_endpoint": "http://localhost:11434",
  "llm_provider": "ollama"
}
```

---

**Porady dla Raspberry Pi 4:**
- 🌡️ **Monitoruj temperaturę**: `vcgencmd measure_temp` - powyżej 80°C może występować throttling
- ⚡ **Używaj dobrego zasilacza**: Oficjalny zasilacz 5V/3A jest konieczny dla stabilności
- 💾 **Rozważ SSD przez USB 3.0**: Szybsze niż karta microSD, szczególnie dla operacji I/O
- 🔧 **Undervolting/Overclocking**: Tylko dla zaawansowanych użytkowników z dobrym chłodzeniem

### 6. Konfiguracja n8n
```bash
# Instalacja przez npm
npm install n8n -g
n8n start
# Dostęp przez przeglądarkę: http://localhost:5678
```

### 7. Konfiguracja OpenClaw
```bash
# Postępuj zgodnie z oficjalną dokumentacją OpenClaw
git clone https://github.com/openclaw/openclaw.git 
cd openclaw
# ... dalsza konfiguracja
```

### 9. Instalacja i konfiguracja OfficeCli
**OfficeCli** to zaawansowane narzędzie AI do generowania i manipulacji dokumentami biurowymi z linii poleceń.

#### Instalacja OfficeCli
```bash
# Klonowanie repozytorium OfficeCli
git clone https://github.com/iOfficeAI/OfficeCli.git
cd OfficeCli

# Instalacja zależności
pip install -r requirements.txt

# Instalacja narzędzia
sudo make install

# Weryfikacja instalacji
officecli --version
```

#### Konfiguracja OfficeCli
```bash
# Tworzenie pliku konfiguracyjnego
officecli config init

# Konfiguracja modelu AI (opcjonalne)
officecli config set ai-model qwen2.5-coder:3b
officecli config set ai-endpoint http://localhost:11434

# Ustawienie domyślnego formatu wyjściowego
officecli config set default-format doc
```

#### Przykłady użycia
```bash
# Generowanie dokumentu .doc z chunków
officecli generate \
  --input /workspace/chunks \
  --output /workspace/finish/book.doc \
  --format doc \
  --ai-compose

# Generowanie z konkretnym modelem AI
officecli generate \
  --input /workspace/chunks \
  --output /workspace/finish/book.doc \
  --ai-model qwen2.5-coder:3b \
  --ai-compose
```

> **Więcej informacji**: [Oficjalna dokumentacja OfficeCli](https://github.com/iOfficeAI/OfficeCli)

### 10. Edycja pliku konfiguracyjnego
```bash
cp config.example.json config.json
nano config.json
```

Przykładowa konfiguracja:
```json
{
  "input_folder": "/home/pi/books/input",
  "output_folder": "/home/pi/books/output",
  "chunk_size_tokens": 4096,
  "mempalace_endpoint": "http://localhost:8080",
  "llm_model": "qwen2.5-coder:3b",
  "llm_endpoint": "http://localhost:11434",
  "llm_provider": "ollama",
  "n8n_webhook": "http://localhost:5678/webhook/book-parser",
  "logging_level": "INFO"
}
```

---

## 📖 Użycie

### Tryb podstawowy
```bash
./build/book-parser --process /path/to/input/folder
```

### Tryb z opcjami
```bash
./build/book-parser \
  --input /path/to/books \
  --output /path/to/summaries \
  --chunk-size 4096 \
  --format markdown \
  --verbose
```

### Tryb interaktywny
```bash
./build/book-parser --interactive
```

### Sprawdzenie statusu
```bash
./build/book-parser --status
```

### Czyszczenie cache
```bash
./build/book-parser --clean-cache
```

---

## 📂 Struktura katalogów

```
book-parser/
├── src/                    # Kod źródłowy C++
│   ├── main.cpp
│   ├── parser.cpp
│   ├── chunker.cpp
│   ├── mempalace_client.cpp
│   └── utils.cpp
├── scripts/                # Skrypty Bash
│   ├── convert_docs.sh
│   ├── extract_pdf.sh
│   └── workflow_monitor.sh
├── config/                 # Pliki konfiguracyjne
│   ├── config.json
│   └── logging.conf
├── workflows/              # Workflow n8n (eksport JSON)
│   └── book_parser_workflow.json
├── input/                  # Katalog wejściowy (przykłady)
├── output/                 # Katalog wyjściowy (generowane)
├── chunks/                 # Pliki chunków (.json, .txt)
├── finish/                 # Finalny dokument .doc wygenerowany przez OfficeCli
│   └── book.doc            # Gotowa książka w formacie .doc
├── logs/                   # Logi aplikacji
├── docs/                   # Dodatkowa dokumentacja
├── tests/                  # Testy jednostkowe
├── CMakeLists.txt          # Konfiguracja build
├── LICENSE                 # Licencja Apache 2.0
└── README.md               # Ten plik
```

---

## 🔗 Integracje zewnętrzne

### mempalace
- **Repozytorium**: [github.com/milla-jovovich/mempalace](https://github.com/milla-jovovich/mempalace )
- **Rola**: Lokalna baza wiedzy AI z długoterminową pamięcią
- **API**: RESTful HTTP/JSON
- **Features**:
  - Semantic search
  - Context retrieval
  - Knowledge graph

### Qwen Coder (Alibaba)
- **Model**: Open-source LLM specjalizujący się w kodzie i analizie tekstu
- **Uruchomienie**: llama.cpp / Ollama / LM Studio
- **Zastosowanie**: Generowanie podsumowań, ekstrakcja insightów

### n8n
- **Strona**: [n8n.io](https://n8n.io )
- **Rola**: Workflow automation
- **Możliwości**:
  - Wizualny edytor workflow
  - 200+ integracji
  - Harmonogramowanie zadań
  - Webhooks

### OpenClaw
- **Rola**: Zaawansowana automatyzacja i monitoring
- **Features**:
  - Intelligent retry logic
  - Error handling
  - Performance optimization

### OfficeCli
- **Repozytorium**: [github.com/iOfficeAI/OfficeCli](https://github.com/iOfficeAI/OfficeCli)
- **Rola**: AI-powered generowanie i manipulacja dokumentami biurowymi z linii poleceń
- **Zastosowanie w book-parser**:
  - Finalne składanie chunków w spójny dokument `.doc`
  - Automatyczne generowanie spisu treści
  - Zachowanie formatowania i struktury książki
  - Wsparcie dla metadanych (autor, tytuł, data, ISBN)
- **Integracja z shell**:
  ```bash
  officecli generate \
    --input /workspace/chunks \
    --output /workspace/finish/book.doc \
    --format doc \
    --ai-compose
  ```
- **Features**:
  - AI-powered composition z wykorzystaniem lokalnych modeli LLM
  - Wsparcie dla wielu formatów wyjściowych (.doc, .docx, .pdf, .odt)
  - Batch processing dużych dokumentów
  - Incremental updates istniejących dokumentów
  - Template-based formatting

---

## ⚡ Wydajność i optymalizacja

### Benchmarki na Raspberry Pi 4 (8GB RAM)
| Zadanie | Czas wykonania |
|---------|----------------|
| Konwersja .docx (300 stron) | ~45 sekund |
| Chunkowanie (100k tokenów) | ~12 sekund |
| Wysyłka do mempalace (100 chunków) | ~30 sekund |
| Generowanie podsumowania (Qwen) | ~3-5 minut |

### Optymalizacje
- **Wielowątkowość**: Równoległe przetwarzanie wielu dokumentów
- **Cache**: Buforowanie już przetworzonych chunków
- **Batch processing**: Grupowanie operacji I/O
- **Memory pooling**: Redukcja alokacji pamięci

### Monitoring zasobów
```bash
watch -n 1 'vcgencmd measure_temp; free -h; top -bn1 | grep "Cpu(s)"'
```

---

## 🔒 Bezpieczeństwo i prywatność

### Zalety architektury lokalnej
✅ **Brak wysyłania danych do chmury** — wszystkie dane pozostają na urządzeniu  
✅ **Pełna kontrola** — użytkownik zarządza całym stackiem  
✅ **Szyfrowanie w spoczynku** — możliwość szyfrowania katalogów (LUKS, eCryptFS)  
✅ **Izolacja sieciowa** — możliwość uruchomienia bez dostępu do Internetu  

### Zalecane praktyki bezpieczeństwa
- Regularne aktualizacje systemu (`sudo apt update && sudo apt upgrade`)
- Zmiana domyślnego hasła użytkownika `pi`
- Konfiguracja firewalla (`ufw`)
- Backup danych na zewnętrzny nośnik
- Monitorowanie logów pod kątem anomalii

---

## 🐛 Rozwiązywanie problemów

### Częste błędy i rozwiązania

#### ❌ Błąd: "Cannot find LibreOffice"
**Rozwiązanie**:
```bash
sudo apt install libreoffice-core
```

#### ❌ Błąd: "mempalace connection refused"
**Rozwiązanie**:
```bash
# Sprawdź czy mempalace działa
systemctl status mempalace
# Restart usługi
sudo systemctl restart mempalace
```

#### ❌ Błąd: "Out of memory during chunking"
**Rozwiązanie**:
- Zmniejsz liczbę równoległych wątków w `config.json`
- Dodaj swap: `sudo dphys-swapfile swapoff && sudo dphys-swapfile setup`
- Rozważ upgrade do 8GB RAM

#### ❌ Błąd: "Qwen model not found"
**Rozwiązanie**:
```bash
ollama pull qwen2.5-coder:3b
# Lub sprawdź endpoint w config.json
# Dostępne modele: qwen2.5-coder:0.5b, :1.5b, :3b, :7b
```

### Gdzie szukać logów?
```bash
tail -f logs/book-parser.log
journalctl -u book-parser -f
```

---

## 🚀 Przyszłe rozwinięcia

### Planowane funkcje
- [ ] Wsparcie dla formatów `.epub` i `.mobi`
- [ ] Wielojęzyczne podsumowania (tłumaczenie on-the-fly)
- [ ] Interfejs webowy (React + Flask backend)
- [ ] Integracja z Obsidian/Logseq jako output
- [ ] Tryb incrementalny (tylko nowe/zmienione pliki)
- [ ] Eksport do bazy wiedzy (Notion, Roam Research)
- [ ] Wsparcie dla Raspberry Pi 5 i Jetson Nano
- [ ] Docker container dla łatwiejszej deployowalności

### Roadmap
| Kwartał | Cele |
|---------|------|
| Q2 2025 | Docker support, EPUB parsing |
| Q3 2025 | Web UI, multi-language summaries |
| Q4 2025 | Incremental mode, Notion integration |

---

## 📜 Licencja

Ten projekt jest udostępniany na licencji **Apache License 2.0**.  
Szczegóły znajdziesz w pliku [`LICENSE`](LICENSE).

**Krótko**: Możesz używać, modyfikować i rozpowszechniać ten projekt komercyjnie i niekomercyjnie, pod warunkiem zachowania informacji o licencji i autorach.

---

## 👥 Współtwórcy i kontakt

### Główny autor
- **GitHub**: [@YOUR_USERNAME](https://github.com/bartoszruta26-droid )

### Podziękowania
- Twórcom [mempalace](https://github.com/milla-jovovich/mempalace ) za rewolucyjne podejście do lokalnej pamięci AI
- Zespołowi Alibaba Cloud za udostępnienie modelu Qwen Coder
- Społeczności n8n i OpenClaw za narzędzia automatyzacji

### Jak pomóc?
1. **Forknij** repozytorium
2. Stwórz branch feature'owy (`git checkout -b feature/amazing-feature`)
3. **Commitnij** zmiany (`git commit -m 'Add amazing feature'`)
4. **Pushnij** (`git push origin feature/amazing-feature`)
5. Otwórz **Pull Request**

### Kontakt
- **Issues**: [GitHub Issues](https://github.com/bartoszruta26-droid/book-parser/issues )
- **Discussions**: [GitHub Discussions](https://github.com/bartoszruta26-droid/book-parser/discussions )
- **Email**: your.email@example.com

---

<div align="center">

**⭐ Jeśli ten projekt Ci pomógł, zostaw gwiazdkę! ⭐**

Made with ❤️ on Raspberry Pi 4

---



</div>

---

## 🆕 Ollama Content Expander

Nowy moduł systemu wykorzystujący lokalny LLM przez Ollama do generowania rozszerzonych, spójnych treści na podstawie przetworzonych chunków.

### Funkcje

- **Lokalne LLM**: Integracja z Ollama API (wsparcie dla Qwen Coder, Llama 3.2, i innych)
- **Wielowątkowa ekspansja**: Łączenie informacji z wielu chunków w spójną całość
- **Dostosowanie stylu**: 6 predefiniowanych stylów (akademicki, dziennikarski, techniczny, kreatywny, biznesowy, nieformalny)
- **Kontrola długości**: 4 poziomy długości outputu (~250 do ~2000+ słów)
- **Wsparcie wielojęzyczne**: Output w dowolnym języku obsługiwanym przez model
- **Formaty wyjściowe**: JSON (z metadanymi) lub plain text

### Budowa

```bash
make expander
```

### Użycie

```bash
# Podstawowe
./ollama_expander -i ./chunk/ -o expanded.json

# Z wybranym modelem i stylem
./ollama_expander -i ./chunk/ -o output.txt -t -m llama3.2 -s creative -l long --lang en

# Sprawdzenie dostępności Ollama
./ollama_expander --check

# Lista dostępnych modeli
./ollama_expander --list-models
```

### Opcje

| Opcja | Opis | Domyślnie |
|-------|------|-----------|
| `-i, --input PATH` | Ścieżka do pliku/katalogu z chunkami | `./chunk` |
| `-o, --output PATH` | Ścieżka wyjściowa | `./output/expanded.json` |
| `-t, --text` | Zapis jako plain text zamiast JSON | false |
| `-m, --model MODEL` | Model Ollama | `qwen2.5-coder:7b` |
| `-s, --style STYLE` | Styl: academic/journalistic/technical/creative/business/casual | `technical` |
| `-l, --length LENGTH` | Długość: short/medium/long/very_long | `medium` |
| `--lang LANGUAGE` | Język outputu | `pl` |
| `--temp VALUE` | Temperatura (0.0-2.0) | `0.7` |
| `--max-chunks N` | Maksymalna liczba chunków | `10` |

### Przykłady

```bash
# Ekspansja w stylu akademickim po polsku
./ollama_expander -i ./chunk/ -s academic -l long --lang pl

# Ekspansja w stylu dziennikarskim po angielsku
./ollama_expander -i ./chunk/ -s journalistic --lang en -m llama3.2

# Krótka notatka techniczna
./ollama_expander -i ./chunk/sample_chunk_0.json -l short -s technical
```

### Pipeline z ekspansją

```bash
# Uruchomienie pełnego pipeline z ekspansją
./run_pipeline.sh -e -s creative -l long

# Z innym modelem
./run_pipeline.sh -e -m llama3.2 --lang en
```

### Wymagania

- Uruchomiona usługa Ollama: `ollama serve`
- Pobrany model: `ollama pull qwen2.5-coder:7b`
- Biblioteki: libcurl4-openssl-dev, nlohmann-json3-dev


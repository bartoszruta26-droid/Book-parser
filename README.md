# 📚 Book-Parser

> **Inteligentny system przetwarzania książek z wykorzystaniem lokalnych modeli AI na Raspberry Pi 4**

Book-parser to zaawansowane narzędzie napisane w **C++** i **Bash**, które automatycznie przetwarza dokumenty książkowe, dzieli je na segmenty, zapisuje kontekst w lokalnej bazie wiedzy AI (mempalace), a następnie generuje spójne podsumowania przy użyciu modelu Qwen Coder. Cały proces jest orchestrowany przez agentów AI: **n8n** oraz **OpenClaw**.

---

## 📖 Spis treści

- [Opis projektu](#-opis-projektu)
- [Kluczowe funkcje](#-kluczowe-funkcje)
- [Architektura systemu](#-architektura-systemu)
- [Wymagania sprzętowe i programowe](#-wymagania-sprzętowe-i-programowe)
- [Obsługiwane formaty plików](#-obsługiwane-formaty-plików)
- [Szczegółowy przepływ pracy](#-szczegółowy-przepływ-pracy)
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

## 🔄 Szczegółowy przepływ pracy

### Krok 1: Skanowanie katalogu wejściowego
```bash
./book-parser --scan /path/to/books
```
- Rekurencyjne przeszukiwanie katalogów
- Identyfikacja obsługiwanych formatów
- Walidacja integralności plików
- Tworzenie kolejki zadań

### Krok 2: Konwersja i ekstrakcja tekstu
- Uruchomienie LibreOffice w trybie headless dla formatów .doc/.xls
- Użycie `pdftotext` dla plików PDF
- Bezpośrednie czytanie dla .txt i .odt
- Normalizacja kodowania (UTF-8)

### Krok 3: Chunkowanie tekstu
- Podział na segmenty 4096 tokenów
- Analiza struktury dokumentu (nagłówki, rozdziały)
- Dodawanie metadanych kontekstowych:
  ```json
  {
    "chunk_id": "book_001_chunk_042",
    "source_file": "example_book.pdf",
    "page_range": "210-215",
    "current_chapter": "Rozdział 5: Architektura AI",
    "prev_chapter": "Rozdział 4: Sieci neuronowe",
    "next_chapter": "Rozdział 6: Uczenie głębokie",
    "current_subchapter": "5.2 Transformery",
    "prev_subchapter": "5.1 RNN i LSTM",
    "next_subchapter": "5.3 Attention Mechanisms"
  }
  ```

### Krok 4: Wysyłka do mempalace
- Każdy chunk jest wysyłany jako osobny wpis
- Indeksowanie po metadanych
- Tworzenie relacji między chunkami
- Budowanie grafu wiedzy

### Krok 5: Analiza przez Qwen Coder
- Odpytanie mempalace o pełny kontekst książki
- Generowanie spójnego podsumowania
- Ekstrakcja kluczowych koncepcji
- Identyfikacja głównych wątków

### Krok 6: Zapis wyników
- Podsumowania zapisywane w katalogu wyjściowym
- Format: `.txt` lub `.md` (Markdown)
- Opcjonalnie: eksport do `.pdf` lub `.html`

### Krok 7: Orchestrowanie przez n8n i OpenClaw
- **n8n**:
  - Harmonogramowanie zadań (cron-like)
  - Powiadomienia (email, Telegram, Discord)
  - Integracja z zewnętrznymi API
- **OpenClaw**:
  - Monitorowanie postępów
  - Retry logic dla nieudanych zadań
  - Dynamiczne skalowanie priorytetów

### Krok 8: Generowanie finalnego dokumentu .doc z OfficeCli
- **Integracja z [OfficeCli](https://github.com/iOfficeAI/OfficeCli)**:
  - Wywołanie z poziomu shell: `officecli generate --input /workspace/chunks --output /workspace/finish/book.doc`
  - AI-powered składanie chunków w spójny dokument
  - Zachowanie formatowania i struktury oryginalnej książki
  - Automatyczne generowanie spisu treści
  - Wsparcie dla metadanych (autor, tytuł, data)

---

### 📦 Output końcowy

> **Na koniec w katalogu `/finish` będzie poskładana książka w formacie `.doc` z wykorzystaniem [iOfficeAI OfficeCli](https://github.com/iOfficeAI/OfficeCli).**
> 
> Przykładowe wywołanie z shell:
> ```bash
> officecli generate \
>   --input /workspace/chunks \
>   --output /workspace/finish/book.doc \
>   --format doc \
>   --ai-compose
> ```

---

## 🛠️ Instalacja i konfiguracja

### 1. Klonowanie repozytorium
```bash
git clone https://github.com/YOUR_USERNAME/book-parser.git 
cd book-parser
```

### 2. Instalacja zależności systemowych
```bash
sudo apt update
sudo apt install -y \
    build-essential \
    g++ \
    libreoffice \
    poppler-utils \
    pandoc \
    git \
    curl
```

### 3. Kompilacja projektu
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 4. Konfiguracja mempalace
```bash
# Sklonuj i skonfiguruj mempalace
git clone https://github.com/milla-jovovich/mempalace.git 
cd mempalace
# Postępuj zgodnie z instrukcjami instalacji mempalace
```

### 5. Konfiguracja Qwen Coder
```bash
# Przykład z użyciem Ollama
curl -fsSL https://ollama.ai/install.sh  | sh
ollama pull qwen-coder
```

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

### 8. Instalacja i konfiguracja OfficeCli
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
officecli config set ai-model qwen-coder
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
  --ai-model qwen-coder \
  --ai-compose

# Podgląd dostępnych opcji
officecli generate --help
```

> **Więcej informacji**: [Oficjalna dokumentacja OfficeCli](https://github.com/iOfficeAI/OfficeCli)

### 9. Edycja pliku konfiguracyjnego
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
  "llm_model": "qwen-coder",
  "llm_endpoint": "http://localhost:11434",
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
ollama pull qwen-coder
# Lub sprawdź endpoint w config.json
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
- **GitHub**: [@YOUR_USERNAME](https://github.com/YOUR_USERNAME )

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
- **Issues**: [GitHub Issues](https://github.com/YOUR_USERNAME/book-parser/issues )
- **Discussions**: [GitHub Discussions](https://github.com/YOUR_USERNAME/book-parser/discussions )
- **Email**: your.email@example.com

---

<div align="center">

**⭐ Jeśli ten projekt Ci pomógł, zostaw gwiazdkę! ⭐**

Made with ❤️ on Raspberry Pi 4

---



</div>

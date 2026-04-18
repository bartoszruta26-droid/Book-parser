# Document Chunker

Inteligentny system do chunkowania dokumentów napisany w **C++** (bez zależności od Pythona).

## Funkcjonalności

### Inteligentne wykrywanie dokumentów
- ✅ Automatyczne skanowanie wskazanych katalogów
- ✅ Obsługa wielu formatów jednocześnie (.txt, .md, .json)
- ✅ Filtrowanie duplikatów i uszkodzonych plików
- ✅ Logowanie postępów w czasie rzeczywistym

### Zaawansowane chunkowanie
- ✅ **Precyzyjne dzielenie**: 4096 tokenów na chunk (~5 stron A4)
- ✅ **Zachowanie granic semantycznych**: algorytm stara się nie przerywać zdań ani akapitów
- ✅ **Metadane kontekstowe**: każdy chunk zawiera informacje o:
  - Poprzednim i następnym rozdziale
  - Poprzednim i następnym podrozdziale
  - Poprzednim i następnym pod-podrozdziale
  - Tytułach i podtytułach
  - Numeracji stron
  - Źródłowym pliku

## Struktura projektu

```
.
├── src/                # Kod źródłowy C++
│   └── chunker.cpp     # Główny moduł chunkera
├── input/              # Pliki wejściowe do przetworzenia
├── chunk/              # Wynikowe pliki po chunkowaniu
│   ├── *.txt           # Zawartość chunków
│   └── *.json          # Metadane kontekstowe chunków
├── logs/               # Logi aplikacji
│   └── chunker.log     # Szczegółowe logi procesu
├── Makefile            # Plik budowania
├── chunker             # Skompilowany binary (po kompilacji)
└── README.md           # Dokumentacja
```

## Wymagania

- Kompilator C++ z obsługą **C++17** (g++ 8+ lub clang++ 7+)
- System Linux/Unix lub macOS (dla Windows użyj WSL)

## Instalacja i kompilacja

### Opcja 1: Użycie Makefile (zalecane)

```bash
# Kompilacja
make

# Uruchomienie testów
make test

# Instalacja w systemie
sudo make install

# Czyszczenie projektu
make clean
```

### Opcja 2: Ręczna kompilacja

```bash
g++ -std=c++17 -O2 -Wall -Wextra -o chunker src/chunker.cpp
```

## Użycie

### Podstawowe

```bash
./chunker
```

Domyślne ustawienia:
- Katalog wejściowy: `./input`
- Katalog wyjściowy: `./chunk`
- Katalog logów: `./logs`
- Rozmiar chunka: 4096 tokenów

### Zaawansowane opcje

```bash
# Tryb szczegółowy z verbose loggingiem
./chunker -v

# Niestandardowe katalogi
./chunker -i /path/to/input -o /path/to/output -l /path/to/logs

# Zmiana rozmiaru chunka (np. 8192 tokenów)
./chunker --chunk-size 8192

# Pełna lista opcji
./chunker --help
```

### Wszystkie dostępne flagi

| Flaga | Opis | Domyślna wartość |
|-------|------|------------------|
| `-i, --input DIR` | Katalog z plikami wejściowymi | `./input` |
| `-o, --output DIR` | Katalog na wyniki chunkowania | `./chunk` |
| `-l, --logs DIR` | Katalog na pliki logów | `./logs` |
| `-s, --chunk-size N` | Rozmiar chunka w tokenach | `4096` |
| `-v, --verbose` | Tryb szczegółowy (logi na stdout) | `false` |
| `-h, --help` | Wyświetlenie pomocy | - |

## Przykład działania

### 1. Przygotuj pliki wejściowe

Umieszczasz pliki `.txt`, `.md` lub `.json` w katalogu `input/`:

```bash
echo "# Rozdział 1\nTo jest przykładowy tekst..." > input/dokument.txt
```

### 2. Uruchom chunker

```bash
./chunker -v
```

### 3. Sprawdź wyniki

W katalogu `chunk/` pojawią się pary plików:

- `dokument_chunk_0.txt` - zawartość pierwszego chunka
- `dokument_chunk_0.json` - metadane z kontekstem

### Przykład metadanych JSON

```json
{
  "source_file": "dokument.txt",
  "chunk_index": 0,
  "total_chunks": 3,
  "token_count": 4089,
  "page_number": 1,
  "timestamp": "2026-04-18T12:00:00",
  "context": {
    "previous_chapter": "",
    "next_chapter": "Rozdział 2: Wprowadzenie",
    "previous_subchapter": "",
    "next_subchapter": "Podrozdział 1.1",
    "previous_subsubchapter": "",
    "next_subsubchapter": "",
    "title": "Rozdział 1",
    "subtitle": ""
  },
  "content": "Treść chunka..."
}
```

## Formaty wyjściowe

### Pliki .txt
Czysta zawartość tekstu chunka, gotowa do dalszego przetwarzania.

### Pliki .json
Strukturalne metadane zawierające:
- Informacje o źródłowym pliku
- Indeks chunka i całkowita liczba chunków
- Liczbę tokenów
- Kontekst strukturalny (rozdziały, podrozdziały)
- Numerację stron
- Timestamp przetworzenia
- Pełną zawartość chunka

## Logowanie

Szczegółowe logi są zapisywane w `logs/chunker.log`:

- Start i koniec sesji
- Lista przetwarzanych plików
- Wykryte rozdziały
- Liczba utworzonych chunków
- Wykryte duplikaty
- Błędy i ostrzeżenia

## Algorytm chunkowania

1. **Skanowanie katalogu** - automatyczne wykrywanie plików
2. **Hashowanie** - obliczanie hash dla detekcji duplikatów
3. **Detekcja struktury** - wykrywanie nagłówków rozdziałów (#, ##, ###)
4. **Segmentacja na zdania** - inteligentne dzielenie tekstu
5. **Grupowanie w chunki** - łączenie zdań do osiągnięcia limitu tokenów
6. **Generowanie metadanych** - tworzenie kontekstu strukturalnego
7. **Zapis wyników** - eksport do .txt i .json

## Porównanie z wersją Python

| Cecha | C++ | Python |
|-------|-----|--------|
| Wydajność | ⚡ Bardzo wysoka | 🐌 Średnia |
| Zależności | ❌ Brak | ✅ Wymagany Python |
| Kompilacja | ✅ Wymagana | ❌ Nie wymaga |
| Wielowątkowość | ✅ Łatwa implementacja | ⚠️ GIL ogranicza |
| Rozmiar binary | ~50 KB | + interpreter ~3 MB |
| Portability | ✅ Binary na dane arch. | ✅ Skrypt uniwersalny |

## Rozwiązywanie problemów

### Błąd kompilacji "filesystem not found"
Upewnij się, że używasz C++17:
```bash
g++ -std=c++17 ...
```

### Brak plików w output
Sprawdź czy katalog `input/` istnieje i zawiera pliki:
```bash
ls -la input/
```

### Duplikaty są pomijane
To oczekiwane zachowanie. Hash pliku jest identyczny.

## License

MIT License - używaj dowolnie.

## Autor

Wygenerowano jako alternatywa C++ dla wersji Python.

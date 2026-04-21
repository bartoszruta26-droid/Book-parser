# 📖 Parser EPUB - Instrukcja Użycia

## Opis

Parser EPUB to narzędzie do konwersji plików EPUB na formaty TXT i JSON, które mogą być dalej przetwarzane przez system Book-Parser.

## Instalacja zależności

```bash
pip3 install ebooklib lxml
```

## Użycie jako standalone

### Konwersja pojedynczego pliku EPUB:

```bash
python3 src/epub_parser.py książka.epub -o ./output -f both -v
```

### Konwersja całego katalogu z plikami EPUB:

```bash
python3 src/epub_parser.py ./moje_ksiazki/ -o ./output -f both -v
```

### Opcje:

- `input` - Plik EPUB lub katalog z plikami EPUB (wymagane)
- `-o, --output DIR` - Katalog wyjściowy dla przekonwertowanych plików
- `-f, --format {txt,json,both}` - Format outputu (domyślnie: txt)
- `-v, --verbose` - Tryb szczegółowy z dodatkowymi informacjami

## Integracja z Pipeline

Parser EPUB jest zintegrowany z głównym skryptem `run_pipeline.sh`.

### Automatyczna konwersja przed pipeline:

```bash
./run_pipeline.sh --epub ./epub_input -e
```

Skrypt automatycznie:
1. Wykryje pliki EPUB w katalogu `./epub_input`
2. Przekonwertuje je na TXT i JSON
3. Skopiuje pliki TXT do katalogu `./input`
4. Kontynuuje standardowe przetwarzanie (chunking, ekspansja, itp.)

### Tylko konwersja EPUB:

```bash
./run_pipeline.sh --epub ./epub_input --epub-only
```

Ta komenda wykonuje wyłącznie konwersję EPUB bez dalszego przetwarzania.

### Opcje pipeline dla EPUB:

- `--epub DIR` - Katalog z plikami EPUB do konwersji (domyślnie: ./epub_input)
- `--convert-dir DIR` - Katalog na przekonwertowane pliki (domyślnie: ./converted)
- `--epub-only` - Tylko konwersja EPUB i zakończenie

## Struktura outputu

### Format TXT:

```
# Tytuł Książki

Autor: Imię Nazwisko


## Rozdział 1

Treść rozdziału...

## Rozdział 2

Treść rozdziału...
```

### Format JSON:

```json
{
  "metadata": {
    "title": "Tytuł Książki",
    "creator": "Imię Nazwisko",
    "language": "pl",
    "publisher": null,
    "date": null,
    "identifier": "isbn-123",
    "description": null
  },
  "chapters": [
    {
      "index": 0,
      "title": "Rozdział 1",
      "content": "Treść rozdziału...",
      "word_count": 1500,
      "char_count": 8500
    }
  ],
  "total_chapters": 10
}
```

## Przykłady użycia

### Przykład 1: Konwersja jednej książki

```bash
# Umieść plik EPUB w katalogu epub_input
cp moja_ksiazka.epub ./epub_input/

# Uruchom konwersję
./run_pipeline.sh --epub ./epub_input --epub-only

# Sprawdź wyniki
ls -la ./converted/
cat ./converted/moja_ksiazka.txt
```

### Przykład 2: Pełne przetwarzanie książki EPUB

```bash
# Umieść plik EPUB w katalogu epub_input
cp moja_ksiazka.epub ./epub_input/

# Uruchom pełne przetwarzanie z ekspansją AI
./run_pipeline.sh --epub ./epub_input -e -s creative -l long

# Sprawdź wyniki w katalogu finish
ls -la ./finish/
```

### Przykład 3: Konwersja wielu książek

```bash
# Umieść wiele plików EPUB w katalogu
cp *.epub ./epub_input/

# Przekonwertuj wszystkie
python3 src/epub_parser.py ./epub_input/ -o ./converted -f both -v

# Lub użyj pipeline
./run_pipeline.sh --epub ./epub_input --epub-only
```

## Rozwiązywanie problemów

### Problem: "Unicode strings with encoding declaration are not supported"

To ostrzeżenie nie wpływa na działanie parsera. Wynika z różnic w parsingu HTML między różnymi wersjami lxml. Parser automatycznie używa fallbackowej metody parsowania.

### Problem: Brak wykrytych rozdziałów

Upewnij się, że plik EPUB zawiera poprawną strukturę HTML z tagami `<h1>`, `<h2>`, `<h3>` dla nagłówków rozdziałów.

### Problem: Pusty output

Sprawdź czy plik EPUB nie jest zabezpieczony DRM. Parser obsługuje tylko pliki EPUB bez zabezpieczeń.

## Wymagania

- Python 3.8+
- ebooklib
- lxml

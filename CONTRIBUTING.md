# 🤝 Contributing do Book-Parser

Dziękujemy za zainteresowanie współtworzeniem Book-Parser! Ten dokument zawiera wskazówki jak możesz przyczynić się do rozwoju projektu.

## 📋 Spis treści

1. [Jak mogę pomóc?](#jak-mogę-pomóc)
2. [Zgłaszanie błędów](#zgłaszanie-błędów)
3. [Propozycje funkcji](#propozycje-funkcji)
4. [Pull Requests](#pull-requests)
5. [Standardy kodu](#standardy-kodu)
6. [Testowanie](#testowanie)
7. [Dokumentacja](#dokumentacja)

---

## 🌟 Jak mogę pomóc?

Możesz przyczynić się na wiele sposobów:

- **🐛 Zgłaszanie błędów** - Pomóż nam znaleźć i naprawiać problemy
- **💡 Propozycje funkcji** - Podziel się pomysłami na ulepszenia
- **📝 Dokumentacja** - Poprawiaj istniejącą lub twórz nową dokumentację
- **🧪 Testowanie** - Testuj nowe funkcje na różnych konfiguracjach
- **💻 Kod** - Pisząc nowy kod lub refaktoryzując istniejący
- **🌍 Tłumaczenia** - Pomóż przetłumaczyć dokumentację na inne języki
- **📢 Promocja** - Polecaj projekt innym użytkownikom

---

## 🐛 Zgłaszanie błędów

Przed zgłoszeniem błędu:
1. Sprawdź czy problem nie został już zgłoszony w [Issues](https://github.com/bartoszruta26-droid/book-parser/issues)
2. Upewnij się że używasz najnowszej wersji
3. Sprawdź logi w katalogu `logs/`

### Dobry raport o błędzie powinien zawierać:

- **Krótki opis** - Co się stało?
- **Kroki do reprodukcji** - Jak odtworzyć problem?
- **Oczekiwany rezultat** - Co powinno się stać?
- **Rzeczywisty rezultat** - Co się stało naprawdę?
- **Środowisko**:
  - System operacyjny i wersja
  - Wersja Book-Parser
  - Wersja Ollama i model
  - Ilość RAM
- **Logi** - Fragmenty logów z błędami
- **Screeny** - Jeśli dotyczy

### Template zgłoszenia błędu:

```markdown
**Opis błędu:**
[Krótki opis problemu]

**Kroki do reprodukcji:**
1. Uruchom komendę '...'
2. Użyj pliku '...'
3. Zobacz błąd

**Oczekiwany rezultat:**
[Co powinno się stać]

**Rzeczywisty rezultat:**
[Co się stało]

**Środowisko:**
- OS: [np. Raspberry Pi OS 64-bit]
- Book-Parser version: [np. 1.0.0]
- Ollama version: [np. 0.1.32]
- Model: [np. qwen2.5-coder:3b]
- RAM: [np. 4GB]

**Logi:**
```
[Wklej fragment logów]
```

**Dodatkowe informacje:**
[Wszystko co może pomóc]
```

---

## 💡 Propozycje funkcji

Chcesz dodać nową funkcję? Najpierw:

1. Sprawdź czy podobna funkcja nie jest już planowana
2. Opisz dokładnie co chcesz osiągnąć
3. Wyjaśnij dlaczego ta funkcja jest potrzebna

### Dobra propozycja funkcji powinna zawierać:

- **Problem** - Jaki problem rozwiązuje ta funkcja?
- **Rozwiązanie** - Jak ma działać?
- **Alternatywy** - Czy są inne sposoby rozwiązania?
- **Przykłady użycia** - Jak użytkownicy będą z tego korzystać?
- **Wpływ na wydajność** - Czy spowolni działanie?

---

## 🔀 Pull Requests

### Przed wysłaniem PR:

1. **Forkuj repozytorium** i stwórz branch z swojej zmiany
   ```bash
   git checkout -b feature/nazwa-funkcji
   # lub
   git checkout -b fix/nazwa-błędu
   ```

2. **Upewnij się że kod działa**
   - Przetestuj lokalnie
   - Uruchom istniejące testy
   - Dodaj nowe testy jeśli to konieczne

3. **Aktualizuj dokumentację**
   - README.md jeśli zmienia się funkcjonalność
   - Komentarze w kodzie
   - Przykłady użycia

4. **Sprawdź styl kodu**
   - C++: zgodny z resztą projektu
   - Bash: shellcheck
   - Python: flake8 lub black

5. **Commit message**
   ```
   feat: dodano obsługę formatu EPUB
   
   - Parser EPUB wykorzystujący ebooklib
   - Ekstrakcja metadanych z książki
   - Obsługa rozdziałów i podrozdziałów
   
   Closes #123
   ```

### Typy commit messages:

- `feat:` - nowa funkcja
- `fix:` - naprawa błędu
- `docs:` - zmiana dokumentacji
- `style:` - formatowanie, brak zmian w działaniu
- `refactor:` - refaktoryzacja kodu
- `test:` - dodanie testów
- `chore:` - narzędzia, build, zależności

### Proces review:

1. Maintainer sprawdzi Twój PR
2. Może poprosić o zmiany
3. Po akceptacji zostanie połączony z main

---

## 📏 Standardy kodu

### C++

- Używaj C++17 lub nowszego
- Nazewnictwo:
  - Klasy: `PascalCase`
  - Funkcje: `snake_case`
  - Zmienne: `snake_case`
  - Stałe: `UPPER_SNAKE_CASE`
- Komentarze w języku polskim lub angielskim
- Unikaj magic numbers - używaj named constants

```cpp
// ✅ Dobrze
class DocumentChunker {
    size_t max_chunk_size = 4096;
    
public:
    std::vector<Chunk> split_document(const std::string& text);
};

// ❌ Źle
class documentchunker {
    size_t x = 4096;
    
public:
    std::vector<Chunk> SplitDocument(std::string t);
};
```

### Bash

- Używaj `shellcheck` do walidacji
- Zawsze cytuj zmienne: `"$var"` nie `$var`
- Używaj `set -e` dla exit on error
- Funkcje w `snake_case`

```bash
#!/bin/bash
set -e

# ✅ Dobrze
process_file() {
    local file_path="$1"
    if [[ -f "$file_path" ]]; then
        echo "Processing: $file_path"
    fi
}

# ❌ Źle
ProcessFile() {
    file_path=$1
    if [ -f $file_path ]; then
        echo Processing: $file_path
    fi
}
```

### Python

- PEP 8 style guide
- Type hints gdzie to możliwe
- Docstrings dla funkcji publicznych

```python
# ✅ Dobrze
def parse_epub(file_path: Path) -> Dict[str, Any]:
    """
    Parse EPUB file and extract chapters.
    
    Args:
        file_path: Path to EPUB file
        
    Returns:
        Dictionary with chapters and metadata
    """
    pass

# ❌ Źle
def parseEpub(p):
    pass
```

---

## 🧪 Testowanie

### Testy manualne

Przed wysłaniem PR przetestuj:

1. **Podstawowa funkcjonalność**
   ```bash
   ./run_pipeline.sh -i ./input
   ```

2. **Z ekspansją**
   ```bash
   ./run_pipeline.sh -i ./input -e -n 2
   ```

3. **Różne formaty**
   - .txt
   - .md
   - .epub (jeśli dotyczy)

4. **Edge cases**
   - Pusty plik
   - Bardzo duży plik
   - Plik ze specjalnymi znakami

### Testy automatyczne

Jeśli dodajesz nową funkcję, dodaj testy:

```bash
make test
make test-mempalace
make test-expander
make test-docgenerator
```

---

## 📚 Dokumentacja

### README.md

Aktualizuj gdy:
- Dodajesz nową funkcję
- Zmieniasz sposób instalacji
- Dodajesz nowe zależności
- Zmieniasz interfejs CLI

### INSTALL.md

Aktualizuj gdy:
- Zmieniasz zależności systemowe
- Dodajesz nowe kroki instalacji
- Aktualizujesz minimalne wymagania

### Komentarze w kodzie

- Wyjaśniaj **dlaczego** a nie **co**
- Dokumentuj nietrywialne algorytmy
- Dodawaj przykłady użycia dla złożonych funkcji

---

## 🎯 Obszary do rozwoju

Szczególnie poszukujemy pomocy w:

1. **Obsługa nowych formatów**
   - MOBI
   - FB2
   - RTF

2. **Optymalizacja**
   - Mniejsze zużycie pamięci
   - Szybsze chunkowanie
   - Równoległe przetwarzanie

3. **Integracje**
   - Więcej modeli LLM
   - Inne systemy AI memory
   - Narzędzia workflow

4. **UI/UX**
   - Web UI
   - CLI z lepszym outputem
   - Progress bars

5. **Testy**
   - Unit tests
   - Integration tests
   - CI/CD pipeline

---

## 📞 Kontakt

Masz pytania? Skontaktuj się z nami:

- **GitHub Issues**: [book-parser/issues](https://github.com/bartoszruta26-droid/book-parser/issues)
- **Discussions**: [book-parser/discussions](https://github.com/bartoszruta26-droid/book-parser/discussions)

---

## 🙏 Podziękowania

Dziękujemy wszystkim contributorom! Każdy wkład, niezależnie od wielkości, jest cenny.

Special thanks dla:
- Społeczności Raspberry Pi
- Twórców Ollama
- Projektów open-source na których bazujemy

---

## 📜 Licencja

Przez dodanie swojego kodu zgadzasz się na jego dystrybucję na warunkach licencji projektu (zobacz [LICENSE](LICENSE)).

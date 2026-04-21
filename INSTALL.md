# 📦 Instalacja Book-Parser

Ten dokument zawiera szczegółowe instrukcje instalacji wszystkich zależności i narzędzi wymaganych do działania Book-Parser.

## 📋 Spis treści

1. [Wymagania wstępne](#wymagania-wstępne)
2. [Instalacja zależności systemowych](#instalacja-zależności-systemowych)
3. [Instalacja Python i zależności](#instalacja-python-i-zależności)
4. [Kompilacja narzędzi C++](#kompilacja-narzędzi-c)
5. [Instalacja Ollama](#instalacja-ollama)
6. [Instalacja OfficeCli (opcjonalne)](#instalacja-officecli-opcjonalne)
7. [Instalacja n8n (opcjonalne)](#instalacja-n8n-opcjonalne)
8. [Weryfikacja instalacji](#weryfikacja-instalacji)

---

## 🔧 Wymagania wstępne

- **System operacyjny**: Raspberry Pi OS (64-bit), Ubuntu Server ARM, lub inny Linux
- **Architektura**: ARM64 (zalecane) lub x86_64
- **RAM**: Minimum 2GB (zalecane 4-8GB)
- **Dysk**: Minimum 10GB wolnego miejsca
- **Połączenie internetowe**: Do pobrania modeli AI i zależności

---

## 1️⃣ Instalacja zależności systemowych

### Debian/Ubuntu/Raspberry Pi OS

```bash
sudo apt update
sudo apt upgrade -y

sudo apt install -y \
    build-essential \
    g++ \
    cmake \
    make \
    git \
    curl \
    wget \
    python3 \
    python3-pip \
    python3-venv \
    libreoffice \
    poppler-utils \
    pandoc \
    libcurl4-openssl-dev \
    nlohmann-json3-dev
```

### Fedora/CentOS/RHEL

```bash
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    curl \
    wget \
    python3 \
    python3-pip \
    libreoffice \
    poppler-utils \
    pandoc \
    libcurl-devel \
    nlohmann_json
```

### Arch Linux/Manjaro

```bash
sudo pacman -Syu --noconfirm
sudo pacman -S --noconfirm \
    base-devel \
    gcc \
    cmake \
    make \
    git \
    curl \
    wget \
    python \
    python-pip \
    libreoffice-fresh \
    poppler \
    pandoc \
    curl \
    nlohmann-json
```

---

## 2️⃣ Instalacja Python i zależności

### Tworzenie wirtualnego środowiska (zalecane)

```bash
cd /path/to/book-parser

# Tworzenie wirtualnego środowiska
python3 -m venv venv

# Aktywacja środowiska
source venv/bin/activate

# Aktualizacja pip
pip install --upgrade pip
```

### Instalacja zależności Python

```bash
# Z pliku requirements.txt
pip install -r requirements.txt

# Lub ręcznie:
pip install ebooklib lxml
```

**Główne zależności Python:**
- `ebooklib` >= 0.18 - Parsowanie plików EPUB
- `lxml` >= 5.0.0 - Przetwarzanie XML/HTML

---

## 3️⃣ Kompilacja narzędzi C++

### Kompilacja wszystkich narzędzi

```bash
cd /path/to/book-parser

# Kompilacja
make all

# Lub pojedynczo:
make chunker
make mempalace
make expander
make docgenerator
```

### Instalacja globalna (opcjonalne)

```bash
sudo make install
```

To zainstaluje narzędzia w `/usr/local/bin`:
- `chunker` - Dzielenie dokumentów na segmenty
- `mempalace_client` - Klient mempalace AI memory
- `ollama_expander` - Ekspansja treści przez Ollama
- `doc_generator` - Generator dokumentów .doc

### Wymagania dla kompilacji C++

Narzędzia C++ wymagają:
- **g++** z obsługą C++17 lub nowszego
- **libcurl** - Do komunikacji HTTP/HTTPS
- **nlohmann/json** - Do parsowania JSON

Jeśli napotkasz błędy kompilacji związane z JSON:

```bash
# Debian/Ubuntu
sudo apt install -y nlohmann-json3-dev

# Fedora
sudo dnf install -y nlohmann_json

# Arch Linux
sudo pacman -S nlohmann-json

# Lub pobierz bezpośrednio:
git clone https://github.com/nlohmann/json.git
cd json
mkdir build && cd build
cmake ..
sudo make install
```

---

## 4️⃣ Instalacja Ollama

### Automatyczna instalacja (oficjalny skrypt)

```bash
curl -fsSL https://ollama.com/install.sh | sh
```

### Ręczna instalacja dla ARM64 (Raspberry Pi)

```bash
# Pobranie
wget https://github.com/ollama/ollama/releases/latest/download/ollama-linux-arm64.tgz

# Rozpakowanie
tar -xzf ollama-linux-arm64.tgz

# Przeniesienie do PATH
sudo mv ollama /usr/local/bin/
sudo chmod +x /usr/local/bin/ollama
```

### Uruchomienie usługi

```bash
# Jako usługa systemd
sudo systemctl start ollama
sudo systemctl enable ollama

# Lub w tle
ollama serve &
```

### Pobranie modelu Qwen 2.5 Coder

```bash
# Wybierz model odpowiedni do Twojego sprzętu:

# Dla 2GB RAM (podstawowe):
ollama pull qwen2.5-coder:1.5b

# Dla 4GB RAM (zalecany balans):
ollama pull qwen2.5-coder:3b

# Dla 8GB+ RAM (maksymalna jakość):
ollama pull qwen2.5-coder:7b
```

### Weryfikacja

```bash
ollama --version
ollama list
```

---

## 5️⃣ Instalacja OfficeCli (opcjonalne)

OfficeCli jest opcjonalne - jeśli nie jest zainstalowane, skrypt użyje wbudowanego `doc_generator`.

```bash
# Klonowanie repozytorium
git clone https://github.com/iOfficeAI/OfficeCli.git
cd OfficeCli

# Instalacja zależności
pip install -r requirements.txt

# Instalacja narzędzia
sudo make install

# Weryfikacja
officecli --version
```

---

## 6️⃣ Instalacja n8n (opcjonalne)

n8n służy do orchestrowania workflow i jest opcjonalne.

```bash
# Instalacja przez npm
npm install n8n -g

# Uruchomienie
n8n start

# Dostęp przez przeglądarkę: http://localhost:5678
```

Lub użyj Docker:

```bash
docker run -it --rm \
  --name n8n \
  -p 5678:5678 \
  -v ~/.n8n:/home/node/.n8n \
  n8n/n8n
```

---

## 7️⃣ Weryfikacja instalacji

Uruchom skrypt sprawdzający:

```bash
# Sprawdź czy wszystkie narzędzia są dostępne
./run_pipeline.sh --check

# Lub ręcznie:
which chunker
which mempalace_client
which ollama_expander
which doc_generator
ollama --version
python3 --version
```

### Testowe uruchomienie

```bash
# Utwórz przykładowy dokument testowy
echo "# Testowy dokument\n\nTo jest przykładowy tekst do przetworzenia." > input/test.txt

# Uruchom pipeline bez ekspansji
./run_pipeline.sh

# Sprawdź wyniki
ls -lh chunk/
ls -lh finish/
```

---

## 🐛 Rozwiązywanie problemów

### Błąd kompilacji: "json.hpp: No such file or directory"

```bash
# Zainstaluj nlohmann-json
sudo apt install -y nlohmann-json3-dev

# Lub skopiuj nagłówek do projektu
wget https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
mv json.hpp src/
```

### Błąd: "libcurl not found"

```bash
# Zainstaluj libcurl
sudo apt install -y libcurl4-openssl-dev

# Lub dla innych dystrybucji:
sudo dnf install -y libcurl-devel
sudo pacman -S curl
```

### Ollama nie działa

```bash
# Sprawdź status usługi
sudo systemctl status ollama

# Restart usługi
sudo systemctl restart ollama

# Sprawdź logi
journalctl -u ollama -f
```

### Brak pamięci RAM

Jeśli masz problemy z pamięcią:
1. Użyj mniejszego modelu (1.5b zamiast 7b)
2. Zwiększ plik swap:
   ```bash
   sudo dphys-swapfile swapoff
   sudo nano /etc/dphys-swapfile
   # Zmień CONF_SWAPSIZE=1024 na CONF_SWAPSIZE=4096
   sudo dphys-swapfile setup
   sudo dphys-swapfile swapon
   ```

---

## 📚 Następne kroki

Po pomyślnej instalacji:
1. Przeczytaj [README.md](README.md) aby poznać szczegóły użycia
2. Sprawdź [README_WEBUI.md](README_WEBUI.md) dla interfejsu webowego
3. Zapoznaj się z przykładami w katalogu `input/`

---

## 📞 Wsparcie

Jeśli napotkasz problemy:
- Sprawdź [Rozwiązywanie problemów](README.md#-rozwiązywanie-problemów) w README.md
- Otwórz issue na GitHub
- Sprawdź logi w katalogu `logs/`

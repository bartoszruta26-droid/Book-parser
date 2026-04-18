# Makefile for Document Chunker

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
TARGET = chunker
SRC = src/chunker.cpp

.PHONY: all clean test install help

all: $(TARGET)

$(TARGET): $(SRC)
$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
rm -f $(TARGET)
rm -rf chunk/*
rm -rf logs/*

test: $(TARGET)
@echo "Tworzenie plików testowych..."
mkdir -p input
echo "# Rozdzial 1" > input/test1.txt
echo "To jest przykladowy tekst do testowania chunkowania. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat." >> input/test1.txt
echo "# Wstep" > input/test2.md
echo "## Podrozdzial 1.1" >> input/test2.md
echo "Tekst w podrozdziale z wieloma zdaniami. Kolejne zdanie testowe. Jeszcze jedno zdanie dla lepszego testu." >> input/test2.md
@echo "Uruchamianie testow..."
./$(TARGET) -v

install: $(TARGET)
cp $(TARGET) /usr/local/bin/

help:
@echo "Dostepne cele:"
@echo "  all     - Kompilacja programu (domyslny)"
@echo "  clean   - Usuwa pliki wynikowe i czysci katalogi"
@echo "  test    - Kompiluje i uruchamia testy"
@echo "  install - Instaluje program w /usr/local/bin"
@echo "  help    - Wyswietla te pomoc"

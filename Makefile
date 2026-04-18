# Makefile for Document Chunker and Mempalace Client

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
CHUNKER_TARGET = chunker
MEMPALACE_TARGET = mempalace_client
CHUNKER_SRC = src/chunker.cpp
MEMPALACE_SRC = src/mempalace_client.cpp

# Flags dla mempalace client (wymaga CURL i nlohmann/json)
MEMPALACE_LDFLAGS = -lcurl

.PHONY: all clean test install help mempalace chunker

all: $(CHUNKER_TARGET) $(MEMPALACE_TARGET)

chunker: $(CHUNKER_TARGET)

mempalace: $(MEMPALACE_TARGET)

$(CHUNKER_TARGET): $(CHUNKER_SRC)
	$(CXX) $(CXXFLAGS) -o $(CHUNKER_TARGET) $(CHUNKER_SRC)

$(MEMPALACE_TARGET): $(MEMPALACE_SRC)
	$(CXX) $(CXXFLAGS) -o $(MEMPALACE_TARGET) $(MEMPALACE_SRC) $(MEMPALACE_LDFLAGS)

clean:
	rm -f $(CHUNKER_TARGET) $(MEMPALACE_TARGET)
	rm -rf chunk/*
	rm -rf logs/*

test: $(CHUNKER_TARGET)
	@echo "Tworzenie plikow testowych..."
	mkdir -p input
	echo "# Rozdzial 1" > input/test1.txt
	./$(CHUNKER_TARGET) -v

test-mempalace: $(MEMPALACE_TARGET)
	@echo "Testowanie mempalace client..."
	./$(MEMPALACE_TARGET) --help

install: $(CHUNKER_TARGET) $(MEMPALACE_TARGET)
	cp $(CHUNKER_TARGET) /usr/local/bin/
	cp $(MEMPALACE_TARGET) /usr/local/bin/

help:
	@echo "Dostepne cele:"
	@echo "  all            - Kompilacja wszystkich programow (domyslny)"
	@echo "  chunker        - Kompilacja tylko chunkera"
	@echo "  mempalace      - Kompilacja tylko klienta mempalace"
	@echo "  clean          - Usuwa pliki wynikowe i czysci katalogi"
	@echo "  test           - Kompiluje i uruchamia testy chunkera"
	@echo "  test-mempalace - Testuje klienta mempalace"
	@echo "  install        - Instaluje programy w /usr/local/bin"
	@echo "  help           - Wyswietla te pomoc"

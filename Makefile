# Makefile for Document Chunker, Mempalace Client and Ollama Content Expander

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
CHUNKER_TARGET = chunker
MEMPALACE_TARGET = mempalace_client
OLLAMA_EXPANDER_TARGET = ollama_expander
CHUNKER_SRC = src/chunker.cpp
MEMPALACE_SRC = src/mempalace_client.cpp
OLLAMA_EXPANDER_SRC = src/ollama_expander.cpp

# Flags dla mempalace client i ollama expander (wymaga CURL i nlohmann/json)
COMMON_LDFLAGS = -lcurl

.PHONY: all clean test install help mempalace chunker expander

all: $(CHUNKER_TARGET) $(MEMPALACE_TARGET) $(OLLAMA_EXPANDER_TARGET)

chunker: $(CHUNKER_TARGET)

mempalace: $(MEMPALACE_TARGET)

expander: $(OLLAMA_EXPANDER_TARGET)

$(CHUNKER_TARGET): $(CHUNKER_SRC)
	$(CXX) $(CXXFLAGS) -o $(CHUNKER_TARGET) $(CHUNKER_SRC)

$(MEMPALACE_TARGET): $(MEMPALACE_SRC)
	$(CXX) $(CXXFLAGS) -o $(MEMPALACE_TARGET) $(MEMPALACE_SRC) $(COMMON_LDFLAGS)

$(OLLAMA_EXPANDER_TARGET): $(OLLAMA_EXPANDER_SRC) src/ollama_client.h src/content_expander.h
	$(CXX) $(CXXFLAGS) -o $(OLLAMA_EXPANDER_TARGET) $(OLLAMA_EXPANDER_SRC) $(COMMON_LDFLAGS)

clean:
	rm -f $(CHUNKER_TARGET) $(MEMPALACE_TARGET) $(OLLAMA_EXPANDER_TARGET)
	rm -rf chunk/*
	rm -rf logs/*
	rm -rf output/*

test: $(CHUNKER_TARGET)
	@echo "Tworzenie plikow testowych..."
	mkdir -p input
	echo "# Rozdzial 1" > input/test1.txt
	./$(CHUNKER_TARGET) -v

test-mempalace: $(MEMPALACE_TARGET)
	@echo "Testowanie mempalace client..."
	./$(MEMPALACE_TARGET) --help

test-expander: $(OLLAMA_EXPANDER_TARGET)
	@echo "Testowanie ollama expander..."
	./$(OLLAMA_EXPANDER_TARGET) --help

install: $(CHUNKER_TARGET) $(MEMPALACE_TARGET) $(OLLAMA_EXPANDER_TARGET)
	cp $(CHUNKER_TARGET) /usr/local/bin/
	cp $(MEMPALACE_TARGET) /usr/local/bin/
	cp $(OLLAMA_EXPANDER_TARGET) /usr/local/bin/

help:
	@echo "Dostepne cele:"
	@echo "  all            - Kompilacja wszystkich programow (domyslny)"
	@echo "  chunker        - Kompilacja tylko chunkera"
	@echo "  mempalace      - Kompilacja tylko klienta mempalace"
	@echo "  expander       - Kompilacja tylko ollama content expander"
	@echo "  clean          - Usuwa pliki wynikowe i czysci katalogi"
	@echo "  test           - Kompiluje i uruchamia testy chunkera"
	@echo "  test-mempalace - Testuje klienta mempalace"
	@echo "  test-expander  - Testuje ollama content expander"
	@echo "  install        - Instaluje programy w /usr/local/bin"
	@echo "  help           - Wyswietla te pomoc"

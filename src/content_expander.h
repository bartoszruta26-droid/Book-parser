#ifndef CONTENT_EXPANDER_H
#define CONTENT_EXPANDER_H

#include "ollama_client.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <thread>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

// Konfiguracja ekspandera treści
struct ExpanderConfig {
    // Styl generowania
    enum class Style {
        ACADEMIC,      // Styl akademicki/naukowy
        JOURNALISTIC,  // Styl dziennikarski
        TECHNICAL,     // Styl techniczny/dokumentacyjny
        CREATIVE,      // Styl kreatywny/opowieść
        BUSINESS,      // Styl biznesowy
        CASUAL         // Styl nieformalny
    };
    
    Style style = Style::TECHNICAL;
    std::string custom_style_prompt = "";
    
    // Długość outputu
    enum class Length {
        SHORT,    // ~250 słów
        MEDIUM,   // ~500 słów
        LONG,     // ~1000 słów
        VERY_LONG // ~2000+ słów
    };
    
    Length length = Length::MEDIUM;
    int target_word_count = 500;
    
    // Język outputu
    std::string language = "pl";
    
    // Dodatkowe opcje
    bool preserve_structure = true;
    bool add_citations = false;
    bool include_summary = true;
    bool expand_with_examples = true;
    
    // Template promptu systemowego
    std::string system_template = R"(Jesteś ekspertem w tworzeniu spójnych, wielowątkowych treści na podstawie dostarczonych fragmentów. 
Twoim zadaniem jest analiza przesłanych chunków i mempalace, a następnie stworzenie rozszerzonej, bogatej treści.

ZASADY:
1. Analizuj dokładnie wszystkie dostarczone fragmenty
2. Zachowaj spójność tematyczną i logiczną
3. Rozwijaj wątki, dodając kontekst i szczegóły
4. Używaj języka: {language}
5. Styl: {style}
6. Docelowa długość: około {word_count} słów

STRUKTURA OUTPUTU:
- Wstęp wprowadzający w temat
- Rozwinięcie głównych wątków z kontekstem
- Przykłady i ilustracje (jeśli dotyczy)
- Podsumowanie kluczowych punktów

WAŻNE:
- Nie wymyślaj faktów sprzecznych z dostarczonymi danymi
- Zachowaj profesjonalny ton
- Dbaj o płynność przejść między wątkami
- Używaj odpowiedniego formatowania (nagłówki, listy, akapity))";

    std::string get_style_prompt() const {
        if (!custom_style_prompt.empty()) {
            return custom_style_prompt;
        }
        
        switch (style) {
            case Style::ACADEMIC:
                return "Styl akademicki: używaj formalnego języka naukowego, odwołań do teorii, precyzyjnych definicji. Unikaj uproszczeń.";
            case Style::JOURNALISTIC:
                return "Styl dziennikarski: pisz angażująco, używaj leadów, cytatów, konkretnych przykładów. Zasada odwróconej piramidy.";
            case Style::TECHNICAL:
                return "Styl techniczny: precyzyjny, konkretny język. Używaj terminologii specjalistycznej. Struktura: problem -> rozwiązanie -> implementacja.";
            case Style::CREATIVE:
                return "Styl kreatywny: opowiadaj historię, używaj metafor, buduj napięcie. Angażuj emocjonalnie czytelnika.";
            case Style::BUSINESS:
                return "Styl biznesowy: konkretny, zorientowany na wyniki. Używaj danych, wskaźników, rekomendacji. Struktura: sytuacja -> komplikacja -> rozwiązanie.";
            case Style::CASUAL:
                return "Styl nieformalny: przystępny, konwersacyjny ton. Używaj prostego języka, przykładów z życia codziennego.";
            default:
                return "Styl techniczny: precyzyjny i konkretny.";
        }
    }
    
    int get_word_count() const {
        switch (length) {
            case Length::SHORT: return 250;
            case Length::MEDIUM: return 500;
            case Length::LONG: return 1000;
            case Length::VERY_LONG: return 2000;
            default: return target_word_count;
        }
    }
};

// Wynik ekspansji
struct ExpansionResult {
    std::string expanded_content;
    std::string summary;
    std::vector<std::string> threads;  // Zidentyfikowane wątki
    std::vector<std::string> key_points;
    int source_chunks_count;
    int output_word_count;
    double processing_time_seconds;
    bool success;
    std::string error_message;
    
    ExpansionResult() : source_chunks_count(0), output_word_count(0), 
                       processing_time_seconds(0), success(false) {}
};

// Główna klasa ContentExpander
class ContentExpander {
private:
    OllamaClient& ollama;
    ExpanderConfig config;
    std::string log_file;
    
    void log(const std::string& message, bool error = false) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        
        std::string prefix = error ? "[ERROR]" : "[INFO]";
        std::string log_entry = "[" + ss.str() + "] " + prefix + " " + message;
        
        std::cout << log_entry << std::endl;
        
        if (!log_file.empty()) {
            std::ofstream ofs(log_file, std::ios::app);
            if (ofs.is_open()) {
                ofs << log_entry << std::endl;
            }
        }
    }
    
    std::string build_system_prompt() const {
        std::string prompt = config.system_template;
        
        // Podstaw parametry
        size_t pos = prompt.find("{language}");
        if (pos != std::string::npos) {
            prompt.replace(pos, 10, config.language);
        }
        
        pos = prompt.find("{style}");
        if (pos != std::string::npos) {
            prompt.replace(pos, 7, config.get_style_prompt());
        }
        
        pos = prompt.find("{word_count}");
        if (pos != std::string::npos) {
            prompt.replace(pos, 12, std::to_string(config.get_word_count()));
        }
        
        // Dodaj dodatkowe instrukcje
        std::stringstream additions;
        
        if (config.preserve_structure) {
            additions << "\n\nDODATKOWE WYMAGANIA:\n";
            additions << "- Zachowaj oryginalną strukturę dokumentu (rozdziały, podrozdziały)\n";
        }
        
        if (config.add_citations) {
            additions << "- Dodawaj odniesienia do źródłowych chunków tam, gdzie to stosowne\n";
        }
        
        if (config.include_summary) {
            additions << "- Na końcu dodaj krótkie podsumowanie (2-3 zdania)\n";
        }
        
        if (config.expand_with_examples) {
            additions << "- Ilustruj kluczowe punkty konkretnymi przykładami\n";
        }
        
        if (!additions.str().empty()) {
            prompt += additions.str();
        }
        
        return prompt;
    }
    
    std::string build_user_prompt(const std::vector<ContextChunk>& chunks,
                                  const std::string& custom_instruction = "") const {
        std::stringstream ss;
        
        ss << "=== INSTRUKCJA ===\n";
        ss << "Na podstawie poniższych fragmentów (chunków) stwórz spójną, rozszerzoną treść.\n";
        ss << "Połącz informacje z różnych źródeł w logiczną całość.\n";
        ss << "Rozwiń każdy wątek, dodając kontekst i szczegóły.\n\n";
        
        if (!custom_instruction.empty()) {
            ss << "DODATKOWA INSTRUKCJA: " << custom_instruction << "\n\n";
        }
        
        ss << "=== FRAGMENTY DO ANALIZY (" << chunks.size() << " chunków) ===\n\n";
        
        for (size_t i = 0; i < chunks.size(); ++i) {
            const auto& chunk = chunks[i];
            ss << "--- FRAGMENT #" << (i + 1) << " ---\n";
            ss << "ID: " << chunk.chunk_id << "\n";
            if (!chunk.title.empty()) {
                ss << "Tytuł: " << chunk.title << "\n";
            }
            if (!chunk.subtitle.empty()) {
                ss << "Podtytuł: " << chunk.subtitle << "\n";
            }
            ss << "Indeks: " << chunk.chunk_index << "\n\n";
            ss << "TREŚĆ:\n" << chunk.content << "\n\n";
        }
        
        ss << "\n=== KONIEC FRAGMENTÓW ===\n\n";
        ss << "Teraz stwórz rozszerzoną, spójną treść na podstawie powyższych fragmentów.\n";
        ss << "Pamiętaj o zachowaniu stylu i długości zgodnie z konfiguracją.";
        
        return ss.str();
    }
    
public:
    ContentExpander(OllamaClient& client, const ExpanderConfig& cfg = ExpanderConfig(),
                   const std::string& log_path = "")
        : ollama(client), config(cfg), log_file(log_path) {
        log("Zainicjalizowano ContentExpander");
        log("Styl: " + std::to_string(static_cast<int>(cfg.style)));
        log("Długość: " + std::to_string(cfg.get_word_count()) + " słów");
        log("Język: " + cfg.language);
    }
    
    // Ekspansja z pojedynczym chunkiem
    ExpansionResult expand_chunk(const ContextChunk& chunk,
                                const std::string& custom_instruction = "") {
        std::vector<ContextChunk> chunks = {chunk};
        return expand_chunks(chunks, custom_instruction);
    }
    
    // Ekspansja z wielu chunków
    ExpansionResult expand_chunks(const std::vector<ContextChunk>& chunks,
                                 const std::string& custom_instruction = "") {
        log("Rozpoczynanie ekspansji dla " + std::to_string(chunks.size()) + " chunków");
        
        ExpansionResult result;
        result.source_chunks_count = chunks.size();
        auto start_time = std::chrono::high_resolution_clock::now();
        
        if (chunks.empty()) {
            result.error_message = "Brak chunków do ekspansji";
            result.success = false;
            log(result.error_message, true);
            return result;
        }
        
        // Budowanie promptów
        std::string system_prompt = build_system_prompt();
        std::string user_prompt = build_user_prompt(chunks, custom_instruction);
        
        log("Wysyłanie żądania do LLM...");
        
        // Generowanie przez Ollama
        GenerationResult gen_result = ollama.generate(user_prompt, system_prompt);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end_time - start_time;
        result.processing_time_seconds = elapsed.count();
        
        if (!gen_result.success) {
            result.error_message = gen_result.error_message;
            result.success = false;
            log("Generowanie nieudane: " + result.error_message, true);
            return result;
        }
        
        result.expanded_content = gen_result.content;
        result.success = true;
        
        // Szacowanie liczby słów
        std::istringstream iss(result.expanded_content);
        result.output_word_count = std::distance(
            std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>()
        );
        
        log("Ekspansja zakończona sukcesem");
        log("Czas przetwarzania: " + std::to_string(result.processing_time_seconds) + "s");
        log("Liczba słów outputu: " + std::to_string(result.output_word_count));
        
        // Ekstrakcja podsumowania (jeśli wymagane)
        if (config.include_summary) {
            result.summary = extract_summary(result.expanded_content);
        }
        
        // Identyfikacja wątków
        result.threads = identify_threads(result.expanded_content);
        
        return result;
    }
    
    // Ekspansja z pliku JSON z chunkami
    ExpansionResult expand_from_json_file(const std::string& json_path,
                                         const std::string& custom_instruction = "") {
        log("Ładowanie chunków z pliku: " + json_path);
        
        if (!fs::exists(json_path)) {
            ExpansionResult result;
            result.error_message = "Plik nie istnieje: " + json_path;
            result.success = false;
            return result;
        }
        
        std::vector<ContextChunk> chunks;
        
        try {
            std::ifstream file(json_path);
            json data = json::parse(file);
            file.close();
            
            // Sprawdź czy to tablica chunków czy pojedynczy obiekt
            if (data.is_array()) {
                for (const auto& item : data) {
                    ContextChunk chunk;
                    chunk.chunk_id = item.value("chunk_id", "");
                    chunk.content = item.value("content", "");
                    chunk.title = item.value("title", "");
                    if (item.contains("context")) {
                        chunk.title = item["context"].value("title", "");
                        chunk.subtitle = item["context"].value("subtitle", "");
                    }
                    chunk.chunk_index = item.value("chunk_index", 0);
                    chunks.push_back(chunk);
                }
            } else {
                // Pojedynczy chunk
                ContextChunk chunk;
                chunk.chunk_id = data.value("chunk_id", "");
                chunk.content = data.value("content", "");
                chunk.title = data.value("title", "");
                if (data.contains("context")) {
                    chunk.title = data["context"].value("title", "");
                    chunk.subtitle = data["context"].value("subtitle", "");
                }
                chunk.chunk_index = data.value("chunk_index", 0);
                chunks.push_back(chunk);
            }
            
        } catch (const std::exception& e) {
            ExpansionResult result;
            result.error_message = "Błąd parsowania JSON: " + std::string(e.what());
            result.success = false;
            return result;
        }
        
        return expand_chunks(chunks, custom_instruction);
    }
    
    // Ekspansja z katalogu chunków
    ExpansionResult expand_from_directory(const std::string& dir_path,
                                         const std::string& custom_instruction = "",
                                         int max_chunks = 10) {
        log("Skanowanie katalogu: " + dir_path);
        
        if (!fs::exists(dir_path)) {
            ExpansionResult result;
            result.error_message = "Katalog nie istnieje: " + dir_path;
            result.success = false;
            return result;
        }
        
        std::vector<ContextChunk> chunks;
        
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                try {
                    std::ifstream file(entry.path());
                    json data = json::parse(file);
                    file.close();
                    
                    ContextChunk chunk;
                    chunk.chunk_id = data.value("chunk_id", entry.path().stem().string());
                    chunk.content = data.value("content", "");
                    chunk.chunk_index = data.value("chunk_index", 0);
                    
                    if (data.contains("context")) {
                        chunk.title = data["context"].value("title", "");
                        chunk.subtitle = data["context"].value("subtitle", "");
                    }
                    
                    chunks.push_back(chunk);
                    
                    if (static_cast<int>(chunks.size()) >= max_chunks) {
                        break;
                    }
                    
                } catch (const std::exception& e) {
                    log("Błąd ładowania pliku " + entry.path().string() + ": " + e.what(), true);
                }
            }
        }
        
        if (chunks.empty()) {
            ExpansionResult result;
            result.error_message = "Nie znaleziono chunków w katalogu";
            result.success = false;
            return result;
        }
        
        log("Załadowano " + std::to_string(chunks.size()) + " chunków");
        return expand_chunks(chunks, custom_instruction);
    }
    
    // Zapis wyniku do pliku
    bool save_result(const ExpansionResult& result, const std::string& output_path) {
        log("Zapisywanie wyniku do: " + output_path);
        
        try {
            std::ofstream file(output_path);
            if (!file.is_open()) {
                log("Nie można otworzyć pliku do zapisu", true);
                return false;
            }
            
            json output;
            output["success"] = result.success;
            output["expanded_content"] = result.expanded_content;
            output["summary"] = result.summary;
            output["source_chunks_count"] = result.source_chunks_count;
            output["output_word_count"] = result.output_word_count;
            output["processing_time_seconds"] = result.processing_time_seconds;
            output["threads"] = result.threads;
            output["key_points"] = result.key_points;
            
            file << output.dump(2) << std::endl;
            file.close();
            
            log("Wynik zapisany pomyślnie");
            return true;
            
        } catch (const std::exception& e) {
            log("Błąd zapisu: " + std::string(e.what()), true);
            return false;
        }
    }
    
    // Zapis wyniku jako plain text
    bool save_as_text(const ExpansionResult& result, const std::string& output_path) {
        log("Zapisywanie tekstu do: " + output_path);
        
        try {
            std::ofstream file(output_path);
            if (!file.is_open()) {
                log("Nie można otworzyć pliku do zapisu", true);
                return false;
            }
            
            file << result.expanded_content << std::endl;
            
            if (!result.summary.empty()) {
                file << "\n\n=== PODSUMOWANIE ===\n";
                file << result.summary << std::endl;
            }
            
            file.close();
            log("Tekst zapisany pomyślnie");
            return true;
            
        } catch (const std::exception& e) {
            log("Błąd zapisu: " + std::string(e.what()), true);
            return false;
        }
    }
    
private:
    // Pomocnicza funkcja do ekstrakcji podsumowania
    std::string extract_summary(const std::string& content) const {
        // Prosta heurystyka: ostatni akapit lub pierwsze 3 zdania
        std::istringstream iss(content);
        std::string line;
        std::vector<std::string> paragraphs;
        
        while (std::getline(iss, line)) {
            if (!line.empty() && line.length() > 20) {
                paragraphs.push_back(line);
            }
        }
        
        if (paragraphs.size() >= 2) {
            return paragraphs.back();
        } else if (!paragraphs.empty()) {
            return paragraphs[0];
        }
        
        return "";
    }
    
    // Identyfikacja wątków w treści
    std::vector<std::string> identify_threads(const std::string& content) const {
        std::vector<std::string> threads;
        
        // Prosta detekcja nagłówków/sekcji
        std::istringstream iss(content);
        std::string line;
        
        while (std::getline(iss, line)) {
            // Wykrywanie nagłówków Markdown
            if ((line.length() > 2 && line.substr(0, 2) == "##") ||
                (line.length() > 1 && line[0] == '#')) {
                // Usuń znaki # i whitespace
                std::string thread = line;
                thread.erase(std::remove(thread.begin(), thread.end(), '#'), thread.end());
                
                // Trim whitespace
                size_t start = thread.find_first_not_of(" \t");
                size_t end = thread.find_last_not_of(" \t");
                if (start != std::string::npos && end != std::string::npos) {
                    thread = thread.substr(start, end - start + 1);
                }
                
                if (!thread.empty() && thread.length() < 100) {
                    threads.push_back(thread);
                }
            }
        }
        
        return threads;
    }
};

#endif // CONTENT_EXPANDER_H

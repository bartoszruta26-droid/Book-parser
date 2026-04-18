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

#include "ollama_client.h"
#include "content_expander.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

void print_help() {
    std::cout << "=== Ollama Content Expander ===" << std::endl;
    std::cout << "Narzędzie do generowania rozszerzonych treści z wykorzystaniem lokalnego LLM przez Ollama" << std::endl;
    std::cout << std::endl;
    std::cout << "Użycie: ./ollama_expander [opcje]" << std::endl;
    std::cout << std::endl;
    std::cout << "Opcje:" << std::endl;
    std::cout << "  -i, --input PATH      Ścieżka do pliku JSON z chunkami lub katalogu z chunkami" << std::endl;
    std::cout << "  -o, --output PATH     Ścieżka wyjściowa dla wygenerowanej treści (domyślnie: ./output/expanded.json)" << std::endl;
    std::cout << "  -t, --text            Zapisz output jako plain text (.txt) zamiast JSON" << std::endl;
    std::cout << std::endl;
    std::cout << "  -m, --model MODEL     Nazwa modelu Ollama (domyślnie: qwen2.5-coder:7b)" << std::endl;
    std::cout << "  -H, --host HOST       Host Ollama (domyślnie: localhost)" << std::endl;
    std::cout << "  -p, --port PORT       Port Ollama (domyślnie: 11434)" << std::endl;
    std::cout << std::endl;
    std::cout << "  -s, --style STYLE     Styl generowania: academic, journalistic, technical, creative, business, casual" << std::endl;
    std::cout << "                        (domyślnie: technical)" << std::endl;
    std::cout << "  -l, --length LENGTH   Długość outputu: short (~250), medium (~500), long (~1000), very_long (~2000+)" << std::endl;
    std::cout << "                        (domyślnie: medium)" << std::endl;
    std::cout << "  --lang LANGUAGE       Język outputu (domyślnie: pl)" << std::endl;
    std::cout << std::endl;
    std::cout << "  --temp TEMPERATURE    Temperatura generowania 0.0-2.0 (domyślnie: 0.7)" << std::endl;
    std::cout << "  --top-p VALUE         Top-p sampling 0.0-1.0 (domyślnie: 0.9)" << std::endl;
    std::cout << "  --max-tokens N        Maksymalna liczba tokenów outputu (domyślnie: 4096)" << std::endl;
    std::cout << std::endl;
    std::cout << "  --instruction TEXT    Dodatkowa instrukcja dla modelu" << std::endl;
    std::cout << "  --no-summary          Nie dodawaj podsumowania" << std::endl;
    std::cout << "  --no-examples         Nie dodawaj przykładów" << std::endl;
    std::cout << "  --max-chunks N        Maksymalna liczba chunków do przetworzenia (domyślnie: 10)" << std::endl;
    std::cout << std::endl;
    std::cout << "  --list-models         Wyświetl dostępne modele i zakończ" << std::endl;
    std::cout << "  --check               Sprawdź dostępność Ollama i zakończ" << std::endl;
    std::cout << "  -v, --verbose         Tryb szczegółowy" << std::endl;
    std::cout << "  -h, --help            Wyświetl tę pomoc" << std::endl;
    std::cout << std::endl;
    std::cout << "Przykłady użycia:" << std::endl;
    std::cout << "  ./ollama_expander -i ./chunk/sample_chunk_0.json -o expanded.json" << std::endl;
    std::cout << "  ./ollama_expander -i ./chunk/ -o output.txt -t -s creative -l long" << std::endl;
    std::cout << "  ./ollama_expander -i ./chunk/ -m llama3.2 --lang en --style journalistic" << std::endl;
    std::cout << std::endl;
}

ExpanderConfig::Style parse_style(const std::string& style_str) {
    std::string lower = style_str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "academic") return ExpanderConfig::Style::ACADEMIC;
    if (lower == "journalistic") return ExpanderConfig::Style::JOURNALISTIC;
    if (lower == "technical") return ExpanderConfig::Style::TECHNICAL;
    if (lower == "creative") return ExpanderConfig::Style::CREATIVE;
    if (lower == "business") return ExpanderConfig::Style::BUSINESS;
    if (lower == "casual") return ExpanderConfig::Style::CASUAL;
    
    return ExpanderConfig::Style::TECHNICAL;
}

ExpanderConfig::Length parse_length(const std::string& length_str) {
    std::string lower = length_str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "short") return ExpanderConfig::Length::SHORT;
    if (lower == "medium") return ExpanderConfig::Length::MEDIUM;
    if (lower == "long") return ExpanderConfig::Length::LONG;
    if (lower == "very_long") return ExpanderConfig::Length::VERY_LONG;
    
    return ExpanderConfig::Length::MEDIUM;
}

int main(int argc, char* argv[]) {
    // Wartości domyślne
    std::string input_path = "./chunk";
    std::string output_path = "./output/expanded.json";
    bool output_as_text = false;
    
    std::string ollama_host = "localhost";
    int ollama_port = 11434;
    std::string model_name = "qwen2.5-coder:7b";
    
    std::string style_str = "technical";
    std::string length_str = "medium";
    std::string language = "pl";
    
    float temperature = 0.7f;
    int top_p = 9;  // 0.9 * 10
    int max_tokens = 4096;
    
    std::string custom_instruction = "";
    bool include_summary = true;
    bool expand_with_examples = true;
    int max_chunks = 10;
    
    bool list_models_only = false;
    bool check_only = false;
    bool verbose = false;
    
    // Parsowanie argumentów
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            input_path = argv[++i];
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output_path = argv[++i];
        } else if (arg == "-t" || arg == "--text") {
            output_as_text = true;
        } else if ((arg == "-m" || arg == "--model") && i + 1 < argc) {
            model_name = argv[++i];
        } else if ((arg == "-H" || arg == "--host") && i + 1 < argc) {
            ollama_host = argv[++i];
        } else if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
            ollama_port = std::stoi(argv[++i]);
        } else if ((arg == "-s" || arg == "--style") && i + 1 < argc) {
            style_str = argv[++i];
        } else if ((arg == "-l" || arg == "--length") && i + 1 < argc) {
            length_str = argv[++i];
        } else if ((arg == "--lang") && i + 1 < argc) {
            language = argv[++i];
        } else if ((arg == "--temp") && i + 1 < argc) {
            temperature = std::stof(argv[++i]);
        } else if ((arg == "--top-p") && i + 1 < argc) {
            top_p = std::stof(argv[++i]) * 10;
        } else if ((arg == "--max-tokens") && i + 1 < argc) {
            max_tokens = std::stoi(argv[++i]);
        } else if ((arg == "--instruction") && i + 1 < argc) {
            custom_instruction = argv[++i];
        } else if (arg == "--no-summary") {
            include_summary = false;
        } else if (arg == "--no-examples") {
            expand_with_examples = false;
        } else if ((arg == "--max-chunks") && i + 1 < argc) {
            max_chunks = std::stoi(argv[++i]);
        } else if (arg == "--list-models") {
            list_models_only = true;
        } else if (arg == "--check") {
            check_only = true;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if (arg == "-h" || arg == "--help") {
            print_help();
            return 0;
        }
    }
    
    std::cout << "=== Ollama Content Expander ===" << std::endl;
    std::cout << "Wersja: 1.0.0" << std::endl;
    std::cout << std::endl;
    
    // Konfiguracja Ollama
    OllamaConfig ollama_config;
    ollama_config.host = ollama_host;
    ollama_config.port = ollama_port;
    ollama_config.model = model_name;
    ollama_config.temperature = temperature;
    ollama_config.top_p = top_p / 10;
    ollama_config.max_tokens = max_tokens;
    
    try {
        OllamaClient ollama(ollama_config, "./logs/ollama.log");
        
        // Tryb: lista modeli
        if (list_models_only) {
            std::cout << "Dostępne modele w Ollama:" << std::endl;
            auto models = ollama.list_models();
            
            if (models.empty()) {
                std::cout << "  Brak modeli lub nie można połączyć z Ollama" << std::endl;
                std::cout << std::endl;
                std::cout << "Aby pobrać model, uruchom:" << std::endl;
                std::cout << "  ollama pull qwen2.5-coder:7b" << std::endl;
                std::cout << "  ollama pull llama3.2" << std::endl;
                return 1;
            }
            
            for (const auto& model : models) {
                std::cout << "  - " << model << std::endl;
            }
            return 0;
        }
        
        // Tryb: sprawdzenie dostępności
        if (check_only) {
            std::cout << "Sprawdzanie dostępności Ollama..." << std::endl;
            if (ollama.health_check()) {
                std::cout << "Ollama jest dostępne!" << std::endl;
                
                auto models = ollama.list_models();
                std::cout << "Dostępne modele: " << models.size() << std::endl;
                
                if (std::find(models.begin(), models.end(), model_name) != models.end()) {
                    std::cout << "Model '" << model_name << "' jest dostępny." << std::endl;
                } else {
                    std::cout << "Model '" << model_name << "' NIE jest dostępny." << std::endl;
                    std::cout << "Dostępne modele:" << std::endl;
                    for (const auto& m : models) {
                        std::cout << "  - " << m << std::endl;
                    }
                }
                return 0;
            } else {
                std::cerr << "Ollama nie jest dostępne!" << std::endl;
                std::cerr << "Upewnij się, że usługa Ollama jest uruchomiona:" << std::endl;
                std::cerr << "  ollama serve" << std::endl;
                return 1;
            }
        }
        
        // Sprawdzenie dostępności przed kontynuacją
        if (!ollama.health_check()) {
            std::cerr << "Nie można połączyć z Ollama!" << std::endl;
            std::cerr << "Upewnij się, że usługa Ollama jest uruchomiona na " 
                      << ollama_host << ":" << ollama_port << std::endl;
            return 1;
        }
        
        // Sprawdzenie czy model jest dostępny
        auto available_models = ollama.list_models();
        if (std::find(available_models.begin(), available_models.end(), model_name) == available_models.end()) {
            std::cerr << "Model '" << model_name << "' nie jest dostępny!" << std::endl;
            std::cerr << "Dostępne modele:" << std::endl;
            for (const auto& m : available_models) {
                std::cout << "  - " << m << std::endl;
            }
            std::cerr << std::endl;
            std::cerr << "Aby pobrać model, uruchom:" << std::endl;
            std::cerr << "  ollama pull " << model_name << std::endl;
            return 1;
        }
        
        // Konfiguracja ekspandera
        ExpanderConfig expander_config;
        expander_config.style = parse_style(style_str);
        expander_config.length = parse_length(length_str);
        expander_config.language = language;
        expander_config.include_summary = include_summary;
        expander_config.expand_with_examples = expand_with_examples;
        
        ContentExpander expander(ollama, expander_config, "./logs/expander.log");
        
        std::cout << "Konfiguracja:" << std::endl;
        std::cout << "  Model: " << model_name << std::endl;
        std::cout << "  Wejście: " << input_path << std::endl;
        std::cout << "  Wyjście: " << output_path << std::endl;
        std::cout << "  Styl: " << style_str << std::endl;
        std::cout << "  Długość: " << length_str << std::endl;
        std::cout << "  Język: " << language << std::endl;
        std::cout << "  Temperatura: " << temperature << std::endl;
        std::cout << "  Max chunków: " << max_chunks << std::endl;
        if (!custom_instruction.empty()) {
            std::cout << "  Instrukcja: " << custom_instruction << std::endl;
        }
        std::cout << std::endl;
        
        // Tworzenie katalogu wyjściowego
        fs::path output_dir = fs::path(output_path).parent_path();
        if (!output_dir.empty() && !fs::exists(output_dir)) {
            std::cout << "Tworzenie katalogu wyjściowego: " << output_dir.string() << std::endl;
            fs::create_directories(output_dir);
        }
        
        // Ekspansja treści
        ExpansionResult result;
        
        if (fs::is_directory(input_path)) {
            std::cout << "Przetwarzanie katalogu chunków..." << std::endl;
            result = expander.expand_from_directory(input_path, custom_instruction, max_chunks);
        } else if (fs::exists(input_path)) {
            std::cout << "Przetwarzanie pliku z chunkami..." << std::endl;
            result = expander.expand_from_json_file(input_path, custom_instruction);
        } else {
            std::cerr << "Błąd: Ścieżka wejściowa nie istnieje: " << input_path << std::endl;
            return 1;
        }
        
        if (!result.success) {
            std::cerr << "Błąd ekspansji: " << result.error_message << std::endl;
            return 1;
        }
        
        // Zapis wyniku
        bool saved = false;
        if (output_as_text) {
            saved = expander.save_as_text(result, output_path);
        } else {
            saved = expander.save_result(result, output_path);
        }
        
        if (!saved) {
            std::cerr << "Błąd zapisu wyniku!" << std::endl;
            return 1;
        }
        
        // Podsumowanie
        std::cout << std::endl;
        std::cout << "=== PODSUMOWANIE ===" << std::endl;
        std::cout << "Status: SUKCES" << std::endl;
        std::cout << "Źródłowych chunków: " << result.source_chunks_count << std::endl;
        std::cout << "Słów w outputcie: " << result.output_word_count << std::endl;
        std::cout << "Czas przetwarzania: " << std::fixed << std::setprecision(2) 
                  << result.processing_time_seconds << "s" << std::endl;
        std::cout << "Wyjście zapisane do: " << output_path << std::endl;
        
        if (!result.threads.empty()) {
            std::cout << std::endl;
            std::cout << "Zidentyfikowane wątki:" << std::endl;
            for (const auto& thread : result.threads) {
                std::cout << "  - " << thread << std::endl;
            }
        }
        
        if (!result.summary.empty() && verbose) {
            std::cout << std::endl;
            std::cout << "Podsumowanie:" << std::endl;
            std::cout << result.summary << std::endl;
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Błąd krytyczny: " << e.what() << std::endl;
        return 1;
    }
}

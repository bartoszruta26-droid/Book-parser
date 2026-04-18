#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <cstring>

namespace fs = std::filesystem;

// Struktura metadanych chunka
struct ChunkMetadata {
    std::string source_file;
    int chunk_index;
    int total_chunks;
    std::string previous_chapter;
    std::string next_chapter;
    std::string previous_subchapter;
    std::string next_subchapter;
    std::string previous_subsubchapter;
    std::string next_subsubchapter;
    std::string title;
    std::string subtitle;
    int page_number;
    int token_count;
    std::string timestamp;
};

// Klasa do obsługi logowania
class Logger {
private:
    std::ofstream log_file;
    bool verbose;
    
public:
    Logger(const std::string& log_path, bool verbose_mode = false) : verbose(verbose_mode) {
        fs::create_directories(fs::path(log_path).parent_path());
        log_file.open(log_path, std::ios::app);
        log("=== Nowa sesja chunkowania ===");
    }
    
    void log(const std::string& message, bool always = false) {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
        
        std::string log_entry = "[" + ss.str() + "] " + message;
        
        if (always || verbose) {
            std::cout << log_entry << std::endl;
        }
        
        if (log_file.is_open()) {
            log_file << log_entry << std::endl;
            log_file.flush();
        }
    }
    
    ~Logger() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }
};

// Klasa do obliczania hash MD5 (uproszczona implementacja)
class MD5Hash {
public:
    static std::string calculate(const std::string& content) {
        // Uproszczony hash dla detekcji duplikatów
        unsigned long hash = 5381;
        for (char c : content) {
            hash = ((hash << 5) + hash) + c;
        }
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(16) << hash;
        return ss.str();
    }
};

// Klasa do szacowania liczby tokenów
class TokenCounter {
public:
    static int estimate(const std::string& text) {
        // Przybliżone liczenie: 1 token ≈ 4 znaki w językach łacińskich
        // Dla polskiego trochę więcej ze względu na znaki specjalne
        int count = 0;
        bool in_word = false;
        
        for (size_t i = 0; i < text.length(); ++i) {
            char c = text[i];
            if (std::isalnum(static_cast<unsigned char>(c)) || c == 0xC4 || c == 0xC5) {
                if (!in_word) {
                    in_word = true;
                    count++;
                }
            } else {
                in_word = false;
            }
        }
        
        // Dodatkowe dostosowanie dla polskich znaków
        return static_cast<int>(text.length() / 4.0) + (count / 2);
    }
};

// Główna klasa chunkera
class DocumentChunker {
private:
    std::string input_dir;
    std::string output_dir;
    std::string log_dir;
    int chunk_size_tokens;
    Logger logger;
    std::set<std::string> processed_hashes;
    
    // Detekcja nagłówków rozdziałów
    std::vector<std::string> detect_chapters(const std::string& content) {
        std::vector<std::string> chapters;
        std::istringstream stream(content);
        std::string line;
        
        while (std::getline(stream, line)) {
            // Wykrywanie nagłówków typu "# Rozdział", "## Podrozdział", itp.
            if (line.find("#") == 0 || 
                (line.length() > 2 && std::isupper(static_cast<unsigned char>(line[0])) && 
                 line.find(":") != std::string::npos && line.length() < 100)) {
                // Usuń znaki # i spacje
                std::string chapter = line;
                size_t start = chapter.find_first_not_of("# \t");
                if (start != std::string::npos) {
                    chapter = chapter.substr(start);
                }
                
                // Dodaj tylko jeśli nie jest pusty i nie za długi
                if (!chapter.empty() && chapter.length() < 200) {
                    chapters.push_back(chapter);
                }
            }
        }
        
        return chapters;
    }
    
    // Dzielenie tekstu na zdania
    std::vector<std::string> split_into_sentences(const std::string& text) {
        std::vector<std::string> sentences;
        std::string current_sentence;
        
        for (size_t i = 0; i < text.length(); ++i) {
            char c = text[i];
            current_sentence += c;
            
            // Sprawdzenie końca zdania
            if (c == '.' || c == '!' || c == '?') {
                // Sprawdź czy to nie skrót (np., itd., etc.)
                bool is_abbreviation = false;
                if (i + 1 < text.length() && text[i+1] >= 'a' && text[i+1] <= 'z') {
                    is_abbreviation = true;
                }
                
                if (!is_abbreviation || i == text.length() - 1) {
                    // Usuń białe znaki z początku
                    size_t start = current_sentence.find_first_not_of(" \t\n\r");
                    if (start != std::string::npos) {
                        current_sentence = current_sentence.substr(start);
                    }
                    
                    if (!current_sentence.empty()) {
                        sentences.push_back(current_sentence);
                    }
                    current_sentence.clear();
                }
            }
        }
        
        // Dodaj pozostały tekst
        if (!current_sentence.empty()) {
            size_t start = current_sentence.find_first_not_of(" \t\n\r");
            if (start != std::string::npos) {
                current_sentence = current_sentence.substr(start);
            }
            if (!current_sentence.empty()) {
                sentences.push_back(current_sentence);
            }
        }
        
        return sentences;
    }
    
    // Tworzenie chunków z zachowaniem granic semantycznych
    std::vector<std::pair<std::string, int>> create_chunks(
        const std::string& content, 
        const std::vector<std::string>& chapters) {
        
        std::vector<std::pair<std::string, int>> chunks;
        std::vector<std::string> sentences = split_into_sentences(content);
        
        std::string current_chunk;
        int current_token_count = 0;
        int chapter_index = 0;
        
        for (const auto& sentence : sentences) {
            int sentence_tokens = TokenCounter::estimate(sentence);
            
            // Jeśli dodanie zdania przekroczy limit, zapisz obecny chunk
            if (current_token_count + sentence_tokens > chunk_size_tokens && !current_chunk.empty()) {
                chunks.push_back({current_chunk, chapter_index});
                current_chunk.clear();
                current_token_count = 0;
                
                // Aktualizuj indeks rozdziału
                chapter_index = (chapter_index + 1) % std::max(1, static_cast<int>(chapters.size()));
            }
            
            current_chunk += sentence + " ";
            current_token_count += sentence_tokens;
        }
        
        // Dodaj ostatni chunk
        if (!current_chunk.empty()) {
            chunks.push_back({current_chunk, chapter_index});
        }
        
        return chunks;
    }
    
    // Generowanie metadanych dla chunka
    ChunkMetadata generate_metadata(
        const std::string& source_file,
        int chunk_index,
        int total_chunks,
        const std::vector<std::string>& chapters,
        int chapter_index,
        int token_count) {
        
        ChunkMetadata metadata;
        metadata.source_file = source_file;
        metadata.chunk_index = chunk_index;
        metadata.total_chunks = total_chunks;
        metadata.token_count = token_count;
        
        // Pobierz aktualny czas
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%dT%H:%M:%S");
        metadata.timestamp = ss.str();
        
        // Ustaw informacje o rozdziałach
        if (!chapters.empty()) {
            metadata.previous_chapter = (chapter_index > 0) ? chapters[chapter_index - 1] : "";
            metadata.next_chapter = (chapter_index < static_cast<int>(chapters.size()) - 1) 
                                    ? chapters[chapter_index + 1] : "";
            
            // Uproszczone ustawienie podrozdziałów (w pełnej wersji można parsować hierarchię)
            metadata.previous_subchapter = metadata.previous_chapter;
            metadata.next_subchapter = metadata.next_chapter;
            metadata.title = chapters[chapter_index];
            metadata.subtitle = "";
        }
        
        metadata.page_number = (chunk_index * chunk_size_tokens) / 800 + 1; // Przybliżenie
        
        return metadata;
    }
    
    // Zapis chunka do pliku JSON
    void save_chunk_json(const std::string& output_path, 
                        const std::string& content,
                        const ChunkMetadata& metadata) {
        std::ofstream json_file(output_path);
        if (!json_file.is_open()) {
            logger.log("Błąd: Nie można otworzyć pliku JSON: " + output_path, true);
            return;
        }
        
        // Escape special characters in strings
        auto escape_json = [](const std::string& s) -> std::string {
            std::string result;
            for (char c : s) {
                switch (c) {
                    case '"': result += "\\\""; break;
                    case '\\': result += "\\\\"; break;
                    case '\n': result += "\\n"; break;
                    case '\r': result += "\\r"; break;
                    case '\t': result += "\\t"; break;
                    default: result += c;
                }
            }
            return result;
        };
        
        json_file << "{\n";
        json_file << "  \"source_file\": \"" << escape_json(metadata.source_file) << "\",\n";
        json_file << "  \"chunk_index\": " << metadata.chunk_index << ",\n";
        json_file << "  \"total_chunks\": " << metadata.total_chunks << ",\n";
        json_file << "  \"token_count\": " << metadata.token_count << ",\n";
        json_file << "  \"page_number\": " << metadata.page_number << ",\n";
        json_file << "  \"timestamp\": \"" << escape_json(metadata.timestamp) << "\",\n";
        json_file << "  \"context\": {\n";
        json_file << "    \"previous_chapter\": \"" << escape_json(metadata.previous_chapter) << "\",\n";
        json_file << "    \"next_chapter\": \"" << escape_json(metadata.next_chapter) << "\",\n";
        json_file << "    \"previous_subchapter\": \"" << escape_json(metadata.previous_subchapter) << "\",\n";
        json_file << "    \"next_subchapter\": \"" << escape_json(metadata.next_subchapter) << "\",\n";
        json_file << "    \"previous_subsubchapter\": \"" << escape_json(metadata.previous_subsubchapter) << "\",\n";
        json_file << "    \"next_subsubchapter\": \"" << escape_json(metadata.next_subsubchapter) << "\",\n";
        json_file << "    \"title\": \"" << escape_json(metadata.title) << "\",\n";
        json_file << "    \"subtitle\": \"" << escape_json(metadata.subtitle) << "\"\n";
        json_file << "  },\n";
        json_file << "  \"content\": \"" << escape_json(content) << "\"\n";
        json_file << "}\n";
        
        json_file.close();
    }
    
    // Przetwarzanie pojedynczego pliku
    bool process_file(const fs::path& file_path) {
        std::string filename = file_path.filename().string();
        std::string extension = file_path.extension().string();
        
        // Sprawdź obsługiwane formaty
        if (extension != ".txt" && extension != ".md" && extension != ".json") {
            logger.log("Pominięto nieobsługiwany format: " + filename);
            return false;
        }
        
        logger.log("Przetwarzanie pliku: " + filename);
        
        // Wczytaj zawartość pliku
        std::ifstream input_file(file_path);
        if (!input_file.is_open()) {
            logger.log("Błąd: Nie można otworzyć pliku: " + filename, true);
            return false;
        }
        
        std::stringstream buffer;
        buffer << input_file.rdbuf();
        std::string content = buffer.str();
        input_file.close();
        
        // Sprawdź duplikaty
        std::string hash = MD5Hash::calculate(content);
        if (processed_hashes.find(hash) != processed_hashes.end()) {
            logger.log("Wykryto duplikat, pominięto: " + filename);
            return false;
        }
        processed_hashes.insert(hash);
        
        // Wykryj rozdziały
        std::vector<std::string> chapters = detect_chapters(content);
        if (chapters.empty()) {
            chapters.push_back("Brak wykrytych rozdziałów");
        }
        
        logger.log("Wykryto rozdziałów: " + std::to_string(chapters.size()));
        
        // Utwórz chunki
        std::vector<std::pair<std::string, int>> chunks = create_chunks(content, chapters);
        logger.log("Utworzono chunków: " + std::to_string(chunks.size()));
        
        // Zapisz chunki
        std::string base_name = file_path.stem().string();
        int chunk_index = 0;
        
        for (const auto& [chunk_content, chapter_idx] : chunks) {
            std::string chunk_filename = base_name + "_chunk_" + 
                                        std::to_string(chunk_index) + ".txt";
            std::string json_filename = base_name + "_chunk_" + 
                                       std::to_string(chunk_index) + ".json";
            
            std::string txt_path = output_dir + "/" + chunk_filename;
            std::string json_path = output_dir + "/" + json_filename;
            
            // Zapisz zawartość chunka jako .txt
            std::ofstream txt_file(txt_path);
            if (txt_file.is_open()) {
                txt_file << chunk_content;
                txt_file.close();
            }
            
            // Generuj i zapisz metadane jako .json
            int token_count = TokenCounter::estimate(chunk_content);
            ChunkMetadata metadata = generate_metadata(
                filename, chunk_index, static_cast<int>(chunks.size()),
                chapters, chapter_idx, token_count
            );
            
            save_chunk_json(json_path, chunk_content, metadata);
            
            chunk_index++;
        }
        
        logger.log("Zakończono przetwarzanie: " + filename);
        return true;
    }
    
public:
    DocumentChunker(const std::string& input, 
                   const std::string& output,
                   const std::string& logs,
                   int chunk_size = 4096,
                   bool verbose = false)
        : input_dir(input), output_dir(output), log_dir(logs),
          chunk_size_tokens(chunk_size),
          logger(logs + "/chunker.log", verbose) {
        
        // Utwórz katalogi wyjściowe
        fs::create_directories(output_dir);
        fs::create_directories(log_dir);
        
        logger.log("Inicjalizacja DocumentChunker");
        logger.log("Katalog wejściowy: " + input_dir);
        logger.log("Katalog wyjściowy: " + output_dir);
        logger.log("Rozmiar chunka: " + std::to_string(chunk_size_tokens) + " tokenów");
    }
    
    void process_all_files() {
        logger.log("Rozpoczynanie skanowania katalogu: " + input_dir);
        
        if (!fs::exists(input_dir)) {
            logger.log("Błąd: Katalog wejściowy nie istnieje!", true);
            return;
        }
        
        int processed_count = 0;
        int skipped_count = 0;
        
        for (const auto& entry : fs::directory_iterator(input_dir)) {
            if (entry.is_regular_file()) {
                if (process_file(entry.path())) {
                    processed_count++;
                } else {
                    std::string ext = entry.path().extension().string();
                    if (ext == ".txt" || ext == ".md" || ext == ".json") {
                        skipped_count++;
                    }
                }
            }
        }
        
        // Policz chunki w katalogu wyjściowym
        int chunk_count = 0;
        for (const auto& entry : fs::directory_iterator(output_dir)) {
            if (entry.is_regular_file()) {
                chunk_count++;
            }
        }
        chunk_count = chunk_count / 2; // Liczymy pary .txt + .json
        
        logger.log("=== Podsumowanie ===", true);
        logger.log("Przetworzono plików: " + std::to_string(processed_count), true);
        logger.log("Pominięto (duplikaty/błędy): " + std::to_string(skipped_count), true);
        logger.log("Łączna liczba chunków w katalogu: " + std::to_string(chunk_count), true);
    }
};

void print_help() {
    std::cout << "Document Chunker - Inteligentne dzielenie dokumentów\n\n";
    std::cout << "Użycie: ./chunker [opcje]\n\n";
    std::cout << "Opcje:\n";
    std::cout << "  -i, --input DIR       Katalog wejściowy (domyślnie: ./input)\n";
    std::cout << "  -o, --output DIR      Katalog wyjściowy (domyślnie: ./chunk)\n";
    std::cout << "  -l, --logs DIR        Katalog logów (domyślnie: ./logs)\n";
    std::cout << "  -s, --chunk-size N    Rozmiar chunka w tokenach (domyślnie: 4096)\n";
    std::cout << "  -v, --verbose         Tryb szczegółowy\n";
    std::cout << "  -h, --help            Wyświetl tę pomoc\n";
    std::cout << "\nObsługiwane formaty: .txt, .md, .json\n";
}

int main(int argc, char* argv[]) {
    std::string input_dir = "./input";
    std::string output_dir = "./chunk";
    std::string log_dir = "./logs";
    int chunk_size = 4096;
    bool verbose = false;
    
    // Parsowanie argumentów
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_help();
            return 0;
        } else if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        } else if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            input_dir = argv[++i];
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            output_dir = argv[++i];
        } else if ((arg == "-l" || arg == "--logs") && i + 1 < argc) {
            log_dir = argv[++i];
        } else if ((arg == "-s" || arg == "--chunk-size") && i + 1 < argc) {
            chunk_size = std::stoi(argv[++i]);
        }
    }
    
    std::cout << "=== Document Chunker (C++) ===" << std::endl;
    std::cout << "Wersja: 1.0.0" << std::endl;
    std::cout << "Kompilacja: C++17" << std::endl;
    std::cout << std::endl;
    
    DocumentChunker chunker(input_dir, output_dir, log_dir, chunk_size, verbose);
    chunker.process_all_files();
    
    return 0;
}

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

// Konfiguracja połączenia z mempalace
struct MempalaceConfig {
    std::string host = "localhost";
    int port = 8080;
    std::string api_version = "v1";
    int timeout_seconds = 30;
    int max_retries = 3;
    int retry_delay_ms = 1000;
    
    std::string get_base_url() const {
        return "http://" + host + ":" + std::to_string(port) + "/api/" + api_version;
    }
};

// Struktura chunka do wysyłki
struct MempalaceChunk {
    std::string chunk_id;
    std::string source_file;
    int chunk_index;
    int total_chunks;
    std::string content;
    std::string title;
    std::string subtitle;
    std::string previous_chapter;
    std::string next_chapter;
    std::string previous_subchapter;
    std::string next_subchapter;
    std::string previous_subsubchapter;
    std::string next_subsubchapter;
    int page_number;
    int token_count;
    std::string timestamp;
    std::vector<std::string> tags;
};

// Klasa do obsługi odpowiedzi HTTP
class HttpResponse {
public:
    long status_code;
    std::string body;
    bool success;
    
    HttpResponse() : status_code(0), success(false) {}
};

// Callback dla CURL
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_size = size * nmemb;
    userp->append((char*)contents, total_size);
    return total_size;
}

// Klient mempalace
class MempalaceClient {
private:
    MempalaceConfig config;
    CURL* curl;
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
    
    HttpResponse make_request(const std::string& endpoint, 
                             const std::string& method,
                             const std::string& body = "") {
        HttpResponse response;
        
        std::string url = config.get_base_url() + endpoint;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, config.timeout_seconds);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        std::string response_body;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
        
        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        } else if (method == "PUT") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        } else if (method == "DELETE") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        } else {
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        }
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            log("Błąd CURL: " + std::string(curl_easy_strerror(res)), true);
            response.success = false;
        } else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);
            response.body = response_body;
            response.success = (response.status_code >= 200 && response.status_code < 300);
        }
        
        curl_slist_free_all(headers);
        return response;
    }
    
public:
    MempalaceClient(const MempalaceConfig& cfg, const std::string& log_path = "") 
        : config(cfg), log_file(log_path) {
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        
        if (!curl) {
            throw std::runtime_error("Nie udało się zainicjalizować CURL");
        }
        
        log("Zainicjalizowano klienta mempalace: " + config.get_base_url());
    }
    
    ~MempalaceClient() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
        log("Zamknięto klienta mempalace");
    }
    
    // Sprawdzenie dostępności mempalace
    bool health_check() {
        log("Sprawdzanie dostępności mempalace...");
        
        for (int i = 0; i < config.max_retries; ++i) {
            HttpResponse response = make_request("/health", "GET");
            
            if (response.success) {
                log("mempalace jest dostępny");
                return true;
            }
            
            log("Próba " + std::to_string(i+1) + "/" + std::to_string(config.max_retries) + 
                " nieudana, czekam...", true);
            
            if (i < config.max_retries - 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.retry_delay_ms));
            }
        }
        
        log("mempalace nie jest dostępny po " + std::to_string(config.max_retries) + " próbach", true);
        return false;
    }
    
    // Wysyłka pojedynczego chunka
    bool upload_chunk(const MempalaceChunk& chunk) {
        log("Wysyłanie chunka: " + chunk.chunk_id);
        
        json payload;
        payload["chunk_id"] = chunk.chunk_id;
        payload["source_file"] = chunk.source_file;
        payload["chunk_index"] = chunk.chunk_index;
        payload["total_chunks"] = chunk.total_chunks;
        payload["content"] = chunk.content;
        payload["token_count"] = chunk.token_count;
        payload["timestamp"] = chunk.timestamp;
        
        // Metadane kontekstowe
        json context;
        context["title"] = chunk.title;
        context["subtitle"] = chunk.subtitle;
        context["previous_chapter"] = chunk.previous_chapter;
        context["next_chapter"] = chunk.next_chapter;
        context["previous_subchapter"] = chunk.previous_subchapter;
        context["next_subchapter"] = chunk.next_subchapter;
        context["previous_subsubchapter"] = chunk.previous_subsubchapter;
        context["next_subsubchapter"] = chunk.next_subsubchapter;
        context["page_number"] = chunk.page_number;
        payload["context"] = context;
        
        // Tagi
        if (!chunk.tags.empty()) {
            payload["tags"] = chunk.tags;
        }
        
        std::string body = payload.dump();
        
        for (int i = 0; i < config.max_retries; ++i) {
            HttpResponse response = make_request("/chunks", "POST", body);
            
            if (response.success) {
                log("Pomyślnie wysłano chunk: " + chunk.chunk_id);
                return true;
            }
            
            log("Błąd wysyłania chunka " + chunk.chunk_id + ": " + 
                std::to_string(response.status_code), true);
            
            if (i < config.max_retries - 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.retry_delay_ms * (i + 1)));
            }
        }
        
        return false;
    }
    
    // Wysłanie wielu chunków (batch)
    int upload_chunks_batch(const std::vector<MempalaceChunk>& chunks) {
        log("Rozpoczynanie wysyłania batcha: " + std::to_string(chunks.size()) + " chunków");
        
        int success_count = 0;
        int fail_count = 0;
        
        for (const auto& chunk : chunks) {
            if (upload_chunk(chunk)) {
                success_count++;
            } else {
                fail_count++;
            }
        }
        
        log("Zakończono batch: " + std::to_string(success_count) + " sukcesów, " + 
            std::to_string(fail_count) + " błędów");
        
        return success_count;
    }
    
    // Wysłanie wszystkich chunków z katalogu
    int upload_directory(const std::string& chunk_dir) {
        log("Skanowanie katalogu chunków: " + chunk_dir);
        
        if (!fs::exists(chunk_dir)) {
            log("Katalog nie istnieje: " + chunk_dir, true);
            return 0;
        }
        
        std::vector<MempalaceChunk> chunks;
        
        // Zbierz wszystkie pliki JSON z chunkami
        for (const auto& entry : fs::directory_iterator(chunk_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                try {
                    std::ifstream file(entry.path());
                    if (!file.is_open()) {
                        log("Nie można otworzyć pliku: " + entry.path().string(), true);
                        continue;
                    }
                    
                    json data;
                    file >> data;
                    file.close();
                    
                    MempalaceChunk chunk;
                    chunk.chunk_id = data.value("chunk_id", 
                        entry.path().stem().string());
                    chunk.source_file = data.value("source_file", "");
                    chunk.chunk_index = data.value("chunk_index", 0);
                    chunk.total_chunks = data.value("total_chunks", 1);
                    chunk.content = data.value("content", "");
                    chunk.token_count = data.value("token_count", 0);
                    chunk.timestamp = data.value("timestamp", "");
                    chunk.page_number = data.value("page_number", 1);
                    
                    // Kontekst
                    if (data.contains("context")) {
                        const auto& ctx = data["context"];
                        chunk.title = ctx.value("title", "");
                        chunk.subtitle = ctx.value("subtitle", "");
                        chunk.previous_chapter = ctx.value("previous_chapter", "");
                        chunk.next_chapter = ctx.value("next_chapter", "");
                        chunk.previous_subchapter = ctx.value("previous_subchapter", "");
                        chunk.next_subchapter = ctx.value("next_subchapter", "");
                        chunk.previous_subsubchapter = ctx.value("previous_subsubchapter", "");
                        chunk.next_subsubchapter = ctx.value("next_subsubchapter", "");
                    }
                    
                    // Generuj ID jeśli brak
                    if (chunk.chunk_id.empty()) {
                        std::stringstream ss;
                        ss << chunk.source_file << "_chunk_" << chunk.chunk_index;
                        chunk.chunk_id = ss.str();
                    }
                    
                    chunks.push_back(chunk);
                    
                } catch (const std::exception& e) {
                    log("Błąd parsowania pliku " + entry.path().string() + ": " + e.what(), true);
                }
            }
        }
        
        if (chunks.empty()) {
            log("Nie znaleziono żadnych chunków w katalogu", true);
            return 0;
        }
        
        log("Znaleziono " + std::to_string(chunks.size()) + " chunków do wysłania");
        
        // Sprawdź dostępność przed wysyłką
        if (!health_check()) {
            log("Przerywam wysyłanie - mempalace niedostępny", true);
            return 0;
        }
        
        return upload_chunks_batch(chunks);
    }
    
    // Odpytanie o kontekst między-chunkowy
    std::string query_context(const std::string& query, int limit = 5) {
        log("Odpytywanie o kontekst: " + query);
        
        json payload;
        payload["query"] = query;
        payload["limit"] = limit;
        
        HttpResponse response = make_request("/query", "POST", payload.dump());
        
        if (response.success) {
            try {
                json result = json::parse(response.body);
                return result.dump(2);
            } catch (...) {
                return response.body;
            }
        }
        
        return "";
    }
    
    // Pobranie powiązanych fragmentów
    std::vector<MempalaceChunk> get_related_chunks(const std::string& chunk_id) {
        log("Pobieranie powiązanych chunków dla: " + chunk_id);
        
        HttpResponse response = make_request("/chunks/" + chunk_id + "/related", "GET");
        
        std::vector<MempalaceChunk> related;
        
        if (response.success) {
            try {
                json result = json::parse(response.body);
                if (result.contains("chunks")) {
                    for (const auto& item : result["chunks"]) {
                        MempalaceChunk chunk;
                        chunk.chunk_id = item.value("chunk_id", "");
                        chunk.source_file = item.value("source_file", "");
                        chunk.content = item.value("content", "");
                        related.push_back(chunk);
                    }
                }
            } catch (const std::exception& e) {
                log("Błąd parsowania odpowiedzi: " + std::string(e.what()), true);
            }
        }
        
        return related;
    }
    
    // Usunięcie chunka
    bool delete_chunk(const std::string& chunk_id) {
        log("Usuwanie chunka: " + chunk_id);
        
        HttpResponse response = make_request("/chunks/" + chunk_id, "DELETE");
        return response.success;
    }
    
    // Wyczyszczenie całej bazy
    bool clear_database() {
        log("Czyszczenie całej bazy danych");
        
        HttpResponse response = make_request("/chunks", "DELETE");
        return response.success;
    }
    
    // Pobranie statystyk
    json get_stats() {
        log("Pobieranie statystyk");
        
        HttpResponse response = make_request("/stats", "GET");
        
        if (response.success) {
            try {
                return json::parse(response.body);
            } catch (...) {
                return json();
            }
        }
        
        return json();
    }
};

// Funkcja pomocnicza do generowania unikalnego ID
std::string generate_chunk_id(const std::string& source_file, int chunk_index) {
    std::stringstream ss;
    
    // Usuń rozszerzenie i ścieżkę
    std::string filename = fs::path(source_file).stem().string();
    
    // Zamień spacje na podkreślenia
    std::replace(filename.begin(), filename.end(), ' ', '_');
    std::replace(filename.begin(), filename.end(), '/', '_');
    std::replace(filename.begin(), filename.end(), '\\', '_');
    
    ss << filename << "_chunk_" << std::setfill('0') << std::setw(4) << chunk_index;
    return ss.str();
}

// Główna funkcja programu
void print_help() {
    std::cout << "Mempalace Client - Integracja z bazą wiedzy AI\n\n";
    std::cout << "Użycie: ./mempalace_client [opcje]\n\n";
    std::cout << "Opcje:\n";
    std::cout << "  -i, --input DIR       Katalog z chunkami (domyślnie: ./chunk)\n";
    std::cout << "  -H, --host HOST       Host mempalace (domyślnie: localhost)\n";
    std::cout << "  -p, --port PORT       Port mempalace (domyślnie: 8080)\n";
    std::cout << "  -q, --query TEXT      Odpytaj o kontekst\n";
    std::cout << "  -l, --limit N         Limit wyników zapytania (domyślnie: 5)\n";
    std::cout << "  -c, --clear           Wyczyść całą bazę danych\n";
    std::cout << "  -s, --stats           Pokaż statystyki bazy\n";
    std::cout << "  -t, --timeout SEC     Timeout połączenia (domyślnie: 30)\n";
    std::cout << "  -r, --retries N       Maksymalna liczba powtórzeń (domyślnie: 3)\n";
    std::cout << "  -v, --verbose         Tryb szczegółowy\n";
    std::cout << "  -h, --help            Wyświetl tę pomoc\n";
    std::cout << "\nPrzykłady:\n";
    std::cout << "  ./mempalace_client -i ./chunk -H localhost -p 8080\n";
    std::cout << "  ./mempalace_client -q \"sztuczna inteligencja\" -l 10\n";
    std::cout << "  ./mempalace_client --stats\n";
    std::cout << "  ./mempalace_client --clear\n";
}

int main(int argc, char* argv[]) {
    std::string input_dir = "./chunk";
    std::string host = "localhost";
    int port = 8080;
    int timeout = 30;
    int retries = 3;
    std::string query_text;
    int query_limit = 5;
    bool clear_db = false;
    bool show_stats = false;
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
        } else if ((arg == "-H" || arg == "--host") && i + 1 < argc) {
            host = argv[++i];
        } else if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if ((arg == "-t" || arg == "--timeout") && i + 1 < argc) {
            timeout = std::stoi(argv[++i]);
        } else if ((arg == "-r" || arg == "--retries") && i + 1 < argc) {
            retries = std::stoi(argv[++i]);
        } else if ((arg == "-q" || arg == "--query") && i + 1 < argc) {
            query_text = argv[++i];
        } else if ((arg == "-l" || arg == "--limit") && i + 1 < argc) {
            query_limit = std::stoi(argv[++i]);
        } else if (arg == "-c" || arg == "--clear") {
            clear_db = true;
        } else if (arg == "-s" || arg == "--stats") {
            show_stats = true;
        }
    }
    
    std::cout << "=== Mempalace Client (C++) ===" << std::endl;
    std::cout << "Wersja: 1.0.0" << std::endl;
    std::cout << "Kompilacja: C++17 z CURL i nlohmann/json" << std::endl;
    std::cout << std::endl;
    
    // Konfiguracja
    MempalaceConfig config;
    config.host = host;
    config.port = port;
    config.timeout_seconds = timeout;
    config.max_retries = retries;
    
    try {
        MempalaceClient client(config, "./logs/mempalace_client.log");
        
        // Tryb czyszczenia bazy
        if (clear_db) {
            std::cout << "Czy na pewno chcesz wyczyścić całą bazę danych? (y/n): ";
            std::string confirm;
            std::cin >> confirm;
            
            if (confirm == "y" || confirm == "Y") {
                if (client.clear_database()) {
                    std::cout << "Baza danych została wyczyszczona." << std::endl;
                    return 0;
                } else {
                    std::cerr << "Błąd czyszczenia bazy danych." << std::endl;
                    return 1;
                }
            } else {
                std::cout << "Anulowano operację." << std::endl;
                return 0;
            }
        }
        
        // Tryb pokazywania statystyk
        if (show_stats) {
            if (client.health_check()) {
                json stats = client.get_stats();
                std::cout << stats.dump(2) << std::endl;
                return 0;
            } else {
                std::cerr << "Nie można połączyć z mempalace." << std::endl;
                return 1;
            }
        }
        
        // Tryb odpytywania
        if (!query_text.empty()) {
            if (client.health_check()) {
                std::string result = client.query_context(query_text, query_limit);
                std::cout << result << std::endl;
                return 0;
            } else {
                std::cerr << "Nie można połączyć z mempalace." << std::endl;
                return 1;
            }
        }
        
        // Domyślny tryb: wysyłanie chunków
        std::cout << "Konfiguracja:" << std::endl;
        std::cout << "  Host: " << config.host << std::endl;
        std::cout << "  Port: " << config.port << std::endl;
        std::cout << "  Katalog wejściowy: " << input_dir << std::endl;
        std::cout << "  Timeout: " << config.timeout_seconds << "s" << std::endl;
        std::cout << "  Max powtórzeń: " << config.max_retries << std::endl;
        std::cout << std::endl;
        
        int uploaded = client.upload_directory(input_dir);
        
        std::cout << std::endl;
        std::cout << "=== Podsumowanie ===" << std::endl;
        std::cout << "Wysłano chunków: " << uploaded << std::endl;
        
        if (uploaded > 0) {
            std::cout << "Status: SUKCES" << std::endl;
            return 0;
        } else {
            std::cout << "Status: BRAK DANYCH LUB BŁĄD" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Błąd krytyczny: " << e.what() << std::endl;
        return 1;
    }
}

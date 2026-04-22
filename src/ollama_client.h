#ifndef OLLAMA_CLIENT_H
#define OLLAMA_CLIENT_H

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

// Konfiguracja połączenia z Ollama
struct OllamaConfig {
    std::string host = "localhost";
    int port = 11434;
    std::string model = "qwen2.5-coder:7b";
    int timeout_seconds = 600;  // 10 minut dla długich generowań (np. 13k znaków)
    int max_retries = 3;
    int retry_delay_ms = 2000;
    bool stream = false;
    
    // Parametry generowania
    float temperature = 0.7;
    int top_p = 0.9;
    int max_tokens = 4096;
    std::string language = "pl";  // Domyślny język outputu
    
    std::string get_base_url() const {
        return "http://" + host + ":" + std::to_string(port);
    }
    
    std::string get_api_url() const {
        return get_base_url() + "/api";
    }
};

// Struktura wyniku generowania
struct GenerationResult {
    std::string content;
    std::string model;
    std::string created_at;
    bool done;
    int total_duration_ms;
    int load_duration_ms;
    int prompt_eval_count;
    int eval_count;
    bool success;
    std::string error_message;
    
    GenerationResult() : content(""), model(""), created_at(""), done(false),
                         total_duration_ms(0), load_duration_ms(0),
                         prompt_eval_count(0), eval_count(0),
                         success(false), error_message("") {}
};

// Struktura chunka z mempalace dla kontekstu
struct ContextChunk {
    std::string chunk_id;
    std::string content;
    std::string title;
    std::string subtitle;
    int chunk_index;
};

// Klasa do obsługi odpowiedzi HTTP
class OllamaHttpResponse {
public:
    long status_code;
    std::string body;
    bool success;
    
    OllamaHttpResponse() : status_code(0), success(false) {}
};

// Callback dla CURL
size_t OllamaWriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_size = size * nmemb;
    userp->append((char*)contents, total_size);
    return total_size;
}

// Klient Ollama
class OllamaClient {
private:
    OllamaConfig config;
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
    
    OllamaHttpResponse make_request(const std::string& endpoint, 
                                   const std::string& method,
                                   const std::string& body = "") {
        OllamaHttpResponse response;
        
        std::string url = config.get_api_url() + endpoint;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, config.timeout_seconds);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        std::string response_body;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OllamaWriteCallback);
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
    OllamaClient(const OllamaConfig& cfg, const std::string& log_path = "") 
        : config(cfg), log_file(log_path) {
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        
        if (!curl) {
            throw std::runtime_error("Nie udało się zainicjalizować CURL");
        }
        
        log("Zainicjalizowano klienta Ollama: " + config.get_base_url());
        log("Model: " + config.model);
    }
    
    ~OllamaClient() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
        log("Zamknięto klienta Ollama");
    }
    
    // Sprawdzenie dostępności Ollama
    bool health_check() {
        log("Sprawdzanie dostępności Ollama...");
        
        for (int i = 0; i < config.max_retries; ++i) {
            OllamaHttpResponse response = make_request("/tags", "GET");
            
            if (response.success) {
                log("Ollama jest dostępne");
                return true;
            }
            
            log("Próba " + std::to_string(i+1) + "/" + std::to_string(config.max_retries) + 
                " nieudana, czekam...", true);
            
            if (i < config.max_retries - 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.retry_delay_ms));
            }
        }
        
        log("Ollama nie jest dostępne po " + std::to_string(config.max_retries) + " próbach", true);
        return false;
    }
    
    // Pobranie listy dostępnych modeli
    std::vector<std::string> list_models() {
        log("Pobieranie listy modeli...");
        
        std::vector<std::string> models;
        OllamaHttpResponse response = make_request("/tags", "GET");
        
        if (response.success) {
            try {
                json result = json::parse(response.body);
                if (result.contains("models")) {
                    for (const auto& model : result["models"]) {
                        std::string name = model.value("name", "");
                        if (!name.empty()) {
                            models.push_back(name);
                            log("  - " + name);
                        }
                    }
                }
            } catch (const std::exception& e) {
                log("Błąd parsowania odpowiedzi: " + std::string(e.what()), true);
            }
        }
        
        return models;
    }
    
    // Sprawdzenie czy model jest dostępny
    bool is_model_available(const std::string& model_name) {
        auto models = list_models();
        return std::find(models.begin(), models.end(), model_name) != models.end();
    }
    
    // Generowanie tekstu z LLM
    GenerationResult generate(const std::string& prompt, 
                             const std::string& system_prompt = "",
                             const std::vector<ContextChunk>& context = {}) {
        log("Generowanie odpowiedzi dla promptu (" + std::to_string(prompt.length()) + " znaków)...");
        
        json payload;
        payload["model"] = config.model;
        payload["prompt"] = prompt;
        payload["stream"] = config.stream;
        
        // Opcje generowania
        json options;
        options["temperature"] = config.temperature;
        options["top_p"] = config.top_p;
        options["num_predict"] = config.max_tokens;
        payload["options"] = options;
        
        // System prompt
        if (!system_prompt.empty()) {
            payload["system"] = system_prompt;
        }
        
        // Kontekst jako część promptu
        if (!context.empty()) {
            std::stringstream context_ss;
            context_ss << "\n\n=== KONTEKST Z MEMPALACE ===\n";
            for (const auto& chunk : context) {
                context_ss << "--- Chunk #" << chunk.chunk_index << " (" << chunk.chunk_id << ") ---\n";
                if (!chunk.title.empty()) {
                    context_ss << "Tytuł: " << chunk.title << "\n";
                }
                if (!chunk.subtitle.empty()) {
                    context_ss << "Podtytuł: " << chunk.subtitle << "\n";
                }
                context_ss << "Treść:\n" << chunk.content << "\n\n";
            }
            context_ss << "=== KONIEC KONTEKSTU ===\n";
            
            std::string current_prompt = payload["prompt"].get<std::string>();
            payload["prompt"] = context_ss.str() + "\n" + current_prompt;
        }
        
        std::string body = payload.dump();
        log("Wysyłanie żądania do Ollama...");
        
        GenerationResult result;
        
        for (int i = 0; i < config.max_retries; ++i) {
            OllamaHttpResponse response = make_request("/generate", "POST", body);
            
            if (response.success) {
                try {
                    json result_json = json::parse(response.body);
                    
                    result.content = result_json.value("response", "");
                    result.model = result_json.value("model", config.model);
                    result.created_at = result_json.value("created_at", "");
                    result.done = result_json.value("done", false);
                    result.total_duration_ms = result_json.value("total_duration", 0);
                    result.load_duration_ms = result_json.value("load_duration", 0);
                    result.prompt_eval_count = result_json.value("prompt_eval_count", 0);
                    result.eval_count = result_json.value("eval_count", 0);
                    result.success = true;
                    
                    log("Generowanie zakończone sukcesem. Tokeny: " + 
                        std::to_string(result.prompt_eval_count) + " -> " + 
                        std::to_string(result.eval_count));
                    
                    return result;
                    
                } catch (const std::exception& e) {
                    result.error_message = "Błąd parsowania odpowiedzi: " + std::string(e.what());
                    log(result.error_message, true);
                }
            } else {
                result.error_message = "Błąd HTTP: " + std::to_string(response.status_code);
                log(result.error_message, true);
            }
            
            if (i < config.max_retries - 1) {
                log("Ponawianie próby za " + std::to_string(config.retry_delay_ms) + " ms...");
                std::this_thread::sleep_for(std::chrono::milliseconds(config.retry_delay_ms));
            }
        }
        
        result.success = false;
        return result;
    }
    
    // Generowanie z konwersacją (chat)
    GenerationResult chat(const std::vector<std::pair<std::string, std::string>>& messages,
                         const std::string& system_prompt = "") {
        log("Rozpoczynanie konwersacji (" + std::to_string(messages.size()) + " wiadomości)...");
        
        json payload;
        payload["model"] = config.model;
        payload["stream"] = config.stream;
        
        // System prompt
        if (!system_prompt.empty()) {
            payload["system"] = system_prompt;
        }
        
        // Wiadomości
        json messages_json = json::array();
        for (const auto& msg : messages) {
            json message;
            message["role"] = msg.first;  // "user", "assistant", "system"
            message["content"] = msg.second;
            messages_json.push_back(message);
        }
        payload["messages"] = messages_json;
        
        // Opcje generowania
        json options;
        options["temperature"] = config.temperature;
        options["top_p"] = config.top_p;
        options["num_predict"] = config.max_tokens;
        payload["options"] = options;
        
        std::string body = payload.dump();
        
        GenerationResult result;
        
        for (int i = 0; i < config.max_retries; ++i) {
            OllamaHttpResponse response = make_request("/chat", "POST", body);
            
            if (response.success) {
                try {
                    json result_json = json::parse(response.body);
                    
                    if (result_json.contains("message")) {
                        result.content = result_json["message"].value("content", "");
                    }
                    result.model = result_json.value("model", config.model);
                    result.created_at = result_json.value("created_at", "");
                    result.done = result_json.value("done", false);
                    result.total_duration_ms = result_json.value("total_duration", 0);
                    result.load_duration_ms = result_json.value("load_duration", 0);
                    result.prompt_eval_count = result_json.value("prompt_eval_count", 0);
                    result.eval_count = result_json.value("eval_count", 0);
                    result.success = true;
                    
                    log("Konwersacja zakończona sukcesem.");
                    return result;
                    
                } catch (const std::exception& e) {
                    result.error_message = "Błąd parsowania odpowiedzi: " + std::string(e.what());
                    log(result.error_message, true);
                }
            } else {
                result.error_message = "Błąd HTTP: " + std::to_string(response.status_code);
                log(result.error_message, true);
            }
            
            if (i < config.max_retries - 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.retry_delay_ms));
            }
        }
        
        result.success = false;
        return result;
    }
    
    // Embeddingi
    std::vector<float> embeddings(const std::string& text) {
        log("Generowanie embeddingów...");
        
        json payload;
        payload["model"] = config.model;
        payload["prompt"] = text;
        
        std::string body = payload.dump();
        OllamaHttpResponse response = make_request("/embeddings", "POST", body);
        
        std::vector<float> result;
        
        if (response.success) {
            try {
                json result_json = json::parse(response.body);
                if (result_json.contains("embedding")) {
                    for (const auto& val : result_json["embedding"]) {
                        result.push_back(val.get<float>());
                    }
                }
                log("Wygenerowano embedding o wymiarze: " + std::to_string(result.size()));
            } catch (const std::exception& e) {
                log("Błąd parsowania embeddingów: " + std::string(e.what()), true);
            }
        }
        
        return result;
    }
    
    // Settery parametrów
    void set_temperature(float temp) { config.temperature = temp; }
    void set_top_p(int top_p) { config.top_p = top_p; }
    void set_max_tokens(int max) { config.max_tokens = max; }
    void set_language(const std::string& lang) { config.language = lang; }
    void set_model(const std::string& model) { config.model = model; }
};

#endif // OLLAMA_CLIENT_H

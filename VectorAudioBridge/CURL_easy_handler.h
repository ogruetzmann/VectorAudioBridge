#pragma once
#include <curl/curl.h>
#include <functional>
#include <string>
class CURL_easy_handler {
public:
    enum class handle_type { rx = 0, tx, active, version };
    const handle_type type;

    CURL_easy_handler(const handle_type t, const std::string& url) noexcept;
    ~CURL_easy_handler();

    CURL* get_handle() const noexcept;
    const std::string get_response() noexcept;
    const long get_response_code() const noexcept;
    const handle_type& get_type() const noexcept;
    const bool is_valid() const noexcept;
    const bool is_changed() const noexcept;
    void process(std::function<void(const CURL_easy_handler::handle_type, const std::string data)> callback);

private:
    std::string buffer;
    std::string last_response;
    long response_code { 0 };
    bool changed { false };
    CURL* curl { nullptr };
#ifdef _DEBUG
    char errbuf[CURL_ERROR_SIZE] {};
#endif

    void init(const std::string& url) noexcept;
};

size_t write_callback(const char*, size_t, size_t, void*);

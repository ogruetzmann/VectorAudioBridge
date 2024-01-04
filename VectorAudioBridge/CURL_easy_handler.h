#pragma once
#include <curl/curl.h>
#include <functional>
#include <string>
class CURL_easy_handler {
public:
    enum class handle_type {
        rx = 0,
        tx,
        active,
        version
    };
    const handle_type type;

    CURL_easy_handler(const handle_type t, const std::string& url);
    ~CURL_easy_handler();
    friend constexpr bool operator==(const CURL_easy_handler& lhs, const CURL_easy_handler& rhs);

    CURL* get_handle() const;
    const std::string get_response();
    const long get_response_code() const;
    const handle_type& get_type() const;
    const bool is_valid() const;
    const bool is_changed() const;
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

    void init(const std::string& url);
};

size_t write_callback(char*, size_t, size_t, std::string*);
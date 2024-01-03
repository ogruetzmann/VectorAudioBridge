#pragma once
#include <string>
#include <curl/curl.h>
class CURL_easy_handler {
public:
    enum class handle_type {
        tx = 0,
        rx,
        active,
        version
    };
    const handle_type type;

    CURL_easy_handler(const handle_type t, const std::string& url);
    ~CURL_easy_handler();
    friend constexpr bool operator==(const CURL_easy_handler& lhs, const CURL_easy_handler& rhs);
    
    CURL* get_handle() const;
    const std::string get_response();
    const long get_response_code();
    const handle_type& get_type() const;
    const bool is_valid() const;

private:
    std::string buffer;
    long response_code { 0 };
    CURL* curl { nullptr };
};

size_t write_callback(char*, size_t, size_t, std::string*);
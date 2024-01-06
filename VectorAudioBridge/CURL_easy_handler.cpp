#include "CURL_easy_handler.h"

CURL_easy_handler::CURL_easy_handler(const handle_type t, const std::string& url) noexcept : type(t)
{
    init(url);
}

CURL_easy_handler::~CURL_easy_handler()
{
    curl_easy_cleanup(curl);
    curl = nullptr;
}

CURL* CURL_easy_handler::get_handle() const noexcept
{
    return curl;
}

const std::string CURL_easy_handler::get_response() noexcept
{
    changed = false;
    return last_response;
}

const long CURL_easy_handler::get_response_code() const noexcept
{
    return response_code;
}

const CURL_easy_handler::handle_type& CURL_easy_handler::get_type() const noexcept
{
    return type;
}

const bool CURL_easy_handler::is_valid() const noexcept
{
    return curl != nullptr;
}

const bool CURL_easy_handler::is_changed() const noexcept
{
    return changed;
}

void CURL_easy_handler::process(std::function<void(const CURL_easy_handler::handle_type, const std::string data)> callback)
{
    constexpr int HTTP_OK { 200 };
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code == HTTP_OK && buffer != last_response) {
        last_response = buffer;
        if (callback) {
            callback(type, last_response);
        }
        else {
            changed = true;
        }
    }
    buffer.clear();
}

void CURL_easy_handler::init(const std::string& url) noexcept
{
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_PRIVATE, this);
    constexpr int timeout_ms { 1000 };
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_ms);
#ifdef _DEBUG
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
#endif
}

size_t write_callback(const char* ptr, size_t size, size_t nmemb, void* userdata)
{
    std::string* buffer = static_cast<std::string*>(userdata);
    if (buffer == nullptr)
        return 0;
    buffer->append(ptr, size * nmemb);
    return size * nmemb;
};

#include "CURL_easy_handler.h"

CURL_easy_handler::CURL_easy_handler(const handle_type t, const std::string& url)
    : type(t)
{
    init(url);
}

CURL_easy_handler::~CURL_easy_handler()
{
    curl_easy_cleanup(curl);
    curl = nullptr;
}

CURL* CURL_easy_handler::get_handle() const
{
    return curl;
}

const std::string CURL_easy_handler::get_response()
{
    changed = false;
    return last_response;
}

const long CURL_easy_handler::get_response_code() const
{
    return response_code;
}

const CURL_easy_handler::handle_type& CURL_easy_handler::get_type() const
{
    return type;
}

const bool CURL_easy_handler::is_valid() const
{
    return curl != nullptr;
}

const bool CURL_easy_handler::is_changed() const
{
    return changed;
}

void CURL_easy_handler::process(std::function<void(const CURL_easy_handler::handle_type, const std::string data)> callback)
{
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200)
        return;
    if (buffer != last_response) {
        last_response = buffer;
        if (callback) {
            callback(type, last_response);
        } else {
            changed = true;
        }
    }
    buffer.clear();
}

void CURL_easy_handler::init(const std::string& url)
{
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_PRIVATE, this);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1000);
#ifdef _DEBUG
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
#endif
}

constexpr bool operator==(const CURL_easy_handler& lhs, const CURL_easy_handler& rhs)
{
    return lhs.curl == rhs.curl;
}

size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* userdata)
{
    if (userdata == nullptr)
        return 0;
    userdata->append(ptr, size * nmemb);
    return size * nmemb;
};

#include "Vectoraudio_Socket.h"

Vectoraudio_socket::Vectoraudio_socket(
    std::function<void(std::string, std::string)> msg)
    : display_message(msg)
{
    init_curl();
    init_handles();
    std::jthread worker(std::bind_front(&Vectoraudio_socket::run, this), 200);
    worker_stop = worker.get_stop_source();
    worker.detach();
}

Vectoraudio_socket::~Vectoraudio_socket()
{
    worker_stop.request_stop();
    curl_multi_remove_handle(curlm, rx_handle);
    curl_multi_remove_handle(curlm, tx_handle);
    curl_easy_cleanup(rx_handle);
    curl_easy_cleanup(tx_handle);
    curl_multi_cleanup(curlm);
    curl_global_cleanup();
}

const bool Vectoraudio_socket::has_error() const
{
    return false;
}

const bool Vectoraudio_socket::rx_changed()
{
    return rx_freqs.is_changed();
}

const bool Vectoraudio_socket::tx_changed()
{
    return tx_freqs.is_changed();
}

const frequency_pairs& Vectoraudio_socket::get_rx()
{
    return rx_freqs.get();
}

const frequency_pairs& Vectoraudio_socket::get_tx()
{
    return tx_freqs.get();
}

void Vectoraudio_socket::poll()
{
    if (error_state)
        return;

    CURLMsg* msg{ nullptr };
    int msgq = 0;
    do {
        msg = curl_multi_info_read(curlm, &msgq);
        if (msg && msg->msg == CURLMSG_DONE) {
            auto x = msg->data.result;
            handle_reply(msg->easy_handle);
            curl_multi_remove_handle(curlm, msg->easy_handle);
            curl_multi_add_handle(curlm, msg->easy_handle);
        }
    } while (msg);

    int runningHandles{ 0 };
    curl_multi_perform(curlm, &runningHandles);
}

void Vectoraudio_socket::ping()
{
}

void Vectoraudio_socket::run(std::stop_token token, int interval_ms)
{
    using clock = std::chrono::system_clock;
    clock::time_point start, now;
    while (!token.stop_requested()) {
        now = clock::now();
        if (start.time_since_epoch() == clock::duration::zero() || now - start > status.interval)
        {
            poll();
            start = clock::now();
        }
    }
}

int Vectoraudio_socket::handle_reply(CURL* handle)
{
    Active_frequencies* freqs{ nullptr };
    curl_easy_getinfo(handle, CURLINFO_PRIVATE, &freqs);

    long code;
    if (curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code) == CURLE_OK && code == 200) {
        if (freqs) {
            frequency_pairs pairs = parse_reply(freqs->curl_buffer);
            freqs->set(pairs);
        }
    }

    freqs->clear();
    return 0;
}

frequency_pairs Vectoraudio_socket::parse_reply(const std::string& reply) const
{
    std::vector<Frequency> freqs;

    if (!reply.size())
        return freqs;

    constexpr char reply_sep = ',';
    constexpr char pos_sep = ':';
    auto begin = reply.begin();
    auto end = reply.begin();
    do {
        end = std::find(begin, reply.end(), reply_sep);
        std::string_view token = std::string_view(begin, end);

        auto pos = std::find(token.begin(), token.end(), pos_sep);
        freqs.push_back({ std::string { token.begin(), pos },
            std::string { pos + 1, token.end() } });

        if (end != reply.end())
            begin = ++end;
    } while (end != reply.end());
    return freqs;
}

//auto write_callback = +[](char* ptr, size_t size, size_t nmemb, std::string* userdata) -> size_t {
//    if (userdata == nullptr)
//        return 0;
//    userdata->append(ptr, size * nmemb);
//    return size * nmemb;
//    };

CURL* Vectoraudio_socket::easy_init(const std::string& url, Active_frequencies& freqs)
{
    auto curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &freqs.curl_buffer);
    curl_easy_setopt(curl, CURLOPT_PRIVATE, &freqs);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1000);
    return curl;
}

void Vectoraudio_socket::init_curl()
{
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK)
        throw std::exception("cURL global initialization failed!");

    curlm = curl_multi_init();
    if (!curlm)
        throw std::exception("cURL multi initialization failed!");
}

void Vectoraudio_socket::init_handles()
{
    rx_handle = easy_init(rx_url, rx_freqs);
    curl_multi_add_handle(curlm, rx_handle);
    tx_handle = easy_init(tx_url, tx_freqs);
    curl_multi_add_handle(curlm, tx_handle);
}

size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* userdata)
{
    if (userdata == nullptr)
        return 0;
    userdata->append(ptr, size * nmemb);
    return size * nmemb;
};


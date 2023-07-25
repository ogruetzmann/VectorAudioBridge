#include "Vectoraudio_Socket.h"

Vectoraudio_socket::Vectoraudio_socket()
{
    if (init_handles() != 0) {
        error_state = true;
        last_error = "Could not initialize curl";
    }
}

Vectoraudio_socket::~Vectoraudio_socket()
{
    curl_multi_cleanup(curl);
    curl_easy_cleanup(rx_handle);
    curl_easy_cleanup(tx_handle);
    curl_global_cleanup();
}

const bool Vectoraudio_socket::has_error() const
{
    return error_state;
}

const std::string& Vectoraudio_socket::error_msg() const
{
    return last_error;
}

const bool Vectoraudio_socket::rx_changed() const
{
    return rx_freqs.is_changed();
}

const bool Vectoraudio_socket::tx_changed() const
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

    CURLMsg* msg { nullptr };
    int msgq { 0 };
    while ((msg = curl_multi_info_read(curl, &msgq))) {
        CURL* hnd = msg->easy_handle;
        if (msg->msg == CURLMSG_DONE) {
            if (msg->data.result == CURLE_OK) {
                handle_reply(hnd);
            }

            Active_frequencies* freqs { nullptr };
            curl_easy_getinfo(hnd, CURLINFO_PRIVATE, &freqs);
            freqs->clear();
            curl_multi_remove_handle(curl, hnd);
            curl_multi_add_handle(curl, hnd);
        }
    }
    int runningHandles { 0 };
    curl_multi_perform(curl, &runningHandles);
}

int Vectoraudio_socket::handle_reply(CURL* handle)
{
    long code;
    if (curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code) != CURLE_OK)
        return 1;
    if (code != 200) {
        return 1;
    }

    Active_frequencies* freqs { nullptr };
    curl_easy_getinfo(handle, CURLINFO_PRIVATE, &freqs);
    if (freqs) {
        frequency_pairs pairs = parse_reply(freqs->curl_buffer);
        freqs->set(pairs);
    }

    return 0;
}

frequency_pairs Vectoraudio_socket::parse_reply(const std::string& reply) const
{
    std::vector<std::pair<std::string, std::string>> freqs;

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
        freqs.push_back(std::pair<std::string, std::string> { std::string { token.begin(), pos }, std::string { pos + 1, token.end() } });

        if (end != reply.end())
            begin = ++end;
    } while (end != reply.end());
    return freqs;
}

CURL* Vectoraudio_socket::easy_init(const std::string& url, Active_frequencies& freqs, callback_t call)
{
    auto curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &freqs.curl_buffer);
    curl_easy_setopt(curl, CURLOPT_PRIVATE, &freqs);
    return curl;
}

int Vectoraudio_socket::init_handles()
{
    if (curl_global_init(CURL_GLOBAL_ALL) != 0)
        return 1;
    curl = curl_multi_init();
    rx_handle = easy_init(rx_url, rx_freqs, write_callback);
    auto res = curl_multi_add_handle(curl, rx_handle);
    tx_handle = easy_init(tx_url, tx_freqs, write_callback);
    curl_multi_add_handle(curl, tx_handle);
    return 0;
}

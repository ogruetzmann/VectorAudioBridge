#include "Vectoraudio_Socket.h"

Vectoraudio_socket::Vectoraudio_socket()
{
    initHandles();
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
    return errorState;
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

int Vectoraudio_socket::poll()
{
    CURLMsg* msg { nullptr };
    int msgq { 0 };
    while ((msg = curl_multi_info_read(curl, &msgq))) {
        if (msg->msg == CURLMSG_DONE) {
            CURL* hnd = msg->easy_handle;
            handleReply(hnd);
            curl_multi_remove_handle(curl, hnd);
            curl_multi_add_handle(curl, hnd);
        }
    }
    int runningHandles { 0 };
    curl_multi_perform(curl, &runningHandles);
    return 0;
}

int Vectoraudio_socket::handleReply(CURL* handle)
{
    long code;
    auto res = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);
    Active_frequencies* fHandle { nullptr };
    if (handle == rx_handle)
        fHandle = &rx_freqs;
    if (handle == tx_handle)
        fHandle = &tx_freqs;
    
    if (!fHandle)
        return 1;
    if (res != CURLE_OK || code != 200) {
        fHandle->clear();
        return 1;
    }

    frequency_pairs pairs = parse_reply(fHandle->curl_buffer);
    fHandle->set(pairs);

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

int Vectoraudio_socket::initHandles()
{
    auto res = curl_global_init(CURL_GLOBAL_ALL);
    if (res == 0)
        errorState = false;
    curl = curl_multi_init();

    rx_handle = curl_easy_init();
    curl_easy_setopt(rx_handle, CURLOPT_URL, rx_url.c_str());
    curl_easy_setopt(rx_handle, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(rx_handle, CURLOPT_WRITEDATA, &rx_freqs.curl_buffer);
    curl_multi_add_handle(curl, rx_handle);

    tx_handle = curl_easy_init();
    curl_easy_setopt(tx_handle, CURLOPT_URL, tx_url.c_str());
    curl_easy_setopt(tx_handle, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(tx_handle, CURLOPT_WRITEDATA, &tx_freqs.curl_buffer);
    curl_multi_add_handle(curl, tx_handle);
    return 0;
}

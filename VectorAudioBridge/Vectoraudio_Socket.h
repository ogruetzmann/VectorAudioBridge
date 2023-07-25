#pragma once
#include "Active_frequencies.h"
#include <algorithm>
#include <curl/curl.h>
#include <exception>
#include <string>
#include <utility>
#include <vector>

using frequency_pairs = std::vector<std::pair<std::string, std::string>>;

class Vectoraudio_socket {
public:
    Vectoraudio_socket();
    ~Vectoraudio_socket();

    const bool has_error() const;
    const std::string& error_msg() const;
    const bool rx_changed() const;
    const bool tx_changed() const;
    const frequency_pairs& get_rx();
    const frequency_pairs& get_tx();
    void poll();

private:
    bool error_state { false };
    std::string last_error;

    Active_frequencies rx_freqs {};
    Active_frequencies tx_freqs {};

    CURL* rx_handle { nullptr };
    CURL* tx_handle { nullptr };
    CURLM* curl { nullptr };
    const std::string rx_url = "http://localhost:49080/rx";
    const std::string tx_url = "http://localhost:49080/tx";

    int handle_reply(CURL* handle);
    frequency_pairs parse_reply(const std::string& reply) const;

    using callback_t = size_t (*)(char*, size_t, size_t, std::string*);
    callback_t write_callback = [](char* ptr, size_t size, size_t nmemb, std::string* userdata) -> size_t {
        if (userdata == nullptr)
            return 0;
        userdata->append(ptr, size * nmemb);
        return size * nmemb;
    };

    CURL* easy_init(const std::string& url, Active_frequencies& freqs, callback_t call);
    int init_handles();
};

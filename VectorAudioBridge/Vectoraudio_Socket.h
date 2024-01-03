#pragma once

#include "Active_frequencies.h"
#include <chrono>
#include <curl/curl.h>
#include <functional>
#include <memory>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include "Status.h"
#include "CURL_easy_handler.h"

//using frequency_pairs = std::vector<std::pair<std::string, std::string>>;
using namespace std::chrono_literals;

class Vectoraudio_socket {
public:
    Vectoraudio_socket(std::function<void(std::string, std::string)> msg);
    ~Vectoraudio_socket();

    const bool has_error() const;
    const bool rx_changed();
    const bool tx_changed();
    const frequency_pairs& get_rx();
    const frequency_pairs& get_tx();
    void poll();
    void ping();

private:
    Status status;
    bool error_state{ false };
    bool connected{ false };
    std::string last_error;

    Active_frequencies rx_freqs{};
    Active_frequencies tx_freqs{};

    std::vector<std::unique_ptr<CURL_easy_handler>> handles;
    CURLM* curlm{ nullptr };

    const std::string rx_url{ "http://localhost:49080/rx" };
    const std::string tx_url{ "http://localhost:49080/tx" };
    const std::string version_url{ "http://localhost:49080/" };
    const std::string active_url{ "http://localhost:49080/transmitting" };

    std::stop_source worker_stop;
    std::function<void(std::string, std::string)> message_callback;
    std::function<void(const std::vector<Frequency>& rx, const std::vector<Frequency>& tx)> data_callback;

    void run(std::stop_token token, int interval);
    int handle_reply(CURL* handle);
    frequency_pairs parse_reply(std::string_view reply) const;

    void init_curl();
    void init_handles();
};

size_t write_cb(char*, size_t, size_t, std::string*);

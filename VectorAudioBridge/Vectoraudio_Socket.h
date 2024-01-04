#pragma once
#include "CURL_easy_handler.h"
#include "Status.h"
#include <chrono>
#include <curl/curl.h>
#include <functional>
#include <memory>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;
using message_callback_t = std::function<void(const std::string sender, const std::string message, bool flash, bool unread)>;
using data_callback_t = std::function<void(const CURL_easy_handler::handle_type, const std::string)>;

class Vectoraudio_socket {
public:
    Vectoraudio_socket();
    ~Vectoraudio_socket();

    const bool has_error() const;
    void poll();
    void start();
    void stop();

    void register_data_callback(data_callback_t callback);
    void register_message_callback(message_callback_t callback);

private:
    Status status;
    std::vector<std::unique_ptr<CURL_easy_handler>> handles;
    CURLM* curlm { nullptr };
    const std::string rx_url { "http://localhost:49080/rx" };
    const std::string tx_url { "http://localhost:49080/tx" };
    const std::string version_url { "http://localhost:49080/" };
    const std::string active_url { "http://localhost:49080/transmitting" };

    std::stop_source worker_stop;
    message_callback_t message_callback;
    data_callback_t data_callback;

    void run(std::stop_token token, int interval);
    void handle_reply(CURL* handle);

    void init_curl();
    void init_handles();
};

size_t write_cb(char*, size_t, size_t, std::string*);

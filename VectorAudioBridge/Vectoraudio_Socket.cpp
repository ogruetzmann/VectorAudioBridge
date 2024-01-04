#include "Vectoraudio_Socket.h"

Vectoraudio_socket::Vectoraudio_socket()
{
    init_curl();
    start();
}

Vectoraudio_socket::~Vectoraudio_socket()
{
    worker_stop.request_stop();
    stop();
    curl_multi_cleanup(curlm);
    curl_global_cleanup();
}

const bool Vectoraudio_socket::has_error() const
{
    return false;
}

void Vectoraudio_socket::poll()
{
    if (status.error)
        return;

    CURLMsg* msg { nullptr };
    CURL_easy_handler* handler { nullptr };
    int msgq = 0;
    do {
        msg = curl_multi_info_read(curlm, &msgq);
        if (msg && msg->msg == CURLMSG_DONE) {
            handle_reply(msg->easy_handle);
            curl_multi_remove_handle(curlm, msg->easy_handle);
            curl_multi_add_handle(curlm, msg->easy_handle);
        }
    } while (msg);

    int runningHandles { 0 };
    curl_multi_perform(curlm, &runningHandles);
}

void Vectoraudio_socket::start()
{
    init_handles();
    for (auto& x : handles)
        curl_multi_add_handle(curlm, x.get()->get_handle());
    std::jthread worker(std::bind_front(&Vectoraudio_socket::run, this), 200);
    worker_stop = worker.get_stop_source();
    worker.detach();
}

void Vectoraudio_socket::stop()
{
    worker_stop.request_stop();
    for (auto& x : handles)
        curl_multi_remove_handle(curlm, x.get()->get_handle());
    handles.clear();
}

void Vectoraudio_socket::register_data_callback(data_callback_t callback)
{
    data_callback = callback;
}

void Vectoraudio_socket::register_message_callback(message_callback_t callback)
{
    message_callback = callback;
}

void Vectoraudio_socket::run(std::stop_token token, int interval_ms)
{
    using clock = std::chrono::system_clock;
    clock::time_point start, now;
    while (!token.stop_requested()) {
        now = clock::now();
        if (start.time_since_epoch() == clock::duration::zero() || now - start > status.interval) {
            poll();
            start = clock::now();
        }
        std::this_thread::sleep_for(20ms);
    }
}

void Vectoraudio_socket::handle_reply(CURL* handle)
{
    CURL_easy_handler* handler { nullptr };
    curl_easy_getinfo(handle, CURLINFO_PRIVATE, &handler);
    if (!handler)
        return;
    handler->process(data_callback);
    auto code = handler->get_response_code();
    if (code != 200) {
        status.disconnected();
        return;
    }
    status.connected();
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
    handles.push_back(std::make_unique<CURL_easy_handler>(CURL_easy_handler::handle_type::rx, rx_url));
    handles.push_back(std::make_unique<CURL_easy_handler>(CURL_easy_handler::handle_type::tx, tx_url));
    handles.push_back(std::make_unique<CURL_easy_handler>(CURL_easy_handler::handle_type::active, active_url));
}

size_t write_cb(char* ptr, size_t size, size_t nmemb, std::string* userdata)
{
    if (userdata == nullptr)
        return 0;
    userdata->append(ptr, size * nmemb);
    return size * nmemb;
};

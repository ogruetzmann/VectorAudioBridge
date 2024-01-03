#pragma once
#include "Vectoraudio_Socket.h"
#include <EuroScopePlugIn.h>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace {
    const char* pluginName = "VectorAudioBridge";
    const char* pluginAuthor = "Oliver Gruetzmann";
    const char* pluginVersion = "0.2";
    const char* pluginCopyright = "MIT License";
}

using namespace EuroScopePlugIn;

class Vectoraudio_bridge : public EuroScopePlugIn::CPlugIn {
public:
    Vectoraudio_bridge();
    ~Vectoraudio_bridge();
    void display_message(std::string_view msg);
    void set_frequencies(const frequency_pairs& pairs, const bool tx);

    bool OnCompileCommand(const char* sCommandLine);
    void OnTimer(int counter);

    void data_callback(const CURL_easy_handler::handle_type, const std::string data);
    void message_callback(const std::string sender, const std::string message, bool flash = false, bool unread = false);

private:
    bool active{ true };
    std::unique_ptr<Vectoraudio_socket> socket;
};

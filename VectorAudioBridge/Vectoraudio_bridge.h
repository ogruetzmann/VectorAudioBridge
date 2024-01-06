#pragma once
#include "Frequency.h"
#include "Helpers.h"
#include "Vectoraudio_Socket.h"
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include <EuroScopePlugIn.h>
#include <Windows.h>

using frequency_pairs = std::vector<Frequency>;

namespace {
const char* pluginName = "VectorAudioBridge";
const char* pluginAuthor = "Oliver Gruetzmann";
const char* pluginVersion = "0.2";
const char* pluginCopyright = "MIT License";
std::unique_ptr<EuroScopePlugIn::CPlugIn> plugin;
} // namespace

using namespace EuroScopePlugIn;

class Vectoraudio_bridge : public EuroScopePlugIn::CPlugIn {
public:
    Vectoraudio_bridge();
    ~Vectoraudio_bridge();

    void display_message(std::string_view msg);
    void set_frequencies(const frequency_pairs& pairs, const bool tx);

    bool OnCompileCommand(const char* sCommandLine) override;
    void OnTimer(int counter) noexcept override;

    void data_callback(const CURL_easy_handler::handle_type, const std::string data);
    void message_callback(const std::string sender, const std::string message, bool flash = false, bool unread = false);

private:
    bool active { true };
    std::unique_ptr<Vectoraudio_socket> socket;
    std::mutex freq_set_mutex;
};

#pragma once
#include "Vectoraudio_Socket.h"
#include <EuroScopePlugIn.h>
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
    virtual ~Vectoraudio_bridge();
    void display_message(std::string_view msg);
    void set_frequencies(const frequency_pairs& pairs, const bool tx);

    bool OnCompileCommand(const char* sCommandLine);
    void OnTimer(int counter);

private:
    bool active { true };
    Vectoraudio_socket socket;
};

#include "Vectoraudio_bridge.h"

Vectoraudio_bridge::Vectoraudio_bridge()
    : EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, pluginName, pluginVersion, pluginAuthor, pluginCopyright)
{
}

Vectoraudio_bridge::~Vectoraudio_bridge()
{
}

void Vectoraudio_bridge::display_message(std::string_view msg)
{
    DisplayUserMessage("VectorAudioBridge", "Info: ", msg.data(), true, false, false, false, false);
}

void Vectoraudio_bridge::set_frequencies(const frequency_pairs& pairs, const bool tx)
{
    for (auto gac = EuroScopePlugIn::CPlugIn::GroundToArChannelSelectFirst(); gac.IsValid(); gac = EuroScopePlugIn::CPlugIn::GroundToArChannelSelectNext(gac)) {
        std::string frequency = std::to_string(gac.GetFrequency());
        std::string_view name = gac.GetName();

        bool found { false };
        auto compare = [&](std::pair<std::string, std::string> p) { return name.starts_with(p.first) && frequency.starts_with(p.second); };
        if (std::find_if(pairs.begin(), pairs.end(), compare) != pairs.end())
            found = true;

        if (tx && found != gac.GetIsTextTransmitOn())
            gac.ToggleTextTransmit();
        else if (!tx && found != gac.GetIsTextReceiveOn())
            gac.ToggleTextReceive();
    }
}

bool Vectoraudio_bridge::OnCompileCommand(const char* sCommandLine)
{
    const std::string_view cmd = sCommandLine;
    if (cmd.starts_with(".vab start")) {
        active = true;
        display_message("Brigde active");
        return true;
    }
    if (cmd.starts_with(".vab stop")) {
        display_message("Brigde inactive");
        active = false;
        return true;
    }
    return false;
}

void Vectoraudio_bridge::OnTimer(int counter)
{
    if (!active)
        return;

    if (!socket.has_error())
        socket.poll();

    if (socket.rx_changed()) {
        set_frequencies(socket.get_rx(), false);
    }
    if (socket.tx_changed()) {
        set_frequencies(socket.get_tx(), true);
    }
}

#include "Vectoraudio_bridge.h"

Vectoraudio_bridge::Vectoraudio_bridge()
    : EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, pluginName, pluginVersion, pluginAuthor, pluginCopyright)
{
    auto msg = [this](const std::string type, const std::string msg) { DisplayUserMessage("VectorAudioBridge", type.c_str(), msg.c_str(), true, false, false, false, false); };

    try {
        socket = std::make_unique<Vectoraudio_socket>(msg);
    } catch (std::exception& e) {
        display_message(e.what());
    }
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
    std::string_view cmd = sCommandLine;
    if (cmd == ".vab start") {
        active = true;
        display_message("Brigde active");
        return true;
    }
    if (cmd == ".vab stop") {
        display_message("Brigde inactive");
        active = false;
        return true;
    }
    return false;
}

void Vectoraudio_bridge::OnTimer(int counter)
{
    if (socket && active && !socket->has_error()) {
        // try {
        //     socket->poll();
        // } catch (std::exception& e) {
        //     display_message(e.what());
        // }
        // if (socket->rx_changed())
        //     set_frequencies(socket->get_rx(), false);
        // if (socket->tx_changed())
        //     set_frequencies(socket->get_tx(), true);
    }
}

std::unique_ptr<EuroScopePlugIn::CPlugIn> plugin;

void EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
    plugin.reset(new Vectoraudio_bridge);
    *ppPlugInInstance = plugin.get();
}

void EuroScopePlugInExit(void)
{
}
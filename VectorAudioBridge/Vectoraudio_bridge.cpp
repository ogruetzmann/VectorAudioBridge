#include "Vectoraudio_bridge.h"

Vectoraudio_bridge::Vectoraudio_bridge()
    : EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, pluginName, pluginVersion, pluginAuthor, pluginCopyright)
{
    try {
        socket = std::make_unique<Vectoraudio_socket>();
    } catch (std::exception& e) {
        display_message(e.what());
    }
    using namespace std::placeholders;
    socket->register_data_callback(std::bind(&Vectoraudio_bridge::data_callback, this, _1, _2));
    socket->register_message_callback(std::bind(&Vectoraudio_bridge::message_callback, this, _1, _2, _3, _4));
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
        auto compare = [&](Frequency p) { return name.starts_with(p.name) && frequency.starts_with(p.frequency); };
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
}

void Vectoraudio_bridge::data_callback(const CURL_easy_handler::handle_type type, const std::string data)
{
    if (type == CURL_easy_handler::handle_type::rx || type == CURL_easy_handler::handle_type::tx) {
        std::vector<Frequency> frequencies;
        auto res = helpers::tokenize(data, ',');
        for (const auto& x : res) {
            auto res2 = helpers::tokenize(x, ':');
            if (res2.size() == 2) {
                frequencies.push_back({ res2[0], res2[1] });
            }
        }
        set_frequencies(frequencies, type == CURL_easy_handler::handle_type::tx ? true : false);
    }

    if (type == CURL_easy_handler::handle_type::active) {

    }
}

void Vectoraudio_bridge::message_callback(const std::string sender, const std::string message, bool flash, bool unread)
{
    DisplayUserMessage("VectorAudioBridge", sender.c_str(), message.c_str(), true, unread, true, flash, false);
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
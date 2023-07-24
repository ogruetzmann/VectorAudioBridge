#include "Vectoraudio_bridge.h"
#include <EuroScopePlugIn.h>
#include <memory>

std::unique_ptr<EuroScopePlugIn::CPlugIn> plugin;

void EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
{
    plugin.reset(new Vectoraudio_bridge);
    *ppPlugInInstance = plugin.get();
}

void EuroScopePlugInExit(void)
{
}
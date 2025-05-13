#include <Hooks.hpp>
#include <Nexus.h>
#include <stdexcept>

void AddonLoad(AddonAPI *aApi);
void AddonUnload();

namespace G
{
AddonAPI *APIDefs = nullptr;
} // namespace G

extern "C" __declspec(dllexport) AddonDefinition *GetAddonDef();

AddonDefinition *GetAddonDef()
{
    static AddonDefinition def{
        .Signature = 0x776E7A32,
        .APIVersion = NEXUS_API_VERSION,
        .Name = "Transparent Heart Progress",
        .Version = AddonVersion{ADDON_VERSION_MAJOR, ADDON_VERSION_MINOR, ADDON_VERSION_PATCH, ADDON_VERSION_REVISION},
        .Author = "Vonsh.1427",
        .Description = "Detailed display of renown heart progression",
        .Load = AddonLoad,
        .Unload = AddonUnload,
        .Flags = EAddonFlags_None,
        .Provider = EUpdateProvider_GitHub,
        .UpdateLink = "https://github.com/jsantorek/GW2-" ADDON_NAME,
    };
    return &def;
}

void AddonLoad(AddonAPI *aApi)
{
    G::APIDefs = aApi;
    try
    {
        THP::Hooks::Enable();
    }
    catch (const std::runtime_error &e)
    {
        G::APIDefs->Log(ELogLevel_CRITICAL, ADDON_NAME, e.what());
    }
}

void AddonUnload()
{
    THP::Hooks::Disable();
    G::APIDefs = nullptr;
}

#include "Addon.hpp"
#include "Config.hpp"
#include <Nexus.h>
#include <imgui.h>
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
    ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext *>(G::APIDefs->ImguiContext));
    ImGui::SetAllocatorFunctions(reinterpret_cast<void *(*)(size_t, void *)>(G::APIDefs->ImguiMalloc),
                                 reinterpret_cast<void (*)(void *, void *)>(G::APIDefs->ImguiFree));
    try
    {
        THP::Config::Load();
        Addon::SetUp(aApi);

        G::APIDefs->Renderer.Register(ERenderType_OptionsRender, THP::Config::Render);
    }
    catch (const std::runtime_error &e)
    {
        G::APIDefs->Log(ELogLevel_CRITICAL, ADDON_NAME, e.what());
    }
}

void AddonUnload()
{
    Addon::TearDown();
    G::APIDefs->Renderer.Deregister(THP::Config::Render);
    THP::Config::Save();
    G::APIDefs = nullptr;
}

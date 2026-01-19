#include "Addon.hpp"
#include "Config.hpp"
#include <Nexus.h>
#include <imgui.h>
#include <stdexcept>

void AddonLoad(AddonAPI *aApi);
void AddonUnload();

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

static AddonAPI *Api = nullptr;
void AddonLoad(AddonAPI *aApi)
{
    Api = std::move(aApi);
    ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext *>(Api->ImguiContext));
    ImGui::SetAllocatorFunctions(reinterpret_cast<void *(*)(size_t, void *)>(Api->ImguiMalloc),
                                 reinterpret_cast<void (*)(void *, void *)>(Api->ImguiFree));
    try
    {
        Config::Load(Api);
        Addon::SetUp(Api);

        Api->Renderer.Register(ERenderType_OptionsRender, Config::Render);
    }
    catch (const std::runtime_error &e)
    {
        Api->Log(ELogLevel_CRITICAL, ADDON_NAME, e.what());
    }
}

void AddonUnload()
{
    Addon::TearDown();
    Api->Renderer.Deregister(Config::Render);
    Config::Save(Api);
    Api = nullptr;
}

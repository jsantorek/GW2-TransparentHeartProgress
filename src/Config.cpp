#include "Config.hpp"
#include "Hooks.hpp"
#include <Nexus.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <nlohmann/json.hpp>

namespace G
{
extern AddonAPI *APIDefs;
} // namespace G

int THP::Config::Decimals = THP::Config::DecimalsMax;

void THP::Config::Load()
{
    auto filepath = std::filesystem::path(G::APIDefs->Paths.GetAddonDirectory(ADDON_NAME)) / ConfigFilename;
    if (std::filesystem::exists(filepath))
    {
        auto json = nlohmann::json::object();
        json = nlohmann::json::parse(std::ifstream(filepath), nullptr, false);
        json.at("Decimals").get_to(Decimals);
    }
}

void THP::Config::Save()
{
    auto filepath = std::filesystem::path(G::APIDefs->Paths.GetAddonDirectory(ADDON_NAME)) / ConfigFilename;
    if (Decimals == DecimalsMax)
    {
        if (std::filesystem::exists(filepath))
            std::filesystem::remove_all(filepath.parent_path());
    }
    else
    {
        if (!std::filesystem::exists(filepath.parent_path()))
            std::filesystem::create_directories(filepath.parent_path());
        auto json = nlohmann::json::object();
        json["Decimals"] = std::clamp(Decimals, DecimalsMin, DecimalsMax);
        std::ofstream(filepath) << json;
    }
}
void THP::Config::Render()
{
    if (ImGui::InputInt("Decimal precision", &Decimals))
    {
        Decimals = std::clamp(Decimals, DecimalsMin, DecimalsMax);
        THP::Hooks::Refresh();
    }
}

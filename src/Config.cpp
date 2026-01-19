#include "Config.hpp"
#include "Addon.hpp"
#include <Nexus.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <nlohmann/json.hpp>

NLOHMANN_JSON_SERIALIZE_ENUM(Config::TieredHeartDisplayMode,
                             {{Config::TieredHeartDisplayMode::Never, "never"},
                              {Config::TieredHeartDisplayMode::MasteryTierProgress, "mastery_tier_progress"},
                              {Config::TieredHeartDisplayMode::MasteryOverallProgress, "mastery_overall_progress"},
                              {Config::TieredHeartDisplayMode::TotalAchievementProgress, "total_achievement_progress"}})

int Config::Decimals = Config::DecimalsMax;
Config::TieredHeartDisplayMode Config::TieredHeartDisplay = TieredHeartDisplayMode::TotalAchievementProgress;
bool Config::FixTieredAchievements = true;

void Config::Load(AddonAPI *aApi)
{
    auto filepath = std::filesystem::path(aApi->Paths.GetAddonDirectory(ADDON_NAME)) / ConfigFilename;
    if (std::filesystem::exists(filepath))
    {
        auto json = nlohmann::json::object();
        json = nlohmann::json::parse(std::ifstream(filepath), nullptr, false);

        if (json.contains("Decimals"))
            Decimals = json.at("Decimals");
        if (json.contains("TieredHeartDisplay"))
            TieredHeartDisplay = json.at("TieredHeartDisplay");
        if (json.contains("FixTieredAchievements"))
            FixTieredAchievements = json.at("FixTieredAchievements");
    }
}

void Config::Save(AddonAPI *aApi)
{
    auto filepath = std::filesystem::path(aApi->Paths.GetAddonDirectory(ADDON_NAME)) / ConfigFilename;
    if (!std::filesystem::exists(filepath.parent_path()))
        std::filesystem::create_directories(filepath.parent_path());
    auto json = nlohmann::json::object();
    json["Decimals"] = std::clamp(Decimals, DecimalsMin, DecimalsMax);
    json["TieredHeartDisplay"] = TieredHeartDisplay;
    json["FixTieredAchievements"] = FixTieredAchievements;
    std::ofstream(filepath) << json;
}
void Config::Render()
{
    if (ImGui::InputInt("Decimal precision", &Decimals))
    {
        Decimals = std::clamp(Decimals, DecimalsMin, DecimalsMax);
        Addon::Reconfigure();
    }
    ImGui::Checkbox(
        "Fix missing achievement associations for Tiered Renown Hearts (change effective upon game's restart)",
        &FixTieredAchievements);
    static const char *items[] = {
        "Never",
        "Mastery Tier Progress",
        "Mastery Overall Progress",
        "Total Achievement Progress",
    };
    const char *combo_preview_value = items[static_cast<int>(TieredHeartDisplay)];
    if (ImGui::BeginCombo("Tiered Heart Display", combo_preview_value))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            const bool is_selected = (static_cast<int>(TieredHeartDisplay) == n);
            if (ImGui::Selectable(items[n], is_selected))
            {
                TieredHeartDisplay = static_cast<TieredHeartDisplayMode>(n);
                Addon::Reconfigure();
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

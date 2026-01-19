#pragma once

#include <Nexus.h>
struct Config
{
    static constexpr auto DecimalsMin = 0;
    static constexpr auto DecimalsMax = 2;
    static constexpr auto ConfigFilename = "Config.json";

    enum class TieredHeartDisplayMode
    {
        Never,
        MasteryTierProgress,
        MasteryOverallProgress,
        TotalAchievementProgress
    };
    static TieredHeartDisplayMode TieredHeartDisplay;
    static int Decimals;
    static bool FixTieredAchievements;
    static void Load(AddonAPI *aApi);
    static void Save(AddonAPI *aApi);
    static void Render();
};

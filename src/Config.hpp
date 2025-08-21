#pragma once

namespace THP
{
struct Config
{
    static constexpr auto DecimalsMin = 0;
    static constexpr auto DecimalsMax = 2;
    static constexpr auto ConfigFilename = "Config.json";

    static int Decimals;
    static void Load();
    static void Save();
    static void Render();
};
} // namespace THP

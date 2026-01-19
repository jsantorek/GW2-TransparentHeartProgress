#pragma once

#include <cstdint>

struct TieredHeart
{
    uint32_t CurrentTier;
    uint32_t MaxTier;

    uint32_t CurrentCompletions;
    uint32_t TiersMaxCompletions;
    uint32_t PointsMaxCompletions;
};

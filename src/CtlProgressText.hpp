#pragma once

#include "FrameExtension.hpp"
#include "Game/Achievement/AchievementDef.h"
#include "Game/Frame/Frame.h"
#include "Game/Task/TaskManager.h"
#include "TieredHeart.hpp"
#include <optional>

class CtlProgressText : private GW2RE::ITaskNotificationHandler, public FrameExtension
{
  public:
    ~CtlProgressText();
    void Proc(GW2RE::HDR_t *, void *, void *) override;
    void Initialize(GW2RE::CFrame);
    inline void SetRepeatableAchievements(std::vector<GW2RE::AchievementDef_t *> achievements)
    {
        RepeatableAchievements = std::move(achievements);
    }
    void RefreshText();
    void OnTaskSubRegionEnter(GW2RE::TaskManager_t *, GW2RE::TaskSubRegionCommonDef_t *) override;
    void OnTaskNotifierCleanup(GW2RE::TaskManager_t *) override;
    void UpdateAchievementStats();

  private:
    void UpdateText(uint32_t);
    uint32_t TextId = 0;
    bool IsAttached = false;
    GW2RE::AchievementDef_t *Achievement = nullptr;
    std::optional<TieredHeart> Stats = std::nullopt;
    std::vector<GW2RE::AchievementDef_t *> RepeatableAchievements = {};
};

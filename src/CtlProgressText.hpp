#pragma once

#include "FrameExtension.hpp"
#include "Game/Achievement/AchCollectionStats.h"
#include "Game/Frame/Frame.h"
#include "Game/Task/TaskManager.h"

class CtlProgressText : private GW2RE::ITaskNotificationHandler, public FrameExtension
{
  public:
    ~CtlProgressText();
    void Proc(GW2RE::HDR_t *, void *, void *) override;
    void Initialize(GW2RE::CFrame);
    void RefreshText();
    void OnTaskSubRegionEnter(GW2RE::TaskManager_t *, GW2RE::TaskSubRegionCommonDef_t *) override;
    void OnTaskNotifierCleanup(GW2RE::TaskManager_t *) override;

  private:
    void UpdateText(uint32_t);
    uint32_t TextId = 0;
    bool IsAttached = false;
    GW2RE::AchievementCollectionStats_t CollectionStats;
};

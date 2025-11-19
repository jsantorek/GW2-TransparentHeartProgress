#pragma once

#include "CtlProgressText.hpp"
#include "FrameExtension.hpp"
#include "Game/Frame/Frame.h"
#include "Game/VdfContext.h"
#include <Nexus.h>

class Addon final : private GW2RE::IVdfNotificationHandler, FrameExtension
{
  public:
    static void SetUp(AddonAPI *api);
    static void TearDown(GW2RE::UNKNOWN = nullptr, GW2RE::UNKNOWN = nullptr);
    static void Reconfigure();

  private:
    Addon() = default;
    void OnGameViewChanged(GW2RE::UNKNOWN, GW2RE::EGameView) override;
    void Proc(GW2RE::HDR_t *, void *, void *) override;
    void ExtendOblTask();
    void ExtendCtlProgress(GW2RE::CFrame);

    static AddonAPI *API;
    static Addon *Self;

    CtlProgressText ProgressText;
};
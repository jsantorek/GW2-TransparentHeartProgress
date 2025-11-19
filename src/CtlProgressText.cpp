#include "CtlProgressText.hpp"
#include "Game/Achievement/AchManager.h"
#include "Game/Char/ChCliContext.h"
#include "Game/Frame/EFrMessage.h"
#include "Game/Frame/FrameApi.h"
#include "Game/Player/Player.h"
#include "Game/PropContext.h"
#include "Game/Task/TaskManager.h"
#include "Game/Task/TaskSubRegionCommonDef.h"
#include <Config.hpp>
#include <Game/Controls/CtlProgress.h>
#include <Game/Controls/CtlText.h>
#include <format>

namespace
{
inline GW2RE::CPlayer GetControlledPlayer()
{
    GW2RE::CCharCliContext ctx = GW2RE::CPropContext::Get().GetCharCliCtx();
    return ctx.GetControlledPlayer();
}
} // namespace

void CtlProgressText::Proc(GW2RE::HDR_t *msg, void *a2, void *)
{
    if (static_cast<GW2RE::CtlProgress::EMessage>(msg->Type) != GW2RE::CtlProgress::EMessage::SetValues)
        return;
    UpdateText(reinterpret_cast<GW2RE::CtlProgress::Values_t *>(a2)->Current);
}

void CtlProgressText::Initialize(GW2RE::CFrame ctlProgress)
{
    GW2RE::CPlayer plr = GetControlledPlayer();
    GW2RE::CTaskManager tmgr = plr.GetTaskMgr();
    if (auto active = tmgr.GetActiveTaskSubRegion())
    {
        GW2RE::CAchManager amgr = plr.GetAchievementMgr();
        CollectionStats = amgr.GetCollectionStats(active->Achievement);
    }
    else
    {
        CollectionStats = {};
    }
    if (!IsAttached)
    {
        tmgr.AttachHandler(this);
        IsAttached = true;
    }
    auto text = ctlProgress.GetDescendant<GW2RE::CtlProgress::Text>();
    TextId = text.null() ? GW2RE::FrameApi::RegisterChildFrame(
                               ctlProgress, static_cast<GW2RE::EFrameFlags>(GW2RE::CtlText::EFlags::Centered),
                               GW2RE::CtlProgress::Text, GW2RE::CtlText::GetProc())
                         : text;
    auto style = GW2RE::CtlText::EStyle::Text_24;
    GW2RE::FrameApi::PostFrameMessage(TextId, static_cast<GW2RE::EFrameMessage>(GW2RE::CtlText::EMessage::SetStyle),
                                      &style);
    RefreshText();
}

void CtlProgressText::OnTaskSubRegionEnter(GW2RE::TaskManager_t *, GW2RE::TaskSubRegionCommonDef_t *region)
{
    GW2RE::CAchManager amgr = GetControlledPlayer().GetAchievementMgr();
    CollectionStats = amgr.GetCollectionStats(region->Achievement);
}

void CtlProgressText::OnTaskNotifierCleanup(GW2RE::TaskManager_t *mgr)
{
    if (IsAttached)
    {
        GW2RE::CTaskManager(mgr).DetachHandler(this);
        IsAttached = false;
    }
}

CtlProgressText::~CtlProgressText()
{
    if (IsInstalled())
    {
        GW2RE::FrameApi::Destroy(TextId);
    }
    if (IsAttached)
    {
        GW2RE::CCharCliContext ctx = GW2RE::CPropContext::Get().GetCharCliCtx();
        GW2RE::CPlayer plr = ctx.GetControlledPlayer();
        GW2RE::CTaskManager mgr = plr.GetTaskMgr();
        mgr.DetachHandler(this);
    }
}

void CtlProgressText::RefreshText()
{
    if (auto ctlProgress = GetHostID())
    {
        float ratio;
        GW2RE::FrameApi::PostFrameMessage(
            ctlProgress, static_cast<GW2RE::EFrameMessage>(GW2RE::CtlProgress::EMessage::GetTargetRatio), nullptr,
            &ratio);
        UpdateText(0.5 + (ratio * 10000.));
    }
}

void CtlProgressText::UpdateText(uint32_t Current)
{
    std::wstring str;
    if (THP::Config::Decimals <= 0)
        str = std::format(L"{}%", uint32_t(Current / 100));
    else if (THP::Config::Decimals == 1)
        str = std::format(L"{}.{:01}%", uint32_t(Current / 100), uint32_t(Current / 10) % 10);
    else
        str = std::format(L"{}.{:02}%", uint32_t(Current / 100), Current % 100);
    GW2RE::FrameApi::PostFrameMessage(TextId, static_cast<GW2RE::EFrameMessage>(GW2RE::CtlText::EMessage::SetRawText),
                                      str.c_str());
}

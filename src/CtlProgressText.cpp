#include "CtlProgressText.hpp"
#include "Game/Achievement/AchManager.h"
#include "Game/Char/ChCliContext.h"
#include "Game/Controls/CtlProgress.h"
#include "Game/Controls/CtlText.h"
#include "Game/Player/Player.h"
#include "Game/PropContext.h"
#include "Game/Task/TaskSubRegionCommonDef.h"
#include "TieredHeart.hpp"
#include <Config.hpp>
#include <Nexus.h>
#include <algorithm>
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
    if (Achievement && msg->Type == GW2RE::EFrameMessage::Update)
    {
        UpdateAchievementStats();
        RefreshText();
    }
    else if (static_cast<GW2RE::CtlProgress::EMessage>(msg->Type) != GW2RE::CtlProgress::EMessage::SetValues)
        return;
    else
        UpdateText(reinterpret_cast<GW2RE::CtlProgress::Values_t *>(a2)->Current);
}

void CtlProgressText::Initialize(GW2RE::CFrame ctlProgress)
{
    GW2RE::CPlayer plr = GetControlledPlayer();
    GW2RE::CTaskManager tmgr = plr.GetTaskMgr();
    const auto active = tmgr.GetActiveTaskSubRegion();
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
    const auto style = GW2RE::CtlText::EStyle::Text_24;
    GW2RE::FrameApi::PostFrameMessage(TextId, static_cast<GW2RE::EFrameMessage>(GW2RE::CtlText::EMessage::SetStyle),
                                      &style);
    if (!active || active->Achievement == nullptr)
    {
        Achievement = nullptr;
        Stats = std::nullopt;
        RefreshText();
    }
    else
        Achievement = active->Achievement;
}

void CtlProgressText::OnTaskSubRegionEnter(GW2RE::TaskManager_t *, GW2RE::TaskSubRegionCommonDef_t *region)
{
    Achievement = region->Achievement;
    UpdateAchievementStats();
}

void CtlProgressText::UpdateAchievementStats()
{
    if (Achievement == nullptr)
    {
        Stats = std::nullopt;
        return;
    }
    GW2RE::CAchManager amgr = GetControlledPlayer().GetAchievementMgr();
    const auto MaxCompletions = Achievement->Progress->Type == GW2RE::EProgressType::BitMask
                                    ? Achievement->Progress->BitMask->CompleteCount
                                    : 1;
    const auto cmpl = amgr.GetCompletionStats(Achievement);
    Stats = TieredHeart{cmpl.CurrentTier, cmpl.TotalTier, cmpl.Progress, MaxCompletions, MaxCompletions};
    auto it = std::find_if(RepeatableAchievements.begin(), RepeatableAchievements.end(),
                           [&](const auto &ach) { return ach->Prerequisite == Achievement; });
    if (it != RepeatableAchievements.end())
    {
        const auto ach = *it;
        const auto rept = amgr.GetCompletionStats(ach);
        uint32_t TotalCount = 0, TotalPoints = 0;
        for (const auto &[count, points] : ach->Tiers)
        {
            TotalPoints += points;
            TotalCount += count;
        }
        Stats->CurrentCompletions += rept.Repeatitions * TotalCount;
        Stats->CurrentCompletions += rept.Progress;
        auto repeatitionsForMax = (ach->PointCap + TotalPoints - 1) / TotalPoints;
        Stats->PointsMaxCompletions += repeatitionsForMax * TotalCount;
    }
    Achievement = nullptr;
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
        GW2RE::CTaskManager mgr = GetControlledPlayer().GetTaskMgr();
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
    if (Config::Decimals <= 0)
        str = std::format(L"{}%", uint32_t(Current / 100));
    else if (Config::Decimals == 1)
        str = std::format(L"{}.{:01}%", uint32_t(Current / 100), uint32_t(Current / 10) % 10);
    else
        str = std::format(L"{}.{:02}%", uint32_t(Current / 100), Current % 100);
    if (Stats.has_value())
    {
        switch (Config::TieredHeartDisplay)
        {
        case Config::TieredHeartDisplayMode::MasteryTierProgress:
            str = std::format(L"{} {}/{}", str, Stats->CurrentTier, Stats->MaxTier);
            break;
        case Config::TieredHeartDisplayMode::MasteryOverallProgress:
            str = std::format(L"{} {}/{}", str, Stats->CurrentCompletions, Stats->TiersMaxCompletions);
            break;
        case Config::TieredHeartDisplayMode::TotalAchievementProgress:
            str = std::format(L"{} {}/{}", str, Stats->CurrentCompletions, Stats->PointsMaxCompletions);
            break;
        case Config::TieredHeartDisplayMode::Never:
        default:
            break;
        }
    }
    GW2RE::FrameApi::PostFrameMessage(TextId, static_cast<GW2RE::EFrameMessage>(GW2RE::CtlText::EMessage::SetRawText),
                                      str.c_str());
}

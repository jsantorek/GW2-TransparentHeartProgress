#include "Addon.hpp"
#include "Game/Content/ContentContext.h"
#include "Game/PropContext.h"
#include "Game/Task/TaskSubRegionCommonDef.h"
#include "Util/Validation.h"
#include <Config.hpp>
#include <FrameRelations.hpp>
#include <Nexus.h>
#include <format>
#include <mutex>
#include <stdexcept>

AddonAPI *Addon::API = nullptr;
Addon *Addon::Self = nullptr;

void Addon::Reconfigure()
{
    if (Self)
    {
        Self->ProgressText.RefreshText();
    }
}

void Addon::SetUp(AddonAPI *api)
{
    API = api;
    if (Self)
    {
        return;
    }
    if (auto err = GW2RE::RunDiag(); !err.empty())
    {
        throw std::runtime_error(err);
    }
    Self = new Addon();
    auto ctx = GW2RE::CVdfContext::Get();
    ctx.AttachHandler(Self);
    Self->OnGameViewChanged(GW2RE::UNKNOWN{}, ctx.GetGameView());
}

void Addon::TearDown(GW2RE::UNKNOWN, GW2RE::UNKNOWN)
{
    static std::mutex Mutex;
    const std::lock_guard<std::mutex> lock(Mutex);
    if (!Self)
    {
        return;
    }
    GW2RE::CVdfContext::Get().DetachHandler(Self);
    delete Self;
    Self = nullptr;
}

void Addon::OnGameViewChanged(GW2RE::UNKNOWN, GW2RE::EGameView view)
{
    if (view == GW2RE::EGameView::Gameplay)
    {
        ExtendOblTask();
    }
}

void Addon::InitializeTieredTasks()
{
    GW2RE::CContentCtx ctx = GW2RE::CPropContext::Get().GetContentCtx();
    auto tasks = ctx.GetContentStream<GW2RE::TaskSubRegionCommonDef_t>(GW2RE::EContentType::TaskSubRegionCommonDef);
    auto tiered = std::vector<GW2RE::TaskSubRegionCommonDef_t *>{};
    while (auto task = tasks.next())
    {
        if (Config::FixTieredAchievements && task->ID == 456 && task->Achievement == nullptr)
        {
            auto achievement =
                ctx.GetContent<GW2RE::AchievementDef_t>(GW2RE::GUID_t(0x9A06A705, 0x4880F5C7, 0xAE6BE187, 0xF6E37831));
            if (achievement->ID != 8605)
            {
                API->Log(ELogLevel_WARNING, ADDON_NAME, "Mistburned Barrens Task achievement fix failed");
            }
            else
            {
                task->Achievement = achievement;
            }
        }
        if (task->Achievement)
        {
            tiered.emplace_back(task);
        }
    }
    auto repeatable = std::vector<GW2RE::AchievementDef_t *>{};
    auto achievements = ctx.GetContentStream<GW2RE::AchievementDef_t>(GW2RE::EContentType::AchievementDef);
    while (auto ach = achievements.next())
    {
        if (!ach->Prerequisite || !ach->Repeatable)
        {
            continue;
        }
        auto it = std::find_if(tiered.begin(), tiered.end(),
                               [ach](auto const &task) { return ach->Prerequisite == task->Achievement; });
        if (it != tiered.end())
        {
            repeatable.emplace_back(ach);
            API->Log(ELogLevel_TRACE, ADDON_NAME,
                     std::format("Task#{} PreAch#{} -> RepAch#{}", (*it)->ID, (*it)->Achievement->ID, ach->ID).c_str());
        }
    }
    ProgressText.SetRepeatableAchievements(std::move(repeatable));
}

void Addon::Proc(GW2RE::HDR_t *msg, void *, void *)
{
    std::call_once(HasInitialized, [&]() { InitializeTieredTasks(); });
    if (msg->Type != (GW2RE::EFrameMessage)1 && msg->Type != GW2RE::EFrameMessage::SetVisible)
        return;
    ExtendCtlProgress(GW2RE::FrameApi::GetFrame(msg->ID));
}

void Addon::ExtendCtlProgress(GW2RE::CFrame oblTask)
{
    using namespace GW2RE;
    auto ctlProgress =
        oblTask.GetDescendant<OblTask::Objective, OblObjectiveCounter::ProgressBar, OblProgressBar::Progress>();
    if (ctlProgress.null())
    {
        API->Log(ELogLevel_WARNING, ADDON_NAME, "Failed to fetch CtlProgress");
    }
    else if (ProgressText.TryInstall(&ctlProgress->Msg))
    {
        API->Log(ELogLevel_INFO, ADDON_NAME, "Installed CtlProgress extension");
        ProgressText.Initialize(std::move(ctlProgress));
    }
    else
    {
        API->Log(ELogLevel_INFO, ADDON_NAME, "Failed to install CtlProgress extension");
    }
}

void Addon::ExtendOblTask()
{
    using namespace GW2RE;
    CFrame uiRoot = FrameApi::GetUiRoot();
    auto oblTask = uiRoot.GetDescendant<UiRoot_<EGameView::Gameplay>::Objectives, OblCatalog::RenownTask,
                                        OblAnimationFrame<TaskRegionIcon, OblTask>::Content>();
    if (oblTask.null())
    {
        API->Log(ELogLevel_WARNING, ADDON_NAME, "Failed to fetch OblTask");
    }
    else if (TryInstall(&oblTask->Msg))
    {
        API->Log(ELogLevel_INFO, ADDON_NAME, "Installed OblTask extension");
        ExtendCtlProgress(std::move(oblTask));
    }
    else
    {
        API->Log(ELogLevel_INFO, ADDON_NAME, "Failed to install OblTask extension");
    }
}

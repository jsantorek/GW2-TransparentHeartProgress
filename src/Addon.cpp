#include "Addon.hpp"
#include "Game/Frame/EFrMessage.h"
#include "Game/Frame/Frame.h"
#include "Game/Frame/FrameApi.h"
#include "Game/Types.h"
#include "Game/VdfContext.h"
#include "Util/Validation.h"
#include <FrameRelations.hpp>
#include <Nexus.h>
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

void Addon::Proc(GW2RE::HDR_t *msg, void *, void *)
{
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

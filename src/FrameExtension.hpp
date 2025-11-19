#pragma once
#include <Game/Frame/FrMsg.h>
#include <Game/Frame/FrameApi.h>
#include <mutex>

class FrameExtension
{
  public:
    FrameExtension() = default;

    virtual ~FrameExtension()
    {
        Uninstall();
    }

    virtual bool TryInstall(GW2RE::FrMsg_t *host)
    {
        auto lock = std::lock_guard(Mutex);
        if (IsInstalled())
            return false;
        if (host->ClassCount >= GW2RE::FrMsg_t::MaxClasses)
            return false;
        Host = host;
        Host->Classes[Host->ClassCount].Proc = FrameExtension::Handle;
        Host->Classes[Host->ClassCount].Param = this;
        Host->Classes[Host->ClassCount].Flags = GW2RE::EFrameClassFlags::IsDispatchable;
        Host->ClassCount++;
        return true;
    }

    virtual void Uninstall()
    {
        auto lock = std::lock_guard(Mutex);
        if (!IsInstalled())
        {
            return;
        }
        for (auto index = 0; index < GW2RE::FrMsg_t::MaxClasses; index++)
        {
            if (Host->Classes[index].Param != this)
            {
                continue;
            }
            Host->ClassCount--;
            Host->Classes[index].Flags = static_cast<GW2RE::EFrameClassFlags>(0);
            Host->Classes[index].Proc = nullptr;
            Host->Classes[index].Param = nullptr;
            break;
        }
        Host = nullptr;
    }

    FrameExtension(const FrameExtension &) = delete;
    FrameExtension(FrameExtension &&) = delete;
    FrameExtension &operator=(const FrameExtension &) = delete;
    FrameExtension &operator=(FrameExtension &&) = delete;

  protected:
    virtual void Proc(GW2RE::HDR_t *, void *, void *) = 0;
    inline uint32_t GetHostID() const
    {
        return Host ? Host->ID : 0;
    }

    inline bool IsInstalled() const
    {
        return Host != nullptr;
    }

  private:
    static void Handle(GW2RE::HDR_t *a1, void *a2, void *a3)
    {
        if (auto ptr = a1->GetParam<FrameExtension>())
        {
            if (a1->Type == GW2RE::EFrameMessage::Destroy)
            {
                ptr->Uninstall();
            }
            else
            {
                auto lock = std::lock_guard(ptr->Mutex);
                if (ptr->IsInstalled()) // could have been uninstalled when waiting for mutex
                {
                    ptr->Proc(a1, a2, a3);
                }
            }
        }
        GW2RE::FrameApi::PassMessage(a1, a2, a3);
    }

    std::recursive_mutex Mutex;
    GW2RE::FrMsg_t *Host = nullptr;
};

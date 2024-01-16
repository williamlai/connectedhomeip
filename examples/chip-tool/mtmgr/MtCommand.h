#pragma once

#include "MatterManagerCore.h"

class MtCommand
{
public:
    using ChipDeviceCommissioner = ::chip::Controller::DeviceCommissioner;
    using ChipDeviceController   = ::chip::Controller::DeviceController;
    using IPAddress              = ::chip::Inet::IPAddress;
    using NodeId                 = ::chip::NodeId;
    using PeerId                 = ::chip::PeerId;
    using PeerAddress            = ::chip::Transport::PeerAddress;

    struct AddressWithInterface
    {
        ::chip::Inet::IPAddress address;
        ::chip::Inet::InterfaceId interfaceId;
    };

    MtCommand(MatterManagerCore & mtmgrCore) : mMtmgrCore(mtmgrCore) {}
    virtual ~MtCommand() {}

    CHIP_ERROR Run();

    virtual CHIP_ERROR RunCommand() = 0;

    virtual chip::System::Clock::Timeout GetWaitDuration() const = 0;

    void SetCommandExitStatus(CHIP_ERROR status);
    CHIP_ERROR StartWaiting(chip::System::Clock::Timeout seconds);
    void StopWaiting();

protected:
    MatterManagerCore & mMtmgrCore;

private:
    std::mutex cvWaitingForResponseMutex;
    bool mWaitingForResponse;

    CHIP_ERROR mCommandExitStatus = CHIP_ERROR_INTERNAL;
};
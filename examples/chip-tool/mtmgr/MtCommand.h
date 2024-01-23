/*
 * Copyright 2023-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

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
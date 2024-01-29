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

#include "MtCommand.h"

CHIP_ERROR MtCommand::Run()
{
    chip::System::Clock::Timeout duration = GetWaitDuration();

    mCommandExitStatus = RunCommand();
    if (mCommandExitStatus == CHIP_NO_ERROR)
    {
        mCommandExitStatus = StartWaiting(GetWaitDuration());
    }

    return mCommandExitStatus;
}

static void OnResponseTimeout(chip::System::Layer *, void * appState)
{
    auto command = reinterpret_cast<MtCommand *>(appState);
    command->SetCommandExitStatus(CHIP_ERROR_TIMEOUT);
}

void MtCommand::SetCommandExitStatus(CHIP_ERROR status)
{
    mCommandExitStatus = status;
    if (CHIP_NO_ERROR != status)
    {
        ChipLogError(chipTool, "Run command failure: %s", chip::ErrorStr(status));
    }
    StopWaiting();
}

CHIP_ERROR MtCommand::StartWaiting(chip::System::Clock::Timeout duration)
{
    if (duration.count() > 0)
    {
        {
            std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
            mWaitingForResponse = true;
        }

        ReturnLogErrorOnFailure(chip::DeviceLayer::SystemLayer().StartTimer(duration, OnResponseTimeout, this));

        while (mWaitingForResponse)
        {
            chip::DeviceLayer::PlatformMgr().RunEventLoop();
        }
    }

    return mCommandExitStatus;
}

void MtCommand::StopWaiting()
{
    {
        if (cvWaitingForResponseMutex.try_lock())
        {
            mWaitingForResponse = false;
            cvWaitingForResponseMutex.unlock();
        }
        else
        {
            /* This function is called within the even loop. */
            mWaitingForResponse = false;
        }
    }
    LogErrorOnFailure(chip::DeviceLayer::PlatformMgr().StopEventLoopTask());
}

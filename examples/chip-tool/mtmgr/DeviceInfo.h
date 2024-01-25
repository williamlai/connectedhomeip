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

#include "MtCommand.h"

#include <app/ClusterStateCache.h>
#include <app/OperationalSessionSetup.h>
#include <lib/core/CHIPCallback.h>

class DeviceInfo : public MtCommand, public chip::app::ClusterStateCache::Callback
{
public:
    DeviceInfo(MatterManagerCore & mtmgrCore, NodeId nodeId) :
        MtCommand(mtmgrCore), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        mNodeId = nodeId;
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand();
    chip::System::Clock::Timeout GetWaitDuration() const { return chip::System::Clock::Seconds16(mTimeoutSecs.ValueOr(10)); }

    void setTimeout(uint16_t timeout) { mTimeoutSecs.SetValue(timeout); }

    void OnDone(chip::app::ReadClient *) override;

private:
    chip::NodeId mNodeId;
    chip::Optional<uint16_t> mTimeoutSecs;
    chip::Optional<bool> mExpireExistingSession;

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;

    chip::Platform::UniquePtr<chip::app::ClusterStateCache> mAttributeCache;
    chip::Platform::UniquePtr<chip::app::ReadClient> mReadClient;
};

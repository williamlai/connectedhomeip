/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include "MtCommand.h"
#include <app/OperationalSessionSetup.h>
#include <lib/core/CHIPCallback.h>

class MtWaitForCommissioneeCommand : public MtCommand
{
public:
    MtWaitForCommissioneeCommand(MatterManagerCore & mtmgrCore, NodeId nodeId) :
        MtCommand(mtmgrCore), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
    {
        mNodeId = nodeId;
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand();
    chip::System::Clock::Timeout GetWaitDuration() const { return chip::System::Clock::Seconds16(mTimeoutSecs.ValueOr(10)); }

    void setTimeout(uint16_t timeout) { mTimeoutSecs.SetValue(timeout); }

private:
    chip::NodeId mNodeId;
    chip::Optional<uint16_t> mTimeoutSecs;
    chip::Optional<bool> mExpireExistingSession;

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
};

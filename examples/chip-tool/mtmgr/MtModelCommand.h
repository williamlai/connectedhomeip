/*
 *   Copyright (c) 2020 Project CHIP Authors
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
#include <lib/core/CHIPEncoding.h>

class MtModelCommand : public MtCommand
{
public:
    MtModelCommand(MatterManagerCore & mtmgrCore, bool supportsMultipleEndpoints = false) :
        MtCommand(mtmgrCore), mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this), mSupportsMultipleEndpoints(supportsMultipleEndpoints)
    {}

#if 0
    void AddArguments(bool skipEndpoints = false)
    {
        AddArgument(
            "destination-id", 0, UINT64_MAX, &mDestinationId,
            "64-bit node or group identifier.\n  Group identifiers are detected by being in the 0xFFFF'FFFF'FFFF'xxxx range.");
        if (skipEndpoints == false)
        {
            if (mSupportsMultipleEndpoints)
            {
                AddArgument("endpoint-ids", 0, UINT16_MAX, &mEndPointId,
                            "Comma-separated list of endpoint ids (e.g. \"1\" or \"1,2,3\").\n  Allowed to be 0xFFFF to indicate a "
                            "wildcard endpoint.");
            }
            else
            {
                AddArgument("endpoint-id-ignored-for-group-commands", 0, UINT16_MAX, &mEndPointId,
                            "Endpoint the command is targeted at.");
            }
        }
        AddArgument("timeout", 0, UINT16_MAX, &mTimeout);
    }
#endif

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return chip::System::Clock::Seconds16(mTimeout.ValueOr(20));
    }

    virtual CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endPointIds) = 0;

    virtual CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex)
    {
        return CHIP_ERROR_BAD_REQUEST;
    };

    void Shutdown() override;

    void SetDestinationId(chip::NodeId nodeId)
    {
        mDestinationId = nodeId;
    }

    void SetEndPointId(std::vector<chip::EndpointId> & endPointId)
    {
        mEndPointId = endPointId;
    }

protected:
    chip::Optional<uint16_t> mTimeout;

private:
    chip::NodeId mDestinationId;
    std::vector<chip::EndpointId> mEndPointId;

    static void OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                    const chip::SessionHandle & sessionHandle);
    static void OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
    const bool mSupportsMultipleEndpoints;
};

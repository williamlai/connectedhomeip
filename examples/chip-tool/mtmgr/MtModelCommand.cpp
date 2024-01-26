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

#include "MtModelCommand.h"

#include <app/InteractionModelEngine.h>
#include <inttypes.h>

using namespace ::chip;

CHIP_ERROR MtModelCommand::RunCommand()
{

    if (IsGroupId(mDestinationId))
    {
        FabricIndex fabricIndex = mMtmgrCore.CurrentCommissioner().GetFabricIndex();
        ChipLogProgress(chipTool, "Sending command to group 0x%x", GroupIdFromNodeId(mDestinationId));

        return SendGroupCommand(GroupIdFromNodeId(mDestinationId), fabricIndex);
    }

    ChipLogProgress(chipTool, "Sending command to node 0x%" PRIx64, mDestinationId);

    CommissioneeDeviceProxy * commissioneeDeviceProxy = nullptr;
    if (CHIP_NO_ERROR == mMtmgrCore.CurrentCommissioner().GetDeviceBeingCommissioned(mDestinationId, &commissioneeDeviceProxy))
    {
        return SendCommand(commissioneeDeviceProxy, mEndPointId);
    }

    return mMtmgrCore.CurrentCommissioner().GetConnectedDevice(mDestinationId, &mOnDeviceConnectedCallback,
                                                               &mOnDeviceConnectionFailureCallback);
}

void MtModelCommand::OnDeviceConnectedFn(void * context, chip::Messaging::ExchangeManager & exchangeMgr,
                                         const chip::SessionHandle & sessionHandle)
{
    MtModelCommand * command = reinterpret_cast<MtModelCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectedFn: context is null"));

    chip::OperationalDeviceProxy device(&exchangeMgr, sessionHandle);
    CHIP_ERROR err = command->SendCommand(&device, command->mEndPointId);
    VerifyOrReturn(CHIP_NO_ERROR == err, command->SetCommandExitStatus(err));
}

void MtModelCommand::OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);

    MtModelCommand * command = reinterpret_cast<MtModelCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectionFailureFn: context is null"));
    command->SetCommandExitStatus(err);
}

void MtModelCommand::Shutdown()
{
    mOnDeviceConnectedCallback.Cancel();
    mOnDeviceConnectionFailureCallback.Cancel();

    MtCommand::Shutdown();
}

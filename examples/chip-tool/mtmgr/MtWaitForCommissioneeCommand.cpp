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

#include "MtWaitForCommissioneeCommand.h"

using namespace chip;

CHIP_ERROR MtWaitForCommissioneeCommand::RunCommand()
{
    chip::FabricIndex fabricIndex = mMtmgrCore.CurrentCommissioner().GetFabricIndex();
    ReturnErrorCodeIf(fabricIndex == chip::kUndefinedFabricIndex, CHIP_ERROR_INCORRECT_STATE);

    if (mExpireExistingSession.ValueOr(true))
    {
        mMtmgrCore.CurrentCommissioner().SessionMgr()->ExpireAllSessions(chip::ScopedNodeId(mNodeId, fabricIndex));
    }

    return mMtmgrCore.CurrentCommissioner().GetConnectedDevice(mNodeId, &mOnDeviceConnectedCallback,
                                                               &mOnDeviceConnectionFailureCallback);
}

void MtWaitForCommissioneeCommand::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                                       const SessionHandle & sessionHandle)
{
    auto * command = reinterpret_cast<MtWaitForCommissioneeCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectedFn: context is null"));
    command->SetCommandExitStatus(CHIP_NO_ERROR);
}

void MtWaitForCommissioneeCommand::OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);

    auto * command = reinterpret_cast<MtWaitForCommissioneeCommand *>(context);
    VerifyOrReturn(command != nullptr, ChipLogError(chipTool, "OnDeviceConnectionFailureFn: context is null"));
    command->SetCommandExitStatus(err);
}

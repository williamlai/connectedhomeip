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

#include "DeviceInfo.h"

using namespace chip;

CHIP_ERROR DeviceInfo::RunCommand()
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

void DeviceInfo::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    std::cout << "OnDeviceConnectedFn++" << std::endl;

    auto * deviceInfo = reinterpret_cast<DeviceInfo *>(context);
    VerifyOrReturn(deviceInfo != nullptr, ChipLogError(chipTool, "OnDeviceConnectedFn: context is null"));
    // command->SetCommandExitStatus(CHIP_NO_ERROR);

    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
    app::ReadPrepareParams readParams(sessionHandle);

    readParams.mIsFabricFiltered = false;

    // NOTE: this array cannot have more than 9 entries, since the spec mandates that server only needs to support 9
    app::AttributePathParams readPaths[9];
    size_t readPathsSize = 0;

    // Read all the feature maps for all the networking clusters on any endpoint to determine what is supported
    readPaths[readPathsSize++] = app::AttributePathParams(app::Clusters::NetworkCommissioning::Id,
                                                          app::Clusters::NetworkCommissioning::Attributes::FeatureMap::Id);

    readPaths[readPathsSize++] = app::AttributePathParams(kRootEndpointId, app::Clusters::BasicInformation::Id,
                                                          app::Clusters::BasicInformation::Attributes::VendorID::Id);
    readPaths[readPathsSize++] = app::AttributePathParams(kRootEndpointId, app::Clusters::BasicInformation::Id,
                                                          app::Clusters::BasicInformation::Attributes::ProductID::Id);

    readParams.mpAttributePathParamsList    = readPaths;
    readParams.mAttributePathParamsListSize = readPathsSize;

    auto attributeCache = Platform::MakeUnique<app::ClusterStateCache>(*deviceInfo);
    auto readClient     = chip::Platform::MakeUnique<app::ReadClient>(engine, &exchangeMgr, attributeCache->GetBufferedCallback(),
                                                                  app::ReadClient::InteractionType::Read);

    CHIP_ERROR err = readClient->SendRequest(readParams);
    if (err != CHIP_NO_ERROR)
    {
        LogErrorOnFailure(err);

        ChipLogError(Controller, "Failed to send read request for networking clusters");
        return;
    }

    deviceInfo->mAttributeCache = std::move(attributeCache);
    deviceInfo->mReadClient     = std::move(readClient);

    std::cout << "OnDeviceConnectedFn--" << std::endl;
}

void DeviceInfo::OnDeviceConnectionFailureFn(void * context, const chip::ScopedNodeId & peerId, CHIP_ERROR err)
{
    LogErrorOnFailure(err);

    auto * deviceInfo = reinterpret_cast<DeviceInfo *>(context);
    VerifyOrReturn(deviceInfo != nullptr, ChipLogError(chipTool, "OnDeviceConnectionFailureFn: context is null"));
    deviceInfo->SetCommandExitStatus(err);
}

// ClusterStateCache::Callback impl
void DeviceInfo::OnDone(app::ReadClient * readClient)
{
    auto * deviceInfo = reinterpret_cast<DeviceInfo *>(readClient);
    VerifyOrReturn(deviceInfo != nullptr, ChipLogError(chipTool, "OnDone: readClient is null"));

    CHIP_ERROR err;
    CHIP_ERROR return_err = CHIP_NO_ERROR;

    VendorId vendorId  = VendorId::Common;
    uint16_t productId = 0;

    {
        using namespace chip::app::Clusters::BasicInformation;
        using namespace chip::app::Clusters::BasicInformation::Attributes;

        err        = mAttributeCache->Get<VendorID::TypeInfo>(kRootEndpointId, vendorId);
        return_err = err == CHIP_NO_ERROR ? return_err : err;

        err        = mAttributeCache->Get<ProductID::TypeInfo>(kRootEndpointId, productId);
        return_err = err == CHIP_NO_ERROR ? return_err : err;
    }

    std::cout << "vendorId: " << static_cast<int>(vendorId) << ", productId: " << productId << std::endl;

    deviceInfo->SetCommandExitStatus(CHIP_NO_ERROR);
}

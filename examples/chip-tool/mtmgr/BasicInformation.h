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

#include "MtReportCommand.h"

class ReportBasicInformationVendorId : public MtReadAttribute
{
public:
    ReportBasicInformationVendorId(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, std::vector<chip::EndpointId> endPointId) :
        MtReadAttribute(chip::app::Clusters::BasicInformation::Id, chip::app::Clusters::BasicInformation::Attributes::VendorID::Id,
                        mtmgrCore)
    {
        SetDestinationId(nodeId);
        SetEndPointId(endPointId);
    }

    void OnAttributeDataAvailable(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data) override
    {
        if (path.mClusterId == chip::app::Clusters::BasicInformation::Id &&
            path.mAttributeId == chip::app::Clusters::BasicInformation::Attributes::VendorID::Id)
        {
            chip::app::DataModel::Decode(*data, mVendorId);
        }
    }

    chip::VendorId GetVendorId() const { return mVendorId; }

private:
    chip::VendorId mVendorId;
};

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

#include <unordered_map>
#include <vector>

#include "MtReportCommand.h"

class ReportDescriptorServerList : public MtReadAttribute
{
public:
    ReportDescriptorServerList(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId) :
        MtReadAttribute(chip::app::Clusters::Descriptor::Id, chip::app::Clusters::Descriptor::Attributes::ServerList::Id, mtmgrCore)
    {
        SetDestinationId(nodeId);

        std::vector<chip::EndpointId> endPointIdList = { endPointId };
        SetEndPointId(endPointIdList);
    }

    void OnAttributeDataAvailable(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data) override
    {
        if (path.mClusterId == chip::app::Clusters::Descriptor::Id &&
            path.mAttributeId == chip::app::Clusters::Descriptor::Attributes::ServerList::Id)
        {
            chip::EndpointId endpointId = path.mEndpointId;
            chip::app::DataModel::DecodableList<chip::ClusterId> value;

            CHIP_ERROR error = chip::app::DataModel::Decode(*data, value);
            if (CHIP_NO_ERROR != error)
            {
                ChipLogError(chipTool, "Response Failure: Can not decode Data");
                mError = error;
                return;
            }

            auto iter = value.begin();
            while (iter.Next())
            {
                mServerList[endpointId].push_back(iter.GetValue());
            }
        }
    }

    std::unordered_map<chip::EndpointId, std::vector<chip::ClusterId>> GetServerList() const { return mServerList; }

private:
    std::unordered_map<chip::EndpointId, std::vector<chip::ClusterId>> mServerList;
};

namespace ReportDescriptor {
static CHIP_ERROR GetServerList(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId,
                                std::unordered_map<chip::EndpointId, std::vector<chip::ClusterId>> & serverList)
{
    ReportDescriptorServerList reportDescriptor(mtmgrCore, nodeId, endPointId);
    ReturnErrorOnFailure(reportDescriptor.Run());
    serverList = reportDescriptor.GetServerList();
    return CHIP_NO_ERROR;
}
} // namespace ReportDescriptor
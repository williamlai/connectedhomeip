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

class ReportGenericAttributeList : public MtReadAttribute
{
public:
    ReportGenericAttributeList(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId,
                               chip::ClusterId clusterId) :
        MtReadAttribute(clusterId, chip::app::Clusters::Descriptor::Attributes::AttributeList::Id, mtmgrCore)
    {
        SetDestinationId(nodeId);

        std::vector<chip::EndpointId> endPointIdList = { endPointId };
        SetEndPointId(endPointIdList);
    }

    void OnAttributeDataAvailable(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data) override
    {
        if (path.mAttributeId == chip::app::Clusters::Descriptor::Attributes::AttributeList::Id)
        {
            chip::app::DataModel::DecodableList<chip::AttributeId> value;

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
                mAttributeList.push_back(iter.GetValue());
            }
        }
    }

    std::vector<chip::AttributeId> GetAttributeList() const { return mAttributeList; }

private:
    std::vector<chip::AttributeId> mAttributeList;
};

namespace ReportGeneric {
static CHIP_ERROR GetAttributeList(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId,
                                   chip::ClusterId clusterId, std::vector<chip::AttributeId> & attributeList)
{
    ReportGenericAttributeList reportGeneric(mtmgrCore, nodeId, endPointId, clusterId);
    ReturnErrorOnFailure(reportGeneric.Run());
    attributeList = reportGeneric.GetAttributeList();
    return CHIP_NO_ERROR;
}
} // namespace ReportGeneric

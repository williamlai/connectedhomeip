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

class ReportOnOffOnOff : public MtReadAttribute
{
public:
    ReportOnOffOnOff(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId) :
        MtReadAttribute(chip::app::Clusters::OnOff::Id, chip::app::Clusters::OnOff::Attributes::OnOff::Id, mtmgrCore)
    {
        SetDestinationId(nodeId);

        std::vector<chip::EndpointId> endPointIdList = { endPointId };
        SetEndPointId(endPointIdList);
    }

    void OnAttributeDataAvailable(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data) override
    {
        if (path.mClusterId == chip::app::Clusters::OnOff::Id &&
            path.mAttributeId == chip::app::Clusters::OnOff::Attributes::OnOff::Id)
        {
            bool value;

            CHIP_ERROR error = chip::app::DataModel::Decode(*data, value);
            if (CHIP_NO_ERROR != error)
            {
                ChipLogError(chipTool, "Response Failure: Can not decode Data");
                mError = error;
                return;
            }

            mOnOff = value;
        }
    }

    uint16_t GetOnOff() const { return mOnOff; }

private:
    bool mOnOff;
};

namespace ReportOnOff {
static CHIP_ERROR GetOnOff(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId, bool & onOff)
{
    ReportOnOffOnOff reportOnOff(mtmgrCore, nodeId, endPointId);
    ReturnErrorOnFailure(reportOnOff.Run());
    onOff = reportOnOff.GetOnOff();
    return CHIP_NO_ERROR;
}
} // namespace ReportOnOff

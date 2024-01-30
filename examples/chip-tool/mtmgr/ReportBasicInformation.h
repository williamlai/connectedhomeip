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

class ReportBasicInformationDataModelRevision : public MtReadAttribute
{
public:
    ReportBasicInformationDataModelRevision(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId) :
        MtReadAttribute(chip::app::Clusters::BasicInformation::Id,
                        chip::app::Clusters::BasicInformation::Attributes::DataModelRevision::Id, mtmgrCore)
    {
        SetDestinationId(nodeId);

        std::vector<chip::EndpointId> endPointIdList = { endPointId };
        SetEndPointId(endPointIdList);
    }

    void OnAttributeDataAvailable(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data) override
    {
        if (path.mClusterId == chip::app::Clusters::BasicInformation::Id &&
            path.mAttributeId == chip::app::Clusters::BasicInformation::Attributes::DataModelRevision::Id)
        {
            uint16_t value;

            CHIP_ERROR error = chip::app::DataModel::Decode(*data, value);
            if (CHIP_NO_ERROR != error)
            {
                ChipLogError(chipTool, "Response Failure: Can not decode Data");
                mError = error;
                return;
            }

            mDataModelRevision = value;
        }
    }

    uint16_t GetDataModelRevision() const { return mDataModelRevision; }

private:
    uint16_t mDataModelRevision;
};

class ReportBasicInformationVendorId : public MtReadAttribute
{
public:
    ReportBasicInformationVendorId(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId) :
        MtReadAttribute(chip::app::Clusters::BasicInformation::Id, chip::app::Clusters::BasicInformation::Attributes::VendorID::Id,
                        mtmgrCore)
    {
        SetDestinationId(nodeId);

        std::vector<chip::EndpointId> endPointIdList = { endPointId };
        SetEndPointId(endPointIdList);
    }

    void OnAttributeDataAvailable(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data) override
    {
        if (path.mClusterId == chip::app::Clusters::BasicInformation::Id &&
            path.mAttributeId == chip::app::Clusters::BasicInformation::Attributes::VendorID::Id)
        {
            chip::VendorId value;

            CHIP_ERROR error = chip::app::DataModel::Decode(*data, mVendorId);
            if (CHIP_NO_ERROR != error)
            {
                ChipLogError(chipTool, "Response Failure: Can not decode Data");
                mError = error;
                return;
            }

            mVendorId = value;
        }
    }

    chip::VendorId GetVendorId() const { return mVendorId; }

private:
    chip::VendorId mVendorId;
};

class ReportBasicInformationProductID : public MtReadAttribute
{
public:
    ReportBasicInformationProductID(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId) :
        MtReadAttribute(chip::app::Clusters::BasicInformation::Id, chip::app::Clusters::BasicInformation::Attributes::ProductID::Id,
                        mtmgrCore)
    {
        SetDestinationId(nodeId);

        std::vector<chip::EndpointId> endPointIdList = { endPointId };
        SetEndPointId(endPointIdList);
    }

    void OnAttributeDataAvailable(const chip::app::ConcreteDataAttributePath & path, chip::TLV::TLVReader * data) override
    {
        if (path.mClusterId == chip::app::Clusters::BasicInformation::Id &&
            path.mAttributeId == chip::app::Clusters::BasicInformation::Attributes::ProductID::Id)
        {
            uint16_t value;

            CHIP_ERROR error = chip::app::DataModel::Decode(*data, mProductID);
            if (CHIP_NO_ERROR != error)
            {
                ChipLogError(chipTool, "Response Failure: Can not decode Data");
                mError = error;
                return;
            }

            mProductID = value;
        }
    }

    uint16_t GetProductID() const { return mProductID; }

private:
    uint16_t mProductID;
};

namespace ReportBasicInformation {
static CHIP_ERROR GetDataModelRevision(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId,
                                       uint16_t & dataModelRevision)
{
    ReportBasicInformationDataModelRevision reportBasicInfo(mtmgrCore, nodeId, endPointId);
    ReturnErrorOnFailure(reportBasicInfo.Run());
    dataModelRevision = reportBasicInfo.GetDataModelRevision();

    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetVendorId(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId,
                              chip::VendorId & vendorId)
{
    ReportBasicInformationVendorId reportBasicInfo(mtmgrCore, nodeId, endPointId);
    ReturnErrorOnFailure(reportBasicInfo.Run());
    vendorId = reportBasicInfo.GetVendorId();
    return CHIP_NO_ERROR;
}

static CHIP_ERROR GetProductID(MatterManagerCore & mtmgrCore, chip::NodeId nodeId, chip::EndpointId endPointId,
                               uint16_t & productId)
{
    ReportBasicInformationProductID reportBasicInfo(mtmgrCore, nodeId, endPointId);
    ReturnErrorOnFailure(reportBasicInfo.Run());
    productId = reportBasicInfo.GetProductID();
    return CHIP_NO_ERROR;
}

} // namespace ReportBasicInformation

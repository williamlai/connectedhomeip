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

#include "MatterManager.h"

#include <vector>

#include "MatterManagerCore.h"
#include "MtDiscoverCommissionablesCommand.h"
#include "MtPairingCommand.h"
#include "MtReportCommand.h"
#include "MtWaitForCommissioneeCommand.h"
#include "NodeIdStorage.h"
#include "ReportBasicInformation.h"
#include "ReportDescriptor.h"

#include <lib/core/CASEAuthTag.h>

#include <commands/common/CredentialIssuerCommands.h>
#include <commands/example/ExampleCredentialIssuerCommands.h>

#include <lib/dnssd/Discovery_ImplPlatform.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C" {
#endif

ExampleCredentialIssuerCommands credIssuerCommands;
MatterManagerCore sMtmgrCore(&credIssuerCommands);

void matterMgr_test(void)
{
    [[maybe_unused]] CHIP_ERROR ret;
    printf("%s(): ++\n", __FUNCTION__);

#if 1
    MtPairOnNetworkLong pairOnNetworkLong(sMtmgrCore, 41, 12345678LLU, 1001);
    ret = pairOnNetworkLong.Run();
    printf("%s(): pairOnNetworkLong ret:%d\n", __FUNCTION__, ret.AsInteger());
#endif

#if 0
    {
        using namespace chip::app::Clusters::BasicInformation;

        MtReadAttribute readAttribute(Id, Attributes::AttributeList::Id, sMtmgrCore);
        chip::NodeId nodeId = 41;
        readAttribute.SetDestinationId(nodeId);
        std::vector<chip::EndpointId> endPointId = { 0 };
        readAttribute.SetEndPointId(endPointId);
        ret = readAttribute.Run();
        printf("%s(): readAttribute ret:%d\n", __FUNCTION__, ret.AsInteger());
    }
#endif

    printf("%s(): --\n", __FUNCTION__);
}

mt_status_t matterMgr_init(void)
{
    sMtmgrCore.SetUpStack();
    return MT_STATUS_OK;
}

mt_status_t matterMgr_isReady(void)
{
    return MT_STATUS_OK;
}

mt_status_t matterMgr_deInit(void)
{
    sMtmgrCore.TearDownStack();
    return MT_STATUS_OK;
}

mt_status_t matterMgr_pairOnNetworkLong(matter_nodeId_t node_id, uint32_t pinCode, uint16_t discoveryFilterCode)
{
    mt_status_t res = MT_STATUS_OK;
    CHIP_ERROR ret;

    MtPairOnNetworkLong pairOnNetworkLong(sMtmgrCore, node_id, pinCode, discoveryFilterCode);
    ret = pairOnNetworkLong.Run();
    if (ret != CHIP_NO_ERROR)
    {
        res = MT_STATUS_GENERAL_ERROR;
    }

    return res;
}

mt_status_t matterMgr_getNetworkInfo(matter_net_t * ret_net_info)
{
    mt_status_t res = MT_STATUS_OK;

    if (ret_net_info == nullptr)
    {
        res = MT_STATUS_BAD_PARAM;
    }
    else
    {
        memset(ret_net_info, 0, sizeof(matter_net_t));

        std::vector<chip::ScopedNodeId> nodeIdList = sMtmgrCore.GetNodeIdStorage().GetScopedNodeIdList();

        ret_net_info->node_cnt = nodeIdList.size();
    }

    return res;
}

mt_status_t matterMgr_getNodeIdList(matter_nodeId_t * ret_node_id_arry, size_t * node_cnt)
{
    mt_status_t res = MT_STATUS_OK;

    if (ret_node_id_arry == nullptr || node_cnt == nullptr)
    {
        res = MT_STATUS_BAD_PARAM;
    }
    else
    {
        std::vector<chip::ScopedNodeId> nodeIdList = sMtmgrCore.GetNodeIdStorage().GetScopedNodeIdList();

        if (nodeIdList.size() > *node_cnt)
        {
            res = MT_STATUS_GENERAL_ERROR;
        }
        else
        {
            for (size_t i = 0; i < nodeIdList.size(); i++)
            {
                ret_node_id_arry[i] = static_cast<matter_nodeId_t>(nodeIdList[i].GetNodeId());
            }
            *node_cnt = nodeIdList.size();
        }
    }

    return res;
}

static CHIP_ERROR getDetailedNodeInfo(chip::NodeId nodeId, matter_node_t * ret_node)
{
    chip::EndpointId endPointIdWildcard = 0xFFFF;
    uint16_t dataModelRevision;
    chip::VendorId vendorId;
    uint16_t productID;
    std::unordered_map<chip::EndpointId, std::vector<chip::ClusterId>> serverList;

    if (ReportBasicInformation::GetDataModelRevision(sMtmgrCore, nodeId, endPointIdWildcard, dataModelRevision) == CHIP_NO_ERROR)
    {
        ret_node->data_model_revision = dataModelRevision;
    }

    if (ReportBasicInformation::GetVendorId(sMtmgrCore, nodeId, endPointIdWildcard, vendorId) == CHIP_NO_ERROR)
    {
        ret_node->vendor_id = static_cast<uint16_t>(vendorId);
    }

    if (ReportBasicInformation::GetProductID(sMtmgrCore, nodeId, endPointIdWildcard, productID) == CHIP_NO_ERROR)
    {
        ret_node->product_id = productID;
    }

    if (ReportDescriptor::GetServerList(sMtmgrCore, nodeId, endPointIdWildcard, serverList) == CHIP_NO_ERROR)
    {
        ret_node->endpoint_cnt = static_cast<uint16_t>(serverList.size());
    }

    return CHIP_NO_ERROR;
}

mt_status_t matterMgr_getDetailedNodeInfo(matter_nodeId_t node_id, matter_node_t * ret_node)
{
    mt_status_t res = MT_STATUS_OK;

    if (ret_node == NULL)
    {
        res = MT_STATUS_BAD_PARAM;
    }
    else
    {
        chip::NodeId nodeId = static_cast<chip::NodeId>(node_id);

        memset(ret_node, 0, sizeof(matter_node_t));

        if (getDetailedNodeInfo(nodeId, ret_node) != CHIP_NO_ERROR)
        {
            res = MT_STATUS_GENERAL_ERROR;
        }
        else
        {
            /* nop */
        }
    }

    return res;
}

mt_status_t matterMgr_getEpIdList(matter_nodeId_t node_id, matter_epId_t * ret_ep_id_arry, size_t * ep_cnt)
{
    mt_status_t res = MT_STATUS_OK;

    if (ret_ep_id_arry == NULL || ep_cnt == NULL)
    {
        res = MT_STATUS_BAD_PARAM;
    }
    else
    {
        chip::NodeId nodeId                 = static_cast<chip::NodeId>(node_id);
        chip::EndpointId endPointIdWildcard = 0xFFFF;
        std::unordered_map<chip::EndpointId, std::vector<chip::ClusterId>> serverList;

        if (ReportDescriptor::GetServerList(sMtmgrCore, nodeId, endPointIdWildcard, serverList) != CHIP_NO_ERROR)
        {
            res = MT_STATUS_GENERAL_ERROR;
        }
        else
        {
            if (*ep_cnt < serverList.size())
            {
                res = MT_STATUS_GENERAL_ERROR;
            }
            else
            {
                size_t i = 0;
                for (auto it : serverList)
                {
                    ret_ep_id_arry[i++] = static_cast<matter_epId_t>(it.first);
                }
                *ep_cnt = serverList.size();
            }
        }
    }

    return res;
}

mt_status_t matterMgr_getDetailedEPInfo(matter_nodeId_t node_id, matter_epId_t ep_id, matter_endpoint_t * ret_ep)
{
    mt_status_t res = MT_STATUS_OK;

    if (ret_ep == NULL)
    {
        res = MT_STATUS_BAD_PARAM;
    }
    else
    {
        chip::NodeId nodeId         = static_cast<chip::NodeId>(node_id);
        chip::EndpointId endPointId = static_cast<chip::EndpointId>(ep_id);
        std::unordered_map<chip::EndpointId, std::vector<chip::ClusterId>> serverList;

        memset(ret_ep, 0, sizeof(matter_endpoint_t));

        if (ReportDescriptor::GetServerList(sMtmgrCore, nodeId, endPointId, serverList) != CHIP_NO_ERROR)
        {
            res = MT_STATUS_GENERAL_ERROR;
        }
        else if (serverList.find(endPointId) == serverList.end())
        {
            res = MT_STATUS_GENERAL_ERROR;
        }
        else
        {
            ret_ep->node_id     = node_id;
            ret_ep->ep_id       = ep_id;
            ret_ep->cluster_cnt = serverList[endPointId].size();
        }
    }

    return res;
}

mt_status_t matterMgr_getClusterList(matter_nodeId_t node_id, matter_epId_t ep_id, matter_clusterId_t * ret_cluster_array,
                                     size_t * cluster_cnt)
{
    mt_status_t res = MT_STATUS_OK;

    if (ret_cluster_array == NULL || cluster_cnt == NULL)
    {
        res = MT_STATUS_BAD_PARAM;
    }
    else
    {
        chip::NodeId nodeId         = static_cast<chip::NodeId>(node_id);
        chip::EndpointId endPointId = static_cast<chip::EndpointId>(ep_id);
        std::unordered_map<chip::EndpointId, std::vector<chip::ClusterId>> serverList;

        if (ReportDescriptor::GetServerList(sMtmgrCore, nodeId, endPointId, serverList) != CHIP_NO_ERROR)
        {
            res = MT_STATUS_GENERAL_ERROR;
        }
        else if (serverList.find(endPointId) == serverList.end())
        {
            res = MT_STATUS_GENERAL_ERROR;
        }
        else
        {
            auto v = serverList[endPointId];

            if (*cluster_cnt < v.size())
            {
                res = MT_STATUS_GENERAL_ERROR;
            }
            else
            {
                for (size_t i = 0; i < v.size(); i++)
                {
                    ret_cluster_array[i] = static_cast<matter_clusterId_t>(v[i]);
                }
                *cluster_cnt = v.size();
            }
        }
    }

    return res;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

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
#include "MtWaitForCommissioneeCommand.h"
#include "NodeIdStorage.h"

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
    CHIP_ERROR ret;
    printf("%s(): ++\n", __FUNCTION__);

    MtPairOnNetworkLong pairOnNetworkLong(sMtmgrCore, 41, 12345678LLU, 1001);
    ret = pairOnNetworkLong.Run();
    printf("%s(): pairOnNetworkLong ret:%d\n", __FUNCTION__, ret.AsInteger());

    MtWaitForCommissioneeCommand waitForCommissioneeCommand(sMtmgrCore, 41);
    ret = waitForCommissioneeCommand.Run();
    printf("%s(): waitForCommissioneeCommand ret:%d\n", __FUNCTION__, ret.AsInteger());

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
            res = MT_STATUS_OUT_OF_RESOURCES;
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

static CHIP_ERROR getDetailedNodeInfo(chip::NodeId nodeId)
{
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

        getDetailedNodeInfo(nodeId);
    }

    return res;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

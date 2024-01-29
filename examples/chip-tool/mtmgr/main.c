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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "MatterManager.h"

#define MAX_NODE_ID_ARRAY_SIZE 10

static void printNodeInfo(matter_nodeId_t node_id)
{
    matter_node_t node;
    if (matterMgr_getDetailedNodeInfo(node_id, &node) == MT_STATUS_OK)
    {
        printf("Node ID:%" PRIu64 "\n", node_id);
        printf("    Data Model Revision: %d\n", node.data_model_revision);
        printf("    Vendor ID:           %d\n", node.vendor_id);
        printf("    Product ID::         %d\n", node.product_id);
    }
}

static void printMatterNetworkInfo()
{
    matter_net_t mtnet;
    size_t node_cnt = MAX_NODE_ID_ARRAY_SIZE;
    matter_nodeId_t node_id_array[MAX_NODE_ID_ARRAY_SIZE];

    if (matterMgr_getNetworkInfo(&mtnet) == MT_STATUS_OK)
    {
        printf("Device node count: %zu\n", mtnet.node_cnt);

        if (matterMgr_getNodeIdList(node_id_array, &node_cnt) == MT_STATUS_OK)
        {
            for (size_t i = 0; i < node_cnt; i++)
            {
                printNodeInfo(node_id_array[i]);
            }
        }
    }
}

int main(int argc, char * argv[])
{
    if (matterMgr_init() != MT_STATUS_OK)
    {
        printf("Failed to initialize Matter Manager.\n");
    }
    else
    {
        int i                   = 0;
        const int maxRetryCount = 10;
        while (matterMgr_isReady() != MT_STATUS_OK && i < maxRetryCount)
        {
            sleep(1);
            i++;
        }

        if (i == maxRetryCount)
        {
            printf("Timeout for waiting for the Matter Manager to be ready.\n");
        }
        else
        {
            // matterMgr_test();

            printMatterNetworkInfo();
        }
    }

    matterMgr_deInit();

    return 0;
}

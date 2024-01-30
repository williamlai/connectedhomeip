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
#define MAX_ENDPOINT_ID_ARRAY_SIZE 10
#define MAX_CLUSTER_ID_ARRAY_SIZE 128

static void printEndpointInfo(matter_nodeId_t node_id, matter_epId_t endpoint_id)
{
    matter_endpoint_t endpoint;
    size_t cluster_cnt = MAX_CLUSTER_ID_ARRAY_SIZE;
    matter_clusterId_t cluster_id_array[MAX_CLUSTER_ID_ARRAY_SIZE];

    if (matterMgr_getDetailedEPInfo(node_id, endpoint_id, &endpoint) == MT_STATUS_OK)
    {
        printf("Node ID: %" PRIu64 ", Endpoint ID: %d\n", node_id, endpoint_id);
        printf("    Cluster Count: %zu\n", endpoint.cluster_cnt);

        if (matterMgr_getClusterList(node_id, endpoint_id, cluster_id_array, &cluster_cnt) == MT_STATUS_OK)
        {
            printf("    Cluster List:");
            for (size_t i = 0; i < cluster_cnt; i++)
            {
                printf(" %d", cluster_id_array[i]);
            }
            printf("\n");
        }
    }
}

static void printNodeInfo(matter_nodeId_t node_id)
{
    matter_node_t node;
    size_t endpoint_cnt = MAX_ENDPOINT_ID_ARRAY_SIZE;
    matter_epId_t endpoint_id_array[MAX_ENDPOINT_ID_ARRAY_SIZE];

    if (matterMgr_getDetailedNodeInfo(node_id, &node) == MT_STATUS_OK)
    {
        printf("Node ID: %" PRIu64 "\n", node_id);
        printf("    Data Model Revision: %d\n", node.data_model_revision);
        printf("    Vendor ID:           %d\n", node.vendor_id);
        printf("    Product ID:          %d\n", node.product_id);
        printf("    Endpoint Count:      %d\n", node.endpoint_cnt);

        if (matterMgr_getEpIdList(node_id, endpoint_id_array, &endpoint_cnt) == MT_STATUS_OK)
        {
            printf("    Endpoint List:");
            for (size_t i = 0; i < endpoint_cnt; i++)
            {
                printf(" %d", endpoint_id_array[i]);
            }
            printf("\n");

            for (size_t i = 0; i < endpoint_cnt; i++)
            {
                printEndpointInfo(node_id, endpoint_id_array[i]);
            }
        }

        printf("\n");
    }
}

[[maybe_unused]] static void printMatterNetworkInfo()
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

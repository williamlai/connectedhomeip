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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "MatterManager.h"

#define MAX_NODE_ID_ARRAY_SIZE 10
#define MAX_ENDPOINT_ID_ARRAY_SIZE 10
#define MAX_CLUSTER_ID_ARRAY_SIZE 128
#define MAX_ATTRIBUTE_ID_ARRAY_SIZE 1024

#define SETUP_DEMO_DEVICE (1)
#if SETUP_DEMO_DEVICE
#define DEVICE_NODE_ID (41)
#define DEVICE_PIN_CODE (12345678)
#define DEVICE_DISCOVERY_FILTER_CODE (1001)
#endif

static void printClusterOnOff(matter_nodeId_t node_id, matter_epId_t endpoint_id)
{
    bool onOff = false;

    if (matterMgr_getOnOffOnOff(node_id, endpoint_id, &onOff) == MT_STATUS_OK)
    {
        printf("Node ID: %" PRIu64 ", Endpoint ID: %d, Cluster OnOff\n", node_id, endpoint_id);
        printf("    OnOff: %s\n", (onOff) ? "TRUE" : "FALSE");
    }
}

static void runToggleTest(matter_nodeId_t node_id, matter_epId_t endpoint_id)
{
    printClusterOnOff(node_id, endpoint_id);

    printf("Sending OnOff Toggle command to Node ID: %" PRIu64 ", EndPoint ID: %d...\n", node_id, endpoint_id);
    if (matterMgr_sendOnOffToggle(node_id, endpoint_id) != MT_STATUS_OK)
    {
        printf("Failed to send OnOff Toggle command!\n");
    }
    else
    {
        printf("Successfully sent OnOff Toggle command\n");
    }

    printClusterOnOff(node_id, endpoint_id);
}

static void printClusterInfo(matter_nodeId_t node_id, matter_epId_t endpoint_id, matter_clusterId_t cluster_id)
{
    matter_cluster_t cluster;
    size_t attribute_cnt = MAX_ATTRIBUTE_ID_ARRAY_SIZE;
    matter_attributeId_t attribute_id_array[MAX_ATTRIBUTE_ID_ARRAY_SIZE];

    if (matterMgr_getDetailedClusterInfo(node_id, endpoint_id, cluster_id, &cluster) == MT_STATUS_OK)
    {
        printf("Node ID: %" PRIu64 ", Endpoint ID: %d, Cluster ID: %s(%d)\n", node_id, endpoint_id,
               matterMgr_getClusterName(cluster_id), cluster_id);
        printf("    Attribute Count: %zu\n", cluster.attribute_cnt);

        if (matterMgr_getAttributeList(node_id, endpoint_id, cluster_id, attribute_id_array, &attribute_cnt) == MT_STATUS_OK)
        {
            printf("    Attribute List:");
            for (size_t i = 0; i < attribute_cnt; i++)
            {
                printf(" %d", attribute_id_array[i]);
            }
            printf("\n");
        }
    }
}

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
            printf("    Cluster List:\n");
            for (size_t i = 0; i < cluster_cnt; i++)
            {
                printf("        %s(%d)\n", matterMgr_getClusterName(cluster_id_array[i]), cluster_id_array[i]);
            }

            for (size_t i = 0; i < cluster_cnt; i++)
            {
                printClusterInfo(node_id, endpoint_id, cluster_id_array[i]);
            }
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

static void printMatterNetworkInfo()
{
    matter_net_t mtnet;
    size_t node_cnt = MAX_NODE_ID_ARRAY_SIZE;
    matter_nodeId_t node_id_array[MAX_NODE_ID_ARRAY_SIZE];

    if (matterMgr_getNetworkInfo(&mtnet) == MT_STATUS_OK)
    {
        printf("Device node count: %zu\n", mtnet.node_cnt);

#if SETUP_DEMO_DEVICE
        if (mtnet.node_cnt == 0)
        {
            printf("Try to pair device...\n");
            matter_nodeId_t node_id      = DEVICE_NODE_ID;
            uint32_t pinCode             = DEVICE_PIN_CODE;
            uint16_t discoveryFilterCode = DEVICE_DISCOVERY_FILTER_CODE;
            if (matterMgr_pairOnNetworkLong(node_id, pinCode, discoveryFilterCode) != MT_STATUS_OK)
            {
                printf("Failed to pair device!\n");
            }
        }
#endif

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
            printMatterNetworkInfo();

#if SETUP_DEMO_DEVICE
            matter_nodeId_t node_id   = DEVICE_NODE_ID;
            matter_epId_t endpoint_id = 1;
            runToggleTest(node_id, endpoint_id);
#endif
        }
    }

    matterMgr_deInit();

    return 0;
}

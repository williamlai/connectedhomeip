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

#ifndef _MATTER_MANAGER_H_
#define _MATTER_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "matter_mgr_status.h"

void matterMgr_test(void);

mt_status_t matterMgr_init(void);

mt_status_t matterMgr_isReady(void);

mt_status_t matterMgr_deInit(void);

typedef uint64_t matter_nodeId_t;

mt_status_t matterMgr_pairOnNetworkLong(matter_nodeId_t node_id, uint32_t pinCode, uint16_t discoveryFilterCode);

typedef struct
{
    /** More Matter network info. Ex. controller node id, fabric count, home fabric index, ... */

    size_t node_cnt;
} matter_net_t;

mt_status_t matterMgr_getNetworkInfo(matter_net_t * ret_net_info);

mt_status_t matterMgr_getNodeIdList(matter_nodeId_t * ret_node_id_arry, size_t * node_cnt);

typedef struct
{
    matter_nodeId_t id;
    uint16_t data_model_revision;
    uint16_t vendor_id;
    uint16_t product_id;

    uint16_t endpoint_cnt;

    /** More Matter node info... */
} matter_node_t;

mt_status_t matterMgr_getDetailedNodeInfo(matter_nodeId_t node_id, matter_node_t * ret_node);

typedef uint16_t matter_epId_t;

mt_status_t matterMgr_getEpIdList(matter_nodeId_t node_id, matter_epId_t * ret_ep_id_arry, size_t * ep_cnt);

typedef struct
{
    matter_nodeId_t node_id;
    matter_epId_t ep_id;

    size_t cluster_cnt;
} matter_endpoint_t;

mt_status_t matterMgr_getDetailedEPInfo(matter_nodeId_t node_id, matter_epId_t ep_id, matter_endpoint_t * ret_ep);

typedef uint32_t matter_clusterId_t;

mt_status_t matterMgr_getClusterList(matter_nodeId_t node_id, matter_epId_t ep_id, matter_clusterId_t * ret_cluster_array,
                                     size_t * cluster_cnt);

const char * matterMgr_getClusterName(matter_clusterId_t cluster_id);

typedef struct
{
    matter_nodeId_t node_id;
    matter_epId_t ep_id;
    matter_clusterId_t cluster_id;

    size_t attribute_cnt;
} matter_cluster_t;

typedef uint32_t matter_attributeId_t;

mt_status_t matterMgr_getDetailedClusterInfo(matter_nodeId_t node_id, matter_epId_t ep_id, matter_clusterId_t cluster_id,
                                             matter_cluster_t * ret_cluster);

mt_status_t matterMgr_getAttributeList(matter_nodeId_t node_id, matter_epId_t ep_id, matter_clusterId_t cluster_id,
                                       matter_attributeId_t * ret_attribute_array, size_t * attribute_cnt);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _MATTER_MANAGER_H_ */
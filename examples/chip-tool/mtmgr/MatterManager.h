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
    uint16_t vendor_id;
    uint16_t vendor_type;
    uint16_t product_id;
    uint16_t protocol_ver;

    /** More Matter node info... */
} matter_node_t;

mt_status_t matterMgr_getDetailedNodeInfo(matter_nodeId_t node_id, matter_node_t * ret_node);

void mtmgr_test();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _MATTER_MANAGER_H_ */
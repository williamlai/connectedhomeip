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

#include <stdio.h>
#include <unistd.h>

#include "MatterManager.h"

static void getMatterNetworkInfo()
{
    matter_net_t mtnet;

    if (matterMgr_getNetworkInfo(&mtnet) == MT_STATUS_OK)
    {
        printf("Device node count: %zu\n", mtnet.node_cnt);
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
            getMatterNetworkInfo();

            matterMgr_test();
        }
    }

    matterMgr_deInit();

    return 0;
}

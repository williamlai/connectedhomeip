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

#include <unordered_set>

#include <app/util/basic-types.h>
#include <controller/CHIPDeviceController.h>
#include <inipp/inipp.h>
#include <lib/support/logging/CHIPLogging.h>

#include <lib/core/ScopedNodeId.h>

class NodeIdStorage : public chip::PersistentStorageDelegate
{
public:
    /**
     * name is the name of the storage to use.  If null, defaults to
     * "chip_tool_config.ini".
     *
     * directory is the directory the storage file should be placed in.  If
     * null, falls back to getenv("TMPDIR") and if that is not set falls back
     * to /tmp.
     *
     * If non-null values are provided, the memory they point to is expected to
     * outlive this object.
     */
    CHIP_ERROR Init(const char * name = nullptr, const char * directory = nullptr);

    /////////// PersistentStorageDelegate Interface /////////
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;
    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;
    bool SyncDoesKeyExist(const char * key) override;

    void DumpKeys() const;

    CHIP_ERROR GetScopedNodeId(chip::ScopedNodeId & scopedNodeId, void * buffer, uint16_t & size);
    CHIP_ERROR SetScopedNodeId(chip::ScopedNodeId & scopedNodeId, const void * value, uint16_t size);
    CHIP_ERROR DeleteScopedNodeId(chip::ScopedNodeId & scopedNodeId);
    bool DoesScopedNodeIdExist(chip::ScopedNodeId & scopedNodeId);

    std::vector<chip::ScopedNodeId> GetScopedNodeIdList();

    // Clear all of the persistent storage for running session.
    CHIP_ERROR SyncClearAll();

    // Get the directory actually being used for the storage.
    const char * GetDirectory() const;

private:
    CHIP_ERROR CommitConfig(const char * directory, const char * name);
    inipp::Ini<char> mConfig;
    const char * mName;
    const char * mDirectory;
};

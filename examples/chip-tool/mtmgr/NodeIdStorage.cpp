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

#include "NodeIdStorage.h"

#include <lib/support/Base64.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/IniEscaping.h>

#include <fstream>
#include <memory>

using String   = std::basic_string<char>;
using Section  = std::map<String, String>;
using Sections = std::map<String, Section>;

using namespace ::chip;
using namespace ::chip::Controller;
using namespace ::chip::IniEscaping;
using namespace ::chip::Logging;

constexpr char kDefaultSectionName[]       = "Default";
constexpr char kPortKey[]                  = "ListenPort";
constexpr char kLoggingKey[]               = "LoggingLevel";
constexpr char kLocalNodeIdKey[]           = "LocalNodeId";
constexpr char kCommissionerCATsKey[]      = "CommissionerCATs";
constexpr LogCategory kDefaultLoggingLevel = kLogCategory_Automation;

static const char * GetUsedDirectory(const char * directory)
{
    const char * dir = directory;

    if (dir == nullptr)
    {
        dir = getenv("TMPDIR");
    }

    if (dir == nullptr)
    {
        dir = "/tmp";
    }

    return dir;
}

static std::string GetFilename(const char * directory, const char * name)
{
    const char * dir = GetUsedDirectory(directory);

    if (name == nullptr)
    {
        return std::string(dir) + "/chip_tool_config.ini";
    }

    return std::string(dir) + "/chip_tool_config." + std::string(name) + ".ini";
}

CHIP_ERROR NodeIdStorage::Init(const char * name, const char * directory)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::ifstream ifs;
    ifs.open(GetFilename(directory, name), std::ifstream::in);
    if (!ifs.good())
    {
        CommitConfig(directory, name);
        ifs.open(GetFilename(directory, name), std::ifstream::in);
    }
    VerifyOrExit(ifs.is_open(), err = CHIP_ERROR_OPEN_FAILED);

    mName      = name;
    mDirectory = directory;
    mConfig.parse(ifs);
    ifs.close();

    // To audit the contents at init, uncomment the following:
#if 0
    DumpKeys();
#endif

exit:
    return err;
}

CHIP_ERROR NodeIdStorage::SyncGetKeyValue(const char * key, void * value, uint16_t & size)
{
    std::string iniValue;

    ReturnErrorCodeIf(((value == nullptr) && (size != 0)), CHIP_ERROR_INVALID_ARGUMENT);

    auto section = mConfig.sections[kDefaultSectionName];

    ReturnErrorCodeIf(!SyncDoesKeyExist(key), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    std::string escapedKey = EscapeKey(key);
    ReturnErrorCodeIf(!inipp::extract(section[escapedKey], iniValue), CHIP_ERROR_INVALID_ARGUMENT);

    iniValue = Base64ToString(iniValue);

    uint16_t dataSize = static_cast<uint16_t>(iniValue.size());
    ReturnErrorCodeIf(size == 0 && dataSize == 0, CHIP_NO_ERROR);
    ReturnErrorCodeIf(value == nullptr, CHIP_ERROR_BUFFER_TOO_SMALL);

    uint16_t sizeToCopy = std::min(size, dataSize);

    memcpy(value, iniValue.data(), sizeToCopy);
    size = sizeToCopy;
    return size < dataSize ? CHIP_ERROR_BUFFER_TOO_SMALL : CHIP_NO_ERROR;
}

CHIP_ERROR NodeIdStorage::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    ReturnErrorCodeIf((value == nullptr) && (size != 0), CHIP_ERROR_INVALID_ARGUMENT);

    auto section = mConfig.sections[kDefaultSectionName];

    std::string escapedKey = EscapeKey(key);
    if (value == nullptr)
    {
        section[escapedKey] = "";
    }
    else
    {
        section[escapedKey] = StringToBase64(std::string(static_cast<const char *>(value), size));
    }

    mConfig.sections[kDefaultSectionName] = section;
    return CommitConfig(mDirectory, mName);
}

CHIP_ERROR NodeIdStorage::SyncDeleteKeyValue(const char * key)
{
    auto section = mConfig.sections[kDefaultSectionName];

    ReturnErrorCodeIf(!SyncDoesKeyExist(key), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    std::string escapedKey = EscapeKey(key);
    section.erase(escapedKey);

    mConfig.sections[kDefaultSectionName] = section;
    return CommitConfig(mDirectory, mName);
}

bool NodeIdStorage::SyncDoesKeyExist(const char * key)
{
    std::string escapedKey = EscapeKey(key);
    auto section           = mConfig.sections[kDefaultSectionName];
    auto it                = section.find(escapedKey);
    return (it != section.end());
}

void NodeIdStorage::DumpKeys() const
{
#if CHIP_PROGRESS_LOGGING
    for (const auto & section : mConfig.sections)
    {
        const std::string & sectionName = section.first;
        const auto & sectionContent     = section.second;

        ChipLogProgress(chipTool, "[%s]", sectionName.c_str());
        for (const auto & entry : sectionContent)
        {
            const std::string & keyName = entry.first;
            ChipLogProgress(chipTool, "  => %s", UnescapeKey(keyName).c_str());
        }
    }
#endif // CHIP_PROGRESS_LOGGING
}

CHIP_ERROR NodeIdStorage::SyncClearAll()
{
    ChipLogProgress(chipTool, "Clearing %s storage", kDefaultSectionName);
    auto section = mConfig.sections[kDefaultSectionName];
    section.clear();
    mConfig.sections[kDefaultSectionName] = section;
    return CommitConfig(mDirectory, mName);
}

const char * NodeIdStorage::GetDirectory() const
{
    return GetUsedDirectory(mDirectory);
}

CHIP_ERROR NodeIdStorage::CommitConfig(const char * directory, const char * name)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::ofstream ofs;
    std::string tmpPath = GetFilename(directory, name) + ".tmp";
    ofs.open(tmpPath, std::ofstream::out | std::ofstream::trunc);
    VerifyOrExit(ofs.good(), err = CHIP_ERROR_WRITE_FAILED);

    mConfig.generate(ofs);
    ofs.close();
    VerifyOrExit(ofs.good(), err = CHIP_ERROR_WRITE_FAILED);

    VerifyOrExit(rename(tmpPath.c_str(), GetFilename(directory, name).c_str()) == 0, err = CHIP_ERROR_WRITE_FAILED);

exit:
    return err;
}

std::string ConvertScopedNodeIdToKey(chip::ScopedNodeId & scopedNodeId)
{
    chip::NodeId nodeId           = scopedNodeId.GetNodeId();
    chip::FabricIndex fabricIndex = scopedNodeId.GetFabricIndex();

    std::stringstream ss;

    ss << nodeId << "_" << static_cast<int>(fabricIndex);

    return ss.str();
}

chip::ScopedNodeId ConvertKeyToScopedNodeId(std::string key)
{
    NodeId nodeId;
    FabricIndex fabricIndex;
    int fabricIndexInt;

    std::replace(key.begin(), key.end(), '_', ' ');

    std::stringstream ss(key);

    ss >> nodeId >> fabricIndexInt;

    fabricIndex = static_cast<FabricIndex>(fabricIndexInt);

    return chip::ScopedNodeId(nodeId, fabricIndex);
}

CHIP_ERROR NodeIdStorage::GetScopedNodeId(chip::ScopedNodeId & scopedNodeId, void * buffer, uint16_t & size)
{
    std::string key = ConvertScopedNodeIdToKey(scopedNodeId);

    return SyncGetKeyValue(key.c_str(), buffer, size);
}

CHIP_ERROR NodeIdStorage::SetScopedNodeId(chip::ScopedNodeId & scopedNodeId, const void * value, uint16_t size)
{
    std::string key = ConvertScopedNodeIdToKey(scopedNodeId);

    return SyncSetKeyValue(key.c_str(), value, size);
}

CHIP_ERROR NodeIdStorage::DeleteScopedNodeId(chip::ScopedNodeId & scopedNodeId)
{
    std::string key = ConvertScopedNodeIdToKey(scopedNodeId);

    return SyncDeleteKeyValue(key.c_str());
}

bool NodeIdStorage::DoesScopedNodeIdExist(chip::ScopedNodeId & scopedNodeId)
{
    std::string key = ConvertScopedNodeIdToKey(scopedNodeId);

    return SyncDoesKeyExist(key.c_str());
}

std::vector<chip::ScopedNodeId> NodeIdStorage::GetScopedNodeIdList()
{
    std::vector<chip::ScopedNodeId> nodeIdSet;
    auto section = mConfig.sections[kDefaultSectionName];

    for (const auto & entry : section)
    {
        const std::string & key = entry.first;

        chip::ScopedNodeId scopedNodeId = ConvertKeyToScopedNodeId(key);

        nodeIdSet.push_back(scopedNodeId);
    }

    return nodeIdSet;
}

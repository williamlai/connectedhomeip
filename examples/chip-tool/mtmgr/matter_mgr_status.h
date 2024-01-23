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
/**
 *
 * @file matter_mgr_status.h
 * @ingroup MT_STATUS
 * @brief Definition of Matter status codes.
 */

#ifndef _MT_STATUS_H_
#define _MT_STATUS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    /**
     * @name General
     * @brief Basic status returns.
     *        Reserved value[0 to -99]
     * @{
     */

    /** Operation completed successfully */
    MT_STATUS_OK = 0,
    /** Unspecified run-time error */
    MT_STATUS_GENERAL_ERROR = -1,
    /** Operation timed out */
    MT_STATUS_TIMEOUT = -2,
    /** Resource not available */
    MT_STATUS_OUT_OF_RESOURCES = -3,
    /** Failed to allocate memory */
    MT_STATUS_OUT_OF_MEMORY = -4,
    /** Out of file handles */
    MT_STATUS_OUT_OF_HANDLES = -5,
    /** Not supported on this platform */
    MT_STATUS_NOT_SUPPORTED = -6,
    /** No permission for operation */
    MT_STATUS_NO_PERMISSION = -7,
    /** Indicated resource not found */
    MT_STATUS_NOT_FOUND = -8,
    /** Null pointer provided */
    MT_STATUS_NULL_POINTER = -9,
    /** Parameter out of range */
    MT_STATUS_PARAM_OUT_OF_RANGE = -10,
    /** Parameter value bad */
    MT_STATUS_BAD_PARAM = -11,
    /** Parameters form incompatible set */
    MT_STATUS_INCOMPATIBLE_PARAMS = -12,
    /** Input/Output error */
    MT_STATUS_IO_ERROR = -13,
    /** Safe to try again */
    MT_STATUS_TRY_AGAIN = -14,
    /** Resource busy */
    MT_STATUS_BUSY = -15,
    /** Mutex in dead lock */
    MT_STATUS_DEAD_LOCK = -16,
    /** Defined data type overflowed */
    MT_STATUS_DATA_TYPE_OVERFLOW = -17,
    /** Destination buffer overflowed */
    MT_STATUS_BUFFER_OVERFLOW = -18,
    /** Operation already in progress */
    MT_STATUS_IN_PROGRESS = -19,
    /** Operation canceled */
    MT_STATUS_CANCELED = -20,
    /** Owner of resource died */
    MT_STATUS_OWNER_DEAD = -21,
    /** Unrecoverable error */
    MT_STATUS_UNRECOVERABLE = -22,
    /** Invalid port */
    MT_STATUS_PORT_INVALID = -23,
    /** Device port not opened */
    MT_STATUS_PORT_NOT_OPEN = -24,
    /** Resource uninitialized */
    MT_STATUS_UNINITIALIZED = -25,
    /** Resource already initialized */
    MT_STATUS_ALREADY_INITIALIZED = -26,
    /** Resource already exists */
    MT_STATUS_ALREADY_EXISTS = -27,
    /** Parameter below acceptable threshold */
    MT_STATUS_BELOW_THRESHOLD = -28,
    /** Resource stopped */
    MT_STATUS_STOPPED = -29,
    /** Storage read failure */
    MT_STATUS_STORAGE_READ_FAIL = -30,
    /** Storage write failure */
    MT_STATUS_STORAGE_WRITE_FAIL = -31,
    /** Storage erase failure */
    MT_STATUS_STORAGE_ERASE_FAIL = -32,
    /** Storage is full */
    MT_STATUS_STORAGE_FULL = -33,
    /** API/Operation is not implemented */
    MT_STATUS_NOT_IMPLEMENTED = -34,
    /** Resource can be reclaimed */
    MT_STATUS_RESOURCE_RECLAIMABLE = -35,
    /** Data is corrupted */
    MT_STATUS_DATA_CORRUPTED = -36,
    /** Connected */
    MT_STATUS_CONNECTED = -37,
    /** Disconnected */
    MT_STATUS_DISCONNECTED = -38,
    /** Reset occured */
    MT_STATUS_RESET = -39,
    /* @} */

    /**
     * @name File operation
     * @brief Errors related to file operations
     *        Reserved value[-100 to -199]
     * @{
     */

    /* @} */

    /**
     * @name HAL
     * @brief Errors related to hardware abstraction layer
     *        Reserved value[-10000 to -10999]
     * @{
     */

    /* @} */

} mt_status_t;

#ifdef __cplusplus
}
#endif

#endif /* _MT_STATUS_H_ */
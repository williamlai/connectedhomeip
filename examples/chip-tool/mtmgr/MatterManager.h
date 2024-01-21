#ifndef _MATTER_MANAGER_H_
#define _MATTER_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "matter_mgr_status.h"

mt_status_t matterMgr_init(void);

mt_status_t matterMgr_isReady(void);

mt_status_t matterMgr_deInit(void);

typedef uint16_t matter_nodeId_t;

typedef struct
{
    // uint8_t home_id;
    // matter_nodeId_t ctl_id;
    size_t node_cnt;
} matter_net_t;

mt_status_t matterMgr_getNetworkInfo(matter_net_t * ret_net_info);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _MATTER_MANAGER_H_ */
#ifndef _MATTER_MANAGER_H_
#define _MATTER_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void Mtmgr_init();

uint32_t Mtmgr_DiscoverCommissionables();
uint32_t Mtmgr_PairOnNetworkLong();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _MATTER_MANAGER_H_ */
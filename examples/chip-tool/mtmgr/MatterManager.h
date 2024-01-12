#ifndef _MATTER_MANAGER_H_
#define _MATTER_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint32_t Mtmgr_setUpStack();
void Mtmgr_tearDownStack();

uint32_t Mtmgr_discoverCommissionables();

uint32_t Mtmgr_pairingCommandOnNetworkLong(uint64_t nodeId, uint64_t mDiscoveryFilterCode);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _MATTER_MANAGER_H_ */

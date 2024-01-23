#include "MatterManager.h"

#include <vector>

#include "MatterManagerCore.h"
#include "MtDiscoverCommissionablesCommand.h"
#include "MtPairingCommand.h"
#include "NodeIdStorage.h"

#include <lib/core/CASEAuthTag.h>

#include <commands/common/CredentialIssuerCommands.h>
#include <commands/example/ExampleCredentialIssuerCommands.h>

#include <lib/dnssd/Discovery_ImplPlatform.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if 1
ExampleCredentialIssuerCommands credIssuerCommands;
MatterManagerCore sMtmgrCore(&credIssuerCommands);
#endif

void Mtmgr_test()
{
#if 0
    NodeIdStorage sNodeIdStorage;

    sNodeIdStorage.Init("node", nullptr);

    chip::NodeId nodeId           = 41;
    chip::FabricIndex fabricIndex = 1;
    chip::ScopedNodeId scopedNodeId1(nodeId, fabricIndex);
    sNodeIdStorage.SetScopedNodeId(scopedNodeId1, "test", 4);

    std::vector<chip::ScopedNodeId> list = sNodeIdStorage.GetScopedNodeIdList();

    std::cout << "Node List:" << std::endl;
    for (auto & scopedNodeId : list)
    {
        std::cout << "Node ID: " << scopedNodeId.GetNodeId()
                  << ", Fabric Index: " << static_cast<int>(scopedNodeId.GetFabricIndex()) << std::endl;
    }
#endif
}

#if 1
mt_status_t matterMgr_init(void)
{
    return MT_STATUS_OK;
}

mt_status_t matterMgr_isReady(void)
{
    return MT_STATUS_OK;
}

mt_status_t matterMgr_deInit(void)
{
    return MT_STATUS_OK;
}

mt_status_t matterMgr_getNetworkInfo(matter_net_t * ret_net_info)
{
    mt_status_t res = MT_STATUS_OK;

    if (ret_net_info == NULL)
    {
        res = MT_STATUS_BAD_PARAM;
    }
    else
    {
        memset(ret_net_info, 0, sizeof(matter_net_t));

        std::vector<chip::ScopedNodeId> nodeIdList = sMtmgrCore.GetNodeIdStorage().GetScopedNodeIdList();

        ret_net_info->node_cnt = nodeIdList.size();
    }

    return res;
}
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

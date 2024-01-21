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

ExampleCredentialIssuerCommands credIssuerCommands;
MatterManagerCore sMtmgrCore(&credIssuerCommands);

#if 0
void Mtmgr_test()
{
    // CHIP_ERROR err = CHIP_NO_ERROR;
    MatterManagerCore mtmgrCore(&credIssuerCommands);

    // MtPairOnNetworkLong paringOnNetworkLong(mtmgrCore, 41, 12345678, 1001);
    // err = paringOnNetworkLong.Run();
    // std::cout << "paringOnNetworkLong.Run() ret:" << err.AsString() << std::endl;

    MtDiscoverCommissionablesCommand discoverCommissionables(mtmgrCore);
    discoverCommissionables.setDiscoverOnce(false);
    discoverCommissionables.setTimeout(chip::System::Clock::Seconds16(1));
    err = discoverCommissionables.Run();
    std::cout << "discoverCommissionables.Run() ret:" << err.AsString() << std::endl;
}
#endif

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

#ifdef __cplusplus
} /* extern "C" */
#endif

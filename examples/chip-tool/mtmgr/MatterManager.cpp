#include "MatterManagerCore.h"
#include "MtDiscoverCommissionablesCommand.h"
#include "MtPairingCommand.h"

#include <lib/core/CASEAuthTag.h>

#include <commands/common/CredentialIssuerCommands.h>
#include <commands/example/ExampleCredentialIssuerCommands.h>

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

static ExampleCredentialIssuerCommands credIssuerCommands;

void Mtmgr_test()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    MatterManagerCore mtmgrCore(&credIssuerCommands);

    MtDiscoverCommissionablesCommand discoverCommissionables(mtmgrCore);
    discoverCommissionables.setDiscoverOnce(false);
    err = discoverCommissionables.Run();
    std::cout << "discoverCommissionables.Run() ret:" << err.AsString() << std::endl;

    // MtPairOnNetworkLong paringOnNetworkLong(mtmgrCore, 41, 12345678, 1001);
    // err = paringOnNetworkLong.Run();
    // std::cout << "paringOnNetworkLong.Run() ret:" << err.AsString() << std::endl;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

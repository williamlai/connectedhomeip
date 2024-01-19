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

void Mtmgr_test()
{
    // CHIP_ERROR err = CHIP_NO_ERROR;
#if 0
    MatterManagerCore mtmgrCore(&credIssuerCommands);

    // MtPairOnNetworkLong paringOnNetworkLong(mtmgrCore, 41, 12345678, 1001);
    // err = paringOnNetworkLong.Run();
    // std::cout << "paringOnNetworkLong.Run() ret:" << err.AsString() << std::endl;

    MtDiscoverCommissionablesCommand discoverCommissionables(mtmgrCore);
    discoverCommissionables.setDiscoverOnce(false);
    discoverCommissionables.setTimeout(chip::System::Clock::Seconds16(1));
    err = discoverCommissionables.Run();
    std::cout << "discoverCommissionables.Run() ret:" << err.AsString() << std::endl;
#endif
}

#ifdef __cplusplus
} /* extern "C" */
#endif

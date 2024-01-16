#include "MatterManagerCore.h"
#include "MtDiscoverCommissionablesCommand.h"

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
    MatterManagerCore mtmgrCore(&credIssuerCommands);

    MtDiscoverCommissionablesCommand discoverCommissionables(mtmgrCore);

    discoverCommissionables.Run();
}

#ifdef __cplusplus
} /* extern "C" */
#endif

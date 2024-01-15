#include "MatterManager.h"

#include <commands/common/RemoteDataModelLogger.h>
#include <lib/core/CASEAuthTag.h>

#include <commands/common/CredentialIssuerCommands.h>
#include <commands/example/ExampleCredentialIssuerCommands.h>

#include <commands/discover/DiscoverCommissionablesCommand.h>
#include <commands/pairing/Commands.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

static ExampleCredentialIssuerCommands mCredIssuerCmds;

static Commands commands;

/* discover command set*/
static DiscoverCommissionablesCommand mDiscoverCommissionablesCommand(&mCredIssuerCmds);

/* pairing command set */
static PairOnNetworkLong mPairOnNetworkLong(&mCredIssuerCmds);

#ifdef __cplusplus
extern "C" {
#endif

static CHIP_ERROR prvDiscoverCommissionables()
{
    mDiscoverCommissionablesCommand.ResetArguments();
    return mDiscoverCommissionablesCommand.Run();
}

uint32_t Mtmgr_DiscoverCommissionables()
{
    return (uint32_t) prvDiscoverCommissionables().AsInteger();
}

static CHIP_ERROR prvPairOnNetworkLong()
{
    mPairOnNetworkLong.ResetArguments();
    int argc = 3;

    const char * argv[] = { "41", "12345678", "1001" };
    if (!mPairOnNetworkLong.InitArguments(argc, (char **) argv))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        return mPairOnNetworkLong.Run();
    }
}

uint32_t Mtmgr_PairOnNetworkLong()
{
    return (uint32_t) prvPairOnNetworkLong().AsInteger();
}

void Mtmgr_init()
{
    chip::Platform::MemoryInit();
}

#ifdef __cplusplus
} /* extern "C" */
#endif
/**
 * Linux related implementation is in
 *      examples/chip-tool/third_party/connectedhomeip/src/include/platform/internal/GenericPlatformManagerImpl_POSIX.ipp"
 */

#include <lib/core/CASEAuthTag.h>

#include <commands/common/CredentialIssuerCommands.h>
#include <commands/example/ExampleCredentialIssuerCommands.h>

#include <app/icd/client/DefaultICDClientStorage.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExamplePersistentStorage.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <crypto/RawKeySessionKeystore.h>
#include <lib/core/CHIPError.h>
#include <lib/support/TestGroupData.h>
#include <thread>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef __cplusplus
} /* extern "C" */
#endif

/* Data member from main executable app */
static ExampleCredentialIssuerCommands credIssuerCommands;

/* Data member from Command.h */
static bool mAdvertiseOperational = false;

/* Data member from Commands.h */
static PersistentStorage mStorage;

/* Data members and defines from CHIPCommand.h */
static constexpr char kIdentityAlpha[] = "alpha";
static constexpr char kIdentityBeta[]  = "beta";
static constexpr char kIdentityGamma[] = "gamma";

struct CommissionerIdentity
{
    bool operator<(const CommissionerIdentity & other) const
    {
        return mName < other.mName || (mName == other.mName && mLocalNodeId < other.mLocalNodeId);
    }
    std::string mName;
    chip::NodeId mLocalNodeId;
    uint8_t mRCAC[chip::Controller::kMaxCHIPDERCertLength] = {};
    uint8_t mICAC[chip::Controller::kMaxCHIPDERCertLength] = {};
    uint8_t mNOC[chip::Controller::kMaxCHIPDERCertLength]  = {};

    size_t mRCACLen;
    size_t mICACLen;
    size_t mNOCLen;
};

static std::map<CommissionerIdentity, std::unique_ptr<chip::Controller::DeviceCommissioner>> mCommissioners;

static PersistentStorage mDefaultStorage;
static PersistentStorage mCommissionerStorage;
static chip::PersistentStorageOperationalKeystore mOperationalKeystore;
static chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
static CredentialIssuerCommands * mCredIssuerCmds = &credIssuerCommands;

static constexpr char kIdentityNull[] = "null-fabric-commissioner";

static constexpr uint16_t kMaxGroupsPerFabric    = 50;
static constexpr uint16_t kMaxGroupKeysPerFabric = 25;

/* Static data from CHIPCommand.cpp */
static constexpr chip::FabricId kIdentityNullFabricId  = chip::kUndefinedFabricId;
static constexpr chip::FabricId kIdentityAlphaFabricId = 1;
static constexpr chip::FabricId kIdentityBetaFabricId  = 2;
static constexpr chip::FabricId kIdentityGammaFabricId = 3;
static constexpr chip::FabricId kIdentityOtherFabricId = 4;
static constexpr char kPAATrustStorePathVariable[]     = "CHIPTOOL_PAA_TRUST_STORE_PATH";
static constexpr char kCDTrustStorePathVariable[]      = "CHIPTOOL_CD_TRUST_STORE_PATH";

const chip::Credentials::AttestationTrustStore * sTrustStore = nullptr;
static chip::Credentials::GroupDataProviderImpl sGroupDataProvider{ kMaxGroupsPerFabric, kMaxGroupKeysPerFabric };
/* All fabrics share the same ICD client storage. */
static chip::app::DefaultICDClientStorage sICDClientStorage;
static chip::Crypto::RawKeySessionKeystore sSessionKeystore;

/* Additional global variable for handling the event loop */
// static bool mIsRunningEventLoop = false;
static std::thread eventLoopThread;

/* variables for handling the timed wait */
static std::mutex cvWaitingForResponseMutex;
static bool mWaitingForResponse = true;

/* Data member from PairingCommand.h */
static bool mDeviceIsICD;

#ifdef __cplusplus
extern "C" {
#endif

/* Function prototypes */
static CHIP_ERROR GetIdentityNodeId(std::string identity, chip::NodeId * nodeId);
static CHIP_ERROR EnsureCommissionerForIdentity(std::string identity);

static CHIP_ERROR StartWaiting(chip::System::Clock::Timeout duration);
static void StopWaiting();

static CHIP_ERROR InitializeCommissioner(CommissionerIdentity & identity, chip::FabricId fabricId)
{
    std::unique_ptr<chip::Controller::DeviceCommissioner> commissioner = std::make_unique<chip::Controller::DeviceCommissioner>();
    chip::Controller::SetupParams commissionerParams;

    ReturnLogErrorOnFailure(mCredIssuerCmds->SetupDeviceAttestation(commissionerParams, sTrustStore));

    chip::Crypto::P256Keypair ephemeralKey;

    if (fabricId != chip::kUndefinedFabricId)
    {
        ReturnLogErrorOnFailure(mCommissionerStorage.Init(identity.mName.c_str(), nullptr));

        ReturnLogErrorOnFailure(mCredIssuerCmds->InitializeCredentialsIssuer(mCommissionerStorage));

        chip::MutableByteSpan nocSpan(identity.mNOC);
        chip::MutableByteSpan icacSpan(identity.mICAC);
        chip::MutableByteSpan rcacSpan(identity.mRCAC);

        ReturnLogErrorOnFailure(ephemeralKey.Initialize(chip::Crypto::ECPKeyTarget::ECDSA));

        ReturnLogErrorOnFailure(mCredIssuerCmds->GenerateControllerNOCChain(identity.mLocalNodeId, fabricId,
                                                                            mCommissionerStorage.GetCommissionerCATs(),
                                                                            ephemeralKey, rcacSpan, icacSpan, nocSpan));

        identity.mRCACLen = rcacSpan.size();
        identity.mICACLen = icacSpan.size();
        identity.mNOCLen  = nocSpan.size();

        commissionerParams.operationalKeypair           = &ephemeralKey;
        commissionerParams.controllerRCAC               = rcacSpan;
        commissionerParams.controllerICAC               = icacSpan;
        commissionerParams.controllerNOC                = nocSpan;
        commissionerParams.permitMultiControllerFabrics = true;
        commissionerParams.enableServerInteractions     = false; /* ref mAdvertiseOperational default value */
    }

    commissionerParams.operationalCredentialsDelegate = mCredIssuerCmds->GetCredentialIssuer();
    commissionerParams.controllerVendorId             = chip::VendorId::TestVendor1;

    ReturnLogErrorOnFailure(
        chip::Controller::DeviceControllerFactory::GetInstance().SetupCommissioner(commissionerParams, *(commissioner.get())));

    if (identity.mName != kIdentityNull)
    {
        /* Initialize Group Data, including IPK */
        chip::FabricIndex fabricIndex = commissioner->GetFabricIndex();
        uint8_t compressed_fabric_id[sizeof(uint64_t)];
        chip::MutableByteSpan compressed_fabric_id_span(compressed_fabric_id);
        ReturnLogErrorOnFailure(commissioner->GetCompressedFabricIdBytes(compressed_fabric_id_span));

        ReturnLogErrorOnFailure(chip::GroupTesting::InitData(&sGroupDataProvider, fabricIndex, compressed_fabric_id_span));

        // Configure the default IPK for all fabrics used by CHIP-tool. The epoch
        // key is the same, but the derived keys will be different for each fabric.
        chip::ByteSpan defaultIpk = chip::GroupTesting::DefaultIpkValue::GetDefaultIpk();
        ReturnLogErrorOnFailure(
            chip::Credentials::SetSingleIpkEpochKey(&sGroupDataProvider, fabricIndex, defaultIpk, compressed_fabric_id_span));
    }

    sICDClientStorage.UpdateFabricList(commissioner->GetFabricIndex());

    mCommissioners[identity] = std::move(commissioner);

    return CHIP_NO_ERROR;
}

static CHIP_ERROR setUpStack()
{
    ReturnLogErrorOnFailure(chip::Platform::MemoryInit());

    ReturnLogErrorOnFailure(mStorage.Init());

    chip::Logging::SetLogFilter(mStorage.GetLoggingLevel());

    /* default "/tmp/chip_tool_config.ini" */
    ReturnLogErrorOnFailure(mDefaultStorage.Init(nullptr, nullptr));
    ReturnLogErrorOnFailure(mOperationalKeystore.Init(&mDefaultStorage));
    ReturnLogErrorOnFailure(mOpCertStore.Init(&mDefaultStorage));

    /* ICD storage lifetime is currently tied to the chip-tool's lifetime. */
    ReturnLogErrorOnFailure(sICDClientStorage.Init(&mDefaultStorage, &sSessionKeystore));

    chip::Controller::FactoryInitParams factoryInitParams;

    factoryInitParams.fabricIndependentStorage = &mDefaultStorage;
    factoryInitParams.operationalKeystore      = &mOperationalKeystore;
    factoryInitParams.opCertStore              = &mOpCertStore;
    factoryInitParams.enableServerInteractions = mAdvertiseOperational;
    factoryInitParams.sessionKeystore          = &sSessionKeystore;

    sGroupDataProvider.SetStorageDelegate(&mDefaultStorage);
    sGroupDataProvider.SetSessionKeystore(factoryInitParams.sessionKeystore);
    ReturnLogErrorOnFailure(sGroupDataProvider.Init());
    chip::Credentials::SetGroupDataProvider(&sGroupDataProvider);
    factoryInitParams.groupDataProvider = &sGroupDataProvider;

    factoryInitParams.listenPort = CHIP_PORT;
    ReturnLogErrorOnFailure(chip::Controller::DeviceControllerFactory::GetInstance().Init(factoryInitParams));

    sTrustStore = chip::Credentials::GetTestAttestationTrustStore();

    CommissionerIdentity nullIdentity{ kIdentityNull, chip::kUndefinedNodeId };
    ReturnLogErrorOnFailure(InitializeCommissioner(nullIdentity, kIdentityNullFabricId));

    return CHIP_NO_ERROR;
}

static std::string GetIdentity()
{
    std::string name = kIdentityAlpha;
    if (name.compare(kIdentityAlpha) != 0 && name.compare(kIdentityBeta) != 0 && name.compare(kIdentityGamma) != 0 &&
        name.compare(kIdentityNull) != 0)
    {
        chip::FabricId fabricId = strtoull(name.c_str(), nullptr, 0);
        if (fabricId >= kIdentityOtherFabricId)
        {
            // normalize name since it is used in persistent storage

            char s[24];
            sprintf(s, "%" PRIu64, fabricId);

            name = s;
        }
        else
        {
            ChipLogError(chipTool, "Unknown commissioner name: %s. Supported names are [%s, %s, %s, 4, 5...]", name.c_str(),
                         kIdentityAlpha, kIdentityBeta, kIdentityGamma);
            chipDie();
        }
    }

    return name;
}

static chip::Controller::DeviceCommissioner & GetCommissioner(std::string identity)
{
    // We don't have a great way to handle commissioner setup failures here.
    // This only matters for commands (like TestCommand) that involve multiple
    // identities.
    VerifyOrDie(EnsureCommissionerForIdentity(identity) == CHIP_NO_ERROR);

    chip::NodeId nodeId;
    VerifyOrDie(GetIdentityNodeId(identity, &nodeId) == CHIP_NO_ERROR);
    CommissionerIdentity lookupKey{ identity, nodeId };
    auto item = mCommissioners.find(lookupKey);
    VerifyOrDie(item != mCommissioners.end());
    return *item->second;
}

static chip::Controller::DeviceCommissioner & CurrentCommissioner()
{
    return GetCommissioner(GetIdentity());
}

static CHIP_ERROR GetIdentityNodeId(std::string identity, chip::NodeId * nodeId)
{
    if (identity == kIdentityNull)
    {
        *nodeId = chip::kUndefinedNodeId;
        return CHIP_NO_ERROR;
    }

    ReturnLogErrorOnFailure(mCommissionerStorage.Init(identity.c_str(), nullptr));

    *nodeId = mCommissionerStorage.GetLocalNodeId();

    return CHIP_NO_ERROR;
}

static CHIP_ERROR EnsureCommissionerForIdentity(std::string identity)
{
    chip::NodeId nodeId;
    ReturnErrorOnFailure(GetIdentityNodeId(identity, &nodeId));
    CommissionerIdentity lookupKey{ identity, nodeId };
    if (mCommissioners.find(lookupKey) != mCommissioners.end())
    {
        return CHIP_NO_ERROR;
    }

    // Need to initialize the commissioner.
    chip::FabricId fabricId;
    if (identity == kIdentityAlpha)
    {
        fabricId = kIdentityAlphaFabricId;
    }
    else if (identity == kIdentityBeta)
    {
        fabricId = kIdentityBetaFabricId;
    }
    else if (identity == kIdentityGamma)
    {
        fabricId = kIdentityGammaFabricId;
    }
    else
    {
        fabricId = strtoull(identity.c_str(), nullptr, 0);
        if (fabricId < kIdentityOtherFabricId)
        {
            ChipLogError(chipTool, "Invalid identity: %s", identity.c_str());
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    return InitializeCommissioner(lookupKey, fabricId);
}

static void OnResponseTimeout(chip::System::Layer *, void * appState)
{
    StopWaiting();
    *(reinterpret_cast<bool *>(appState)) = true;
}

static CHIP_ERROR StartWaiting(chip::System::Clock::Timeout duration)
{
    bool timedOut = false;

    {
        std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
        mWaitingForResponse = true;
    }

    ReturnLogErrorOnFailure(chip::DeviceLayer::SystemLayer().StartTimer(duration, OnResponseTimeout, &timedOut));

    while (mWaitingForResponse)
    {
        chip::DeviceLayer::PlatformMgr().RunEventLoop();
    }

    return timedOut ? CHIP_ERROR_TIMEOUT : CHIP_NO_ERROR;
}

static void StopWaiting()
{
    {
        std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
        mWaitingForResponse = false;
    }
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
}

class DeviceDiscoveryDelegateImpl : public chip::Controller::DeviceDiscoveryDelegate
{
public:
    void OnDiscoveredDevice(const chip::Dnssd::DiscoveredNodeData & nodeData) override
    {
        nodeData.LogDetail();

        StopWaiting();
    }
};

CHIP_ERROR DiscoverCommissionablesCommand()
{
    chip::Controller::DeviceCommissioner * mCommissioner = &CurrentCommissioner();
    DeviceDiscoveryDelegateImpl delegator;

    mCommissioner->RegisterDeviceDiscoveryDelegate(&delegator);
    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kNone, (uint64_t) 0);

    mCommissioner->DiscoverCommissionableNodes(filter);

    StartWaiting(chip::System::Clock::Seconds16(30));

    return CHIP_NO_ERROR;
}

/**
 * Refere to functions in PairingCommand.cpp
 *      PairingCommand::RunCommand()
 *      CHIP_ERROR PairingCommand::RunInternal(NodeId remoteId)
 *      CHIP_ERROR PairingCommand::PairWithMdns(NodeId remoteId)
 */
CHIP_ERROR PairingCommandOnNetworkLong(chip::NodeId nodeId, uint64_t mDiscoveryFilterCode)
{
    mCredIssuerCmds->SetCredentialIssuerCATValues(chip::kUndefinedCATs);

    mDeviceIsICD = false;

    chip::Dnssd::DiscoveryFilter filter(chip::Dnssd::DiscoveryFilterType::kLongDiscriminator);
    filter.code = mDiscoveryFilterCode;

    DeviceDiscoveryDelegateImpl delegator;
    CurrentCommissioner().RegisterDeviceDiscoveryDelegate(&delegator);

    return CHIP_NO_ERROR;
}

uint32_t Mtmgr_setUpStack()
{
    return (uint32_t) setUpStack().AsInteger();
}

void Mtmgr_tearDownStack()
{
    for (auto & commissioner : mCommissioners)
    {
        mCommissioners[commissioner.first].get()->Shutdown();
    }

    chip::Controller::DeviceControllerFactory::GetInstance().Shutdown();
}

uint32_t Mtmgr_discoverCommissionables()
{
    return (uint32_t) DiscoverCommissionablesCommand().AsInteger();
}

uint32_t Mtmgr_pairingCommandOnNetworkLong(uint64_t nodeId, uint64_t mDiscoveryFilterCode)
{
    return (uint32_t) PairingCommandOnNetworkLong(nodeId, mDiscoveryFilterCode).AsInteger();
}

#ifdef __cplusplus
} /* extern "C" */
#endif

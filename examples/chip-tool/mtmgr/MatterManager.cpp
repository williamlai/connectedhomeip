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

#ifdef __cplusplus
extern "C" {
#endif

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

CHIP_ERROR setUpStack()
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

#ifdef __cplusplus
} /* extern "C" */
#endif

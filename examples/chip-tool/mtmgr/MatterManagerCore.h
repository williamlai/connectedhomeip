#pragma once

#include <lib/core/CASEAuthTag.h>

#include <commands/common/CredentialIssuerCommands.h>
#include <commands/example/ExampleCredentialIssuerCommands.h>

#include <app/icd/client/CheckInHandler.h>
#include <app/icd/client/DefaultCheckInDelegate.h>
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

inline constexpr char kIdentityAlpha[] = "alpha";
inline constexpr char kIdentityBeta[]  = "beta";
inline constexpr char kIdentityGamma[] = "gamma";
// The null fabric commissioner is a commissioner that isn't on a fabric.
// This is a legal configuration in which the commissioner delegates
// operational communication and invocation of the commssioning complete
// command to a separate on-fabric administrator node.
//
// The null-fabric-commissioner identity is provided here to demonstrate the
// commissioner portion of such an architecture.  The null-fabric-commissioner
// can carry a commissioning flow up until the point of operational channel
// (CASE) communcation.
inline constexpr char kIdentityNull[] = "null-fabric-commissioner";

// Commissioners are keyed by name and local node id.
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

class MatterManagerCore
{
public:
    using ChipDeviceCommissioner = ::chip::Controller::DeviceCommissioner;
    using ChipDeviceController   = ::chip::Controller::DeviceController;
    using IPAddress              = ::chip::Inet::IPAddress;
    using NodeId                 = ::chip::NodeId;
    using PeerId                 = ::chip::PeerId;
    using PeerAddress            = ::chip::Transport::PeerAddress;

    static constexpr uint16_t kMaxGroupsPerFabric    = 50;
    static constexpr uint16_t kMaxGroupKeysPerFabric = 25;

    MatterManagerCore(CredentialIssuerCommands * credIssuerCmds);
    ~MatterManagerCore();

    const chip::Optional<char *> & GetStorageDirectory() const { return mStorageDirectory; }

    // This method returns the commissioner instance to be used for running the command.
    // The default commissioner instance name is "alpha", but it can be overridden by passing
    // --identity "instance name" when running a command.
    ChipDeviceCommissioner & CurrentCommissioner();

    ChipDeviceCommissioner & GetCommissioner(std::string identity);

protected:
    // mStorageDirectory lives here so we can just set it in RunAsInteractive.
    chip::Optional<char *> mStorageDirectory;

#ifdef CONFIG_USE_LOCAL_STORAGE
    PersistentStorage mDefaultStorage;
    // TODO: It's pretty weird that we re-init mCommissionerStorage for every
    // identity without shutting it down or something in between...
    PersistentStorage mCommissionerStorage;
#endif // CONFIG_USE_LOCAL_STORAGE
    chip::PersistentStorageOperationalKeystore mOperationalKeystore;
    chip::Credentials::PersistentStorageOpCertStore mOpCertStore;
    static chip::Crypto::RawKeySessionKeystore sSessionKeystore;

    static chip::Credentials::GroupDataProviderImpl sGroupDataProvider;
    static chip::app::DefaultICDClientStorage sICDClientStorage;
    static chip::app::DefaultCheckInDelegate sCheckInDelegate;
    static chip::app::CheckInHandler sCheckInHandler;
    CredentialIssuerCommands * mCredIssuerCmds;

    std::string GetIdentity();
    CHIP_ERROR GetIdentityNodeId(std::string identity, chip::NodeId * nodeId);
    CHIP_ERROR GetIdentityRootCertificate(std::string identity, chip::ByteSpan & span);
    void SetIdentity(const char * name);

private:
    CHIP_ERROR SetUpStack();
    void TearDownStack();

    CHIP_ERROR EnsureCommissionerForIdentity(std::string identity);

    CHIP_ERROR InitializeCommissioner(CommissionerIdentity & identity, chip::FabricId fabricId);
    void ShutdownCommissioner(const CommissionerIdentity & key);
    chip::FabricId CurrentCommissionerId();

    // Cached trust store so commands other than the original startup command
    // can spin up commissioners as needed.
    static const chip::Credentials::AttestationTrustStore * sTrustStore;
};
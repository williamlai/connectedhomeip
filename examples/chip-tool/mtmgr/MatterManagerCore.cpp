/*
 * Copyright 2023-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include "MatterManagerCore.h"

#include <lib/support/CHIPMem.h>

#include <controller/CHIPDeviceControllerFactory.h>
#include <credentials/attestation_verifier/FileAttestationTrustStore.h>

std::map<CommissionerIdentity, std::unique_ptr<chip::Controller::DeviceCommissioner>> mCommissioners;

using DeviceControllerFactory = chip::Controller::DeviceControllerFactory;

constexpr chip::FabricId kIdentityNullFabricId  = chip::kUndefinedFabricId;
constexpr chip::FabricId kIdentityAlphaFabricId = 1;
constexpr chip::FabricId kIdentityBetaFabricId  = 2;
constexpr chip::FabricId kIdentityGammaFabricId = 3;
constexpr chip::FabricId kIdentityOtherFabricId = 4;
constexpr char kPAATrustStorePathVariable[]     = "CHIPTOOL_PAA_TRUST_STORE_PATH";
constexpr char kCDTrustStorePathVariable[]      = "CHIPTOOL_CD_TRUST_STORE_PATH";
constexpr char kNodeIdStoreName[]               = "nodeid";

const chip::Credentials::AttestationTrustStore * MatterManagerCore::sTrustStore = nullptr;
chip::Credentials::GroupDataProviderImpl MatterManagerCore::sGroupDataProvider{ kMaxGroupsPerFabric, kMaxGroupKeysPerFabric };

NodeIdStorage MatterManagerCore::sNodeIdStorage;

CHIP_ERROR GetAttestationTrustStore(const char * paaTrustStorePath, const chip::Credentials::AttestationTrustStore ** trustStore)
{
    if (paaTrustStorePath == nullptr)
    {
        paaTrustStorePath = getenv(kPAATrustStorePathVariable);
    }

    if (paaTrustStorePath == nullptr)
    {
        *trustStore = chip::Credentials::GetTestAttestationTrustStore();
        return CHIP_NO_ERROR;
    }

    static chip::Credentials::FileAttestationTrustStore attestationTrustStore{ paaTrustStorePath };

    if (paaTrustStorePath != nullptr && attestationTrustStore.paaCount() == 0)
    {
        ChipLogError(chipTool, "No PAAs found in path: %s", paaTrustStorePath);
        ChipLogError(chipTool,
                     "Please specify a valid path containing trusted PAA certificates using "
                     "the argument [--paa-trust-store-path paa/file/path] "
                     "or environment variable [%s=paa/file/path]",
                     kPAATrustStorePathVariable);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    *trustStore = &attestationTrustStore;
    return CHIP_NO_ERROR;
}

MatterManagerCore::MatterManagerCore(CredentialIssuerCommands * credIssuerCmds) : mCredIssuerCmds(credIssuerCmds)
{
    chip::Platform::MemoryInit();
}

MatterManagerCore::~MatterManagerCore() {}

CHIP_ERROR MatterManagerCore::SetUpStack()
{
    ReturnLogErrorOnFailure(mDefaultStorage.Init(nullptr, GetStorageDirectory().ValueOr(nullptr)));
    ReturnLogErrorOnFailure(mOperationalKeystore.Init(&mDefaultStorage));
    ReturnLogErrorOnFailure(mOpCertStore.Init(&mDefaultStorage));

    ReturnLogErrorOnFailure(sNodeIdStorage.Init(kNodeIdStoreName, GetStorageDirectory().ValueOr(nullptr)));

    chip::Controller::FactoryInitParams factoryInitParams;

    factoryInitParams.fabricIndependentStorage = &mDefaultStorage;
    factoryInitParams.operationalKeystore      = &mOperationalKeystore;
    factoryInitParams.opCertStore              = &mOpCertStore;
    factoryInitParams.enableServerInteractions = false;
    factoryInitParams.sessionKeystore          = &mSessionKeystore;

    // Init group data provider that will be used for all group keys and IPKs for the
    // chip-tool-configured fabrics. This is OK to do once since the fabric tables
    // and the DeviceControllerFactory all "share" in the same underlying data.
    // Different commissioner implementations may want to use alternate implementations
    // of GroupDataProvider for injection through factoryInitParams.
    sGroupDataProvider.SetStorageDelegate(&mDefaultStorage);
    sGroupDataProvider.SetSessionKeystore(factoryInitParams.sessionKeystore);
    ReturnLogErrorOnFailure(sGroupDataProvider.Init());
    chip::Credentials::SetGroupDataProvider(&sGroupDataProvider);
    factoryInitParams.groupDataProvider = &sGroupDataProvider;

    uint16_t port = mDefaultStorage.GetListenPort();
    if (port != 0)
    {
        // Make sure different commissioners run on different ports.
        port = static_cast<uint16_t>(port + CurrentCommissionerId());
    }
    factoryInitParams.listenPort = port;
    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().Init(factoryInitParams));

    ReturnErrorOnFailure(GetAttestationTrustStore(nullptr, &sTrustStore));

    CommissionerIdentity nullIdentity{ kIdentityNull, chip::kUndefinedNodeId };
    ReturnLogErrorOnFailure(InitializeCommissioner(nullIdentity, kIdentityNullFabricId));

    // After initializing first commissioner, add the additional CD certs once
    {
        // const char * cdTrustStorePath = mCDTrustStorePath.ValueOr(nullptr);
        const char * cdTrustStorePath = nullptr;
        if (cdTrustStorePath == nullptr)
        {
            cdTrustStorePath = getenv(kCDTrustStorePathVariable);
        }

        auto additionalCdCerts = chip::Credentials::LoadAllX509DerCerts(cdTrustStorePath);
        if (cdTrustStorePath != nullptr && additionalCdCerts.size() == 0)
        {
            ChipLogError(chipTool, "Warning: no CD signing certs found in path: %s, only defaults will be used", cdTrustStorePath);
            ChipLogError(chipTool,
                         "Please specify a path containing trusted CD verifying key certificates using "
                         "the argument [--cd-trust-store-path cd/file/path] "
                         "or environment variable [%s=cd/file/path]",
                         kCDTrustStorePathVariable);
        }
        ReturnErrorOnFailure(mCredIssuerCmds->AddAdditionalCDVerifyingCerts(additionalCdCerts));
    }
    // bool allowTestCdSigningKey = !mOnlyAllowTrustedCdKeys.ValueOr(false);
    bool allowTestCdSigningKey = !false;
    mCredIssuerCmds->SetCredentialIssuerOption(CredentialIssuerCommands::CredentialIssuerOptions::kAllowTestCdSigningKey,
                                               allowTestCdSigningKey);

    return CHIP_NO_ERROR;
}

void MatterManagerCore::TearDownStack()
{
    //
    // We can call DeviceController::Shutdown() safely without grabbing the stack lock
    // since the CHIP thread and event queue have been stopped, preventing any thread
    // races.
    //
    for (auto & commissioner : mCommissioners)
    {
        ShutdownCommissioner(commissioner.first);
    }

    chip::Controller::DeviceControllerFactory::GetInstance().Shutdown();
}

CHIP_ERROR MatterManagerCore::EnsureCommissionerForIdentity(std::string identity)
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

std::string MatterManagerCore::GetIdentity()
{
    // std::string name = mCommissionerName.HasValue() ? mCommissionerName.Value() : kIdentityAlpha;
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

CHIP_ERROR MatterManagerCore::GetIdentityNodeId(std::string identity, chip::NodeId * nodeId)
{
#if 0
    if (mCommissionerNodeId.HasValue())
    {
        *nodeId = mCommissionerNodeId.Value();
        return CHIP_NO_ERROR;
    }
#endif
    if (identity == kIdentityNull)
    {
        *nodeId = chip::kUndefinedNodeId;
        return CHIP_NO_ERROR;
    }

    ReturnLogErrorOnFailure(mCommissionerStorage.Init(identity.c_str(), GetStorageDirectory().ValueOr(nullptr)));

    *nodeId = mCommissionerStorage.GetLocalNodeId();

    return CHIP_NO_ERROR;
}

CHIP_ERROR MatterManagerCore::GetIdentityRootCertificate(std::string identity, chip::ByteSpan & span)
{
    if (identity == kIdentityNull)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    chip::NodeId nodeId;
    VerifyOrDie(GetIdentityNodeId(identity, &nodeId) == CHIP_NO_ERROR);
    CommissionerIdentity lookupKey{ identity, nodeId };
    auto item = mCommissioners.find(lookupKey);

    span = chip::ByteSpan(item->first.mRCAC, item->first.mRCACLen);
    return CHIP_NO_ERROR;
}

chip::FabricId MatterManagerCore::CurrentCommissionerId()
{
    chip::FabricId id;

    std::string name = GetIdentity();
    if (name.compare(kIdentityAlpha) == 0)
    {
        id = kIdentityAlphaFabricId;
    }
    else if (name.compare(kIdentityBeta) == 0)
    {
        id = kIdentityBetaFabricId;
    }
    else if (name.compare(kIdentityGamma) == 0)
    {
        id = kIdentityGammaFabricId;
    }
    else if (name.compare(kIdentityNull) == 0)
    {
        id = kIdentityNullFabricId;
    }
    else if ((id = strtoull(name.c_str(), nullptr, 0)) < kIdentityOtherFabricId)
    {
        VerifyOrDieWithMsg(false, chipTool, "Unknown commissioner name: %s. Supported names are [%s, %s, %s, 4, 5...]",
                           name.c_str(), kIdentityAlpha, kIdentityBeta, kIdentityGamma);
    }

    return id;
}

chip::Controller::DeviceCommissioner & MatterManagerCore::CurrentCommissioner()
{
    return GetCommissioner(GetIdentity());
}

chip::Controller::DeviceCommissioner & MatterManagerCore::GetCommissioner(std::string identity)
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

void MatterManagerCore::ShutdownCommissioner(const CommissionerIdentity & key)
{
    mCommissioners[key].get()->Shutdown();
}

CHIP_ERROR MatterManagerCore::InitializeCommissioner(CommissionerIdentity & identity, chip::FabricId fabricId)
{
    std::unique_ptr<chip::Controller::DeviceCommissioner> commissioner = std::make_unique<chip::Controller::DeviceCommissioner>();
    chip::Controller::SetupParams commissionerParams;

    ReturnLogErrorOnFailure(mCredIssuerCmds->SetupDeviceAttestation(commissionerParams, sTrustStore));

    chip::Crypto::P256Keypair ephemeralKey;

    if (fabricId != chip::kUndefinedFabricId)
    {

        // TODO - OpCreds should only be generated for pairing command
        //        store the credentials in persistent storage, and
        //        generate when not available in the storage.
        ReturnLogErrorOnFailure(mCommissionerStorage.Init(identity.mName.c_str(), GetStorageDirectory().ValueOr(nullptr)));
#if 0
        if (mUseMaxSizedCerts.HasValue())
        {
            auto option = CredentialIssuerCommands::CredentialIssuerOptions::kMaximizeCertificateSizes;
            mCredIssuerCmds->SetCredentialIssuerOption(option, mUseMaxSizedCerts.Value());
        }
#endif

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
    }

    // TODO: Initialize IPK epoch key in ExampleOperationalCredentials issuer rather than relying on DefaultIpkValue
    commissionerParams.operationalCredentialsDelegate = mCredIssuerCmds->GetCredentialIssuer();
    // commissionerParams.controllerVendorId             = mCommissionerVendorId.ValueOr(chip::VendorId::TestVendor1);
    commissionerParams.controllerVendorId = chip::VendorId::TestVendor1;

    ReturnLogErrorOnFailure(DeviceControllerFactory::GetInstance().SetupCommissioner(commissionerParams, *(commissioner.get())));

    if (identity.mName != kIdentityNull)
    {
        // Initialize Group Data, including IPK
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

    mCommissioners[identity] = std::move(commissioner);

    return CHIP_NO_ERROR;
}

// Copyright 2018 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#include "tink/signature/rsa_ssa_pkcs1_sign_key_manager.h"

#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "tink/core/key_manager_base.h"
#include "tink/key_manager.h"
#include "tink/public_key_sign.h"
#include "tink/signature/rsa_ssa_pkcs1_verify_key_manager.h"
#include "tink/signature/sig_util.h"
#include "tink/subtle/rsa_ssa_pkcs1_sign_boringssl.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/enums.h"
#include "tink/util/errors.h"
#include "tink/util/protobuf_helper.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "tink/util/validation.h"
#include "proto/rsa_ssa_pkcs1.pb.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

using crypto::tink::util::Enums;
using crypto::tink::util::Status;
using crypto::tink::util::StatusOr;
using google::crypto::tink::KeyData;
using google::crypto::tink::RsaSsaPkcs1KeyFormat;
using google::crypto::tink::RsaSsaPkcs1Params;
using google::crypto::tink::RsaSsaPkcs1PrivateKey;

namespace {
std::unique_ptr<RsaSsaPkcs1PrivateKey> RsaPrivateKeySubtleToProto(
    const subtle::SubtleUtilBoringSSL::RsaPrivateKey& private_key) {
  auto key_proto = absl::make_unique<RsaSsaPkcs1PrivateKey>();
  key_proto->set_version(RsaSsaPkcs1SignKeyManager::kVersion);
  key_proto->set_d(private_key.d);
  key_proto->set_p(private_key.p);
  key_proto->set_q(private_key.q);
  key_proto->set_dp(private_key.dp);
  key_proto->set_dq(private_key.dq);
  key_proto->set_crt(private_key.crt);
  auto* public_key_proto = key_proto->mutable_public_key();
  public_key_proto->set_version(RsaSsaPkcs1SignKeyManager::kVersion);
  public_key_proto->set_n(private_key.n);
  public_key_proto->set_e(private_key.e);
  return key_proto;
}

subtle::SubtleUtilBoringSSL::RsaPrivateKey RsaPrivateKeyProtoToSubtle(
    const RsaSsaPkcs1PrivateKey& key_proto) {
  subtle::SubtleUtilBoringSSL::RsaPrivateKey key;
  key.n = key_proto.public_key().n();
  key.e = key_proto.public_key().e();
  key.d = key_proto.d();
  key.p = key_proto.p();
  key.q = key_proto.q();
  key.dp = key_proto.dp();
  key.dq = key_proto.dq();
  key.crt = key_proto.crt();
  return key;
}

}  // namespace

class RsaSsaPkcs1PrivateKeyFactory
    : public PrivateKeyFactory,
      public KeyFactoryBase<RsaSsaPkcs1PrivateKey, RsaSsaPkcs1KeyFormat> {
 public:
  RsaSsaPkcs1PrivateKeyFactory() {}

  KeyData::KeyMaterialType key_material_type() const override {
    return KeyData::ASYMMETRIC_PRIVATE;
  }

  // Returns KeyData proto that contains RsaSsaPkcs1PublicKey
  // extracted from the given serialized_private_key, which must contain
  // RsaSsaPkcs1PrivateKey-proto.
  crypto::tink::util::StatusOr<std::unique_ptr<google::crypto::tink::KeyData>>
  GetPublicKeyData(absl::string_view serialized_private_key) const override;

 protected:
  StatusOr<std::unique_ptr<RsaSsaPkcs1PrivateKey>> NewKeyFromFormat(
      const RsaSsaPkcs1KeyFormat& rsa_ssa_pkcs1_key_format) const override;
};

StatusOr<std::unique_ptr<RsaSsaPkcs1PrivateKey>>
RsaSsaPkcs1PrivateKeyFactory::NewKeyFromFormat(
    const RsaSsaPkcs1KeyFormat& rsa_ssa_pkcs1_key_format) const {
  util::Status is_valid =
      RsaSsaPkcs1SignKeyManager::Validate(rsa_ssa_pkcs1_key_format);
  if (!is_valid.ok()) return is_valid;

  auto e = subtle::SubtleUtilBoringSSL::str2bn(
      rsa_ssa_pkcs1_key_format.public_exponent());
  if (!e.ok()) return e.status();

  subtle::SubtleUtilBoringSSL::RsaPrivateKey private_key;
  subtle::SubtleUtilBoringSSL::RsaPublicKey public_key;
  util::Status status = subtle::SubtleUtilBoringSSL::GetNewRsaKeyPair(
      rsa_ssa_pkcs1_key_format.modulus_size_in_bits(), e.ValueOrDie().get(),
      &private_key, &public_key);
  if (!status.ok()) return status;

  auto key_proto = RsaPrivateKeySubtleToProto(private_key);
  auto* public_key_proto = key_proto->mutable_public_key();
  *public_key_proto->mutable_params() = rsa_ssa_pkcs1_key_format.params();

  return absl::implicit_cast<StatusOr<std::unique_ptr<RsaSsaPkcs1PrivateKey>>>(
      std::move(key_proto));
}

StatusOr<std::unique_ptr<KeyData>>
RsaSsaPkcs1PrivateKeyFactory::GetPublicKeyData(
    absl::string_view serialized_private_key) const {
  RsaSsaPkcs1PrivateKey private_key;
  if (!private_key.ParseFromString(std::string(serialized_private_key))) {
    return ToStatusF(util::error::INVALID_ARGUMENT,
                     "Could not parse the passed string as proto '%s'.",
                     RsaSsaPkcs1VerifyKeyManager::static_key_type().c_str());
  }
  auto status = RsaSsaPkcs1SignKeyManager::Validate(private_key);
  if (!status.ok()) return status;
  auto key_data = absl::make_unique<KeyData>();
  key_data->set_type_url(RsaSsaPkcs1VerifyKeyManager::static_key_type());
  key_data->set_value(private_key.public_key().SerializeAsString());
  key_data->set_key_material_type(KeyData::ASYMMETRIC_PUBLIC);
  return std::move(key_data);
}

constexpr uint32_t RsaSsaPkcs1SignKeyManager::kVersion;

RsaSsaPkcs1SignKeyManager::RsaSsaPkcs1SignKeyManager()
    : key_factory_(absl::make_unique<RsaSsaPkcs1PrivateKeyFactory>()) {}

const KeyFactory& RsaSsaPkcs1SignKeyManager::get_key_factory() const {
  return *key_factory_;
}

uint32_t RsaSsaPkcs1SignKeyManager::get_version() const { return kVersion; }

StatusOr<std::unique_ptr<PublicKeySign>>
RsaSsaPkcs1SignKeyManager::GetPrimitiveFromKey(
    const RsaSsaPkcs1PrivateKey& key_proto) const {
  Status status = Validate(key_proto);
  if (!status.ok()) return status;
  auto key = RsaPrivateKeyProtoToSubtle(key_proto);
  subtle::SubtleUtilBoringSSL::RsaSsaPkcs1Params params;
  const RsaSsaPkcs1Params& params_proto = key_proto.public_key().params();
  params.hash_type = Enums::ProtoToSubtle(params_proto.hash_type());
  auto signer = subtle::RsaSsaPkcs1SignBoringSsl::New(key, params);
  if (!signer.ok()) return signer.status();
  // To check that the key is correct, we sign a test message with private key
  // and verify with public key.
  auto public_key_data_result =
      key_factory_->GetPublicKeyData(key_proto.SerializeAsString());
  if (!public_key_data_result.ok()) return public_key_data_result.status();
  auto public_key_data = std::move(public_key_data_result.ValueOrDie());
  RsaSsaPkcs1VerifyKeyManager verify_key_manager;
  auto verifier = verify_key_manager.GetPrimitive(*public_key_data);
  if (!verifier.ok()) return verifier.status();
  auto sign_verify_result =
      SignAndVerify(signer.ValueOrDie().get(), verifier.ValueOrDie().get());
  if (!sign_verify_result.ok()) {
    return util::Status(util::error::INTERNAL,
                        "security bug: signing with private key followed by "
                        "verifying with public key failed");
  }
  return signer;
}

// static
Status RsaSsaPkcs1SignKeyManager::Validate(const RsaSsaPkcs1PrivateKey& key) {
  Status status = ValidateVersion(key.version(), kVersion);
  if (!status.ok()) return status;
  return RsaSsaPkcs1VerifyKeyManager::Validate(key.public_key());
}

// static
Status RsaSsaPkcs1SignKeyManager::Validate(
    const RsaSsaPkcs1KeyFormat& key_format) {
  auto modulus_status = subtle::SubtleUtilBoringSSL::ValidateRsaModulusSize(
      key_format.modulus_size_in_bits());
  if (!modulus_status.ok()) return modulus_status;
  return RsaSsaPkcs1VerifyKeyManager::Validate(key_format.params());
}
}  // namespace tink
}  // namespace crypto

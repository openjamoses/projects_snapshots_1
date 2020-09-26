// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "tink/signature/rsa_ssa_pkcs1_sign_key_manager.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "absl/container/flat_hash_set.h"
#include "openssl/rsa.h"
#include "tink/public_key_sign.h"
#include "tink/signature/rsa_ssa_pkcs1_verify_key_manager.h"
#include "tink/subtle/rsa_ssa_pkcs1_verify_boringssl.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "tink/util/test_util.h"
#include "tink/util/test_matchers.h"
#include "proto/rsa_ssa_pkcs1.pb.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {
namespace {

using ::crypto::tink::subtle::SubtleUtilBoringSSL;
using ::crypto::tink::test::IsOk;
using ::crypto::tink::util::StatusOr;
using ::google::crypto::tink::HashType;
using ::google::crypto::tink::KeyData;
using ::google::crypto::tink::RsaSsaPkcs1KeyFormat;
using ::google::crypto::tink::RsaSsaPkcs1PrivateKey;
using ::google::crypto::tink::RsaSsaPkcs1PublicKey;
using ::testing::Eq;
using ::testing::Not;
using ::testing::SizeIs;

TEST(RsaSsaPkcsSignKeyManagerTest, Basic) {
  EXPECT_THAT(RsaSsaPkcs1SignKeyManager().get_version(), Eq(0));
  EXPECT_THAT(RsaSsaPkcs1SignKeyManager().key_material_type(),
              Eq(KeyData::ASYMMETRIC_PRIVATE));
  EXPECT_THAT(
      RsaSsaPkcs1SignKeyManager().get_key_type(),
      Eq("type.googleapis.com/google.crypto.tink.RsaSsaPkcs1PrivateKey"));
}

RsaSsaPkcs1KeyFormat CreateKeyFormat(HashType hash_type,
                                     int modulus_size_in_bits,
                                     int public_exponent) {
  RsaSsaPkcs1KeyFormat key_format;
  auto params = key_format.mutable_params();
  params->set_hash_type(hash_type);
  key_format.set_modulus_size_in_bits(modulus_size_in_bits);
  bssl::UniquePtr<BIGNUM> e(BN_new());
  BN_set_word(e.get(), public_exponent);
  key_format.set_public_exponent(
      subtle::SubtleUtilBoringSSL::bn2str(e.get(), BN_num_bytes(e.get()))
          .ValueOrDie());
  return key_format;
}

RsaSsaPkcs1KeyFormat ValidKeyFormat() {
  return CreateKeyFormat(HashType::SHA256, 3072, RSA_F4);
}

TEST(RsaSsaPkcs1SignKeyManagerTest, ValidateKeyFormat) {
  EXPECT_THAT(RsaSsaPkcs1SignKeyManager().ValidateKeyFormat(ValidKeyFormat()),
              IsOk());
}

TEST(RsaSsaPkcs1SignKeyManagerTest, ValidateKeyFormatSha384Allowed) {
  RsaSsaPkcs1KeyFormat key_format = ValidKeyFormat();
  key_format.mutable_params()->set_hash_type(HashType::SHA384);
  EXPECT_THAT(RsaSsaPkcs1SignKeyManager().ValidateKeyFormat(key_format),
              IsOk());
}

TEST(RsaSsaPkcs1SignKeyManagerTest, ValidateKeyFormatSha512Allowed) {
  RsaSsaPkcs1KeyFormat key_format = ValidKeyFormat();
  key_format.mutable_params()->set_hash_type(HashType::SHA512);
  EXPECT_THAT(RsaSsaPkcs1SignKeyManager().ValidateKeyFormat(key_format),
              IsOk());
}

TEST(RsaSsaPkcs1SignKeyManagerTest, ValidateKeyFormatSha1Disallowed) {
  RsaSsaPkcs1KeyFormat key_format = ValidKeyFormat();
  key_format.mutable_params()->set_hash_type(HashType::SHA1);
  EXPECT_THAT(RsaSsaPkcs1SignKeyManager().ValidateKeyFormat(key_format),
              Not(IsOk()));
}

TEST(RsaSsaPkcs1SignKeyManagerTest, ValidateKeyFormatUnkownHashDisallowed) {
  RsaSsaPkcs1KeyFormat key_format = ValidKeyFormat();
  key_format.mutable_params()->set_hash_type(HashType::UNKNOWN_HASH);
  EXPECT_THAT(RsaSsaPkcs1SignKeyManager().ValidateKeyFormat(key_format),
              Not(IsOk()));
}

TEST(RsaSsaPkcs1SignKeyManagerTest, ValidateKeyFormatSmallModulusDisallowed) {
  RsaSsaPkcs1KeyFormat key_format = ValidKeyFormat();
  key_format.set_modulus_size_in_bits(2047);
  EXPECT_THAT(RsaSsaPkcs1SignKeyManager().ValidateKeyFormat(key_format),
              Not(IsOk()));
}

// Checks whether given key is compatible with the given format.
void CheckNewKey(const RsaSsaPkcs1PrivateKey& private_key,
                 const RsaSsaPkcs1KeyFormat& key_format) {
  RsaSsaPkcs1SignKeyManager key_manager;
  RsaSsaPkcs1PublicKey public_key = private_key.public_key();
  EXPECT_EQ(0, private_key.version());
  EXPECT_TRUE(private_key.has_public_key());
  EXPECT_EQ(0, public_key.version());
  EXPECT_GT(public_key.n().length(), 0);
  EXPECT_GT(public_key.e().length(), 0);
  EXPECT_EQ(public_key.params().SerializeAsString(),
            key_format.params().SerializeAsString());
  EXPECT_EQ(key_format.public_exponent(), public_key.e());
  auto n = std::move(SubtleUtilBoringSSL::str2bn(public_key.n()).ValueOrDie());
  auto d = std::move(SubtleUtilBoringSSL::str2bn(private_key.d()).ValueOrDie());
  auto p = std::move(SubtleUtilBoringSSL::str2bn(private_key.p()).ValueOrDie());
  auto q = std::move(SubtleUtilBoringSSL::str2bn(private_key.q()).ValueOrDie());
  auto dp =
      std::move(SubtleUtilBoringSSL::str2bn(private_key.dp()).ValueOrDie());
  auto dq =
      std::move(SubtleUtilBoringSSL::str2bn(private_key.dq()).ValueOrDie());
  bssl::UniquePtr<BN_CTX> ctx(BN_CTX_new());

  // Check n = p * q.
  auto n_calc = bssl::UniquePtr<BIGNUM>(BN_new());
  EXPECT_TRUE(BN_mul(n_calc.get(), p.get(), q.get(), ctx.get()));
  EXPECT_TRUE(BN_equal_consttime(n_calc.get(), n.get()));

  // Check n size >= modulus_size_in_bits bit.
  EXPECT_GE(BN_num_bits(n.get()), key_format.modulus_size_in_bits());

  // dp = d mod (p - 1)
  auto pm1 = bssl::UniquePtr<BIGNUM>(BN_dup(p.get()));
  EXPECT_TRUE(BN_sub_word(pm1.get(), 1));
  auto dp_calc = bssl::UniquePtr<BIGNUM>(BN_new());
  EXPECT_TRUE(BN_mod(dp_calc.get(), d.get(), pm1.get(), ctx.get()));
  EXPECT_TRUE(BN_equal_consttime(dp_calc.get(), dp.get()));

  // dq = d mod (q - 1)
  auto qm1 = bssl::UniquePtr<BIGNUM>(BN_dup(q.get()));
  EXPECT_TRUE(BN_sub_word(qm1.get(), 1));
  auto dq_calc = bssl::UniquePtr<BIGNUM>(BN_new());
  EXPECT_TRUE(BN_mod(dq_calc.get(), d.get(), qm1.get(), ctx.get()));

  EXPECT_TRUE(BN_equal_consttime(dq_calc.get(), dq.get()));
}

TEST(RsaSsaPkcs1SignKeyManagerTest, CreateKey) {
  RsaSsaPkcs1KeyFormat key_format =
      CreateKeyFormat(HashType::SHA256, 3072, RSA_F4);
  StatusOr<RsaSsaPkcs1PrivateKey> private_key_or =
      RsaSsaPkcs1SignKeyManager().CreateKey(key_format);
  ASSERT_THAT(private_key_or.status(), IsOk());
  CheckNewKey(private_key_or.ValueOrDie(), key_format);
}

TEST(RsaSsaPkcs1SignKeyManagerTest, CreateKeySmallKey) {
  RsaSsaPkcs1KeyFormat key_format =
      CreateKeyFormat(HashType::SHA256, 2048, RSA_F4);

  StatusOr<RsaSsaPkcs1PrivateKey> private_key_or =
      RsaSsaPkcs1SignKeyManager().CreateKey(key_format);
  ASSERT_THAT(private_key_or.status(), IsOk());
  CheckNewKey(private_key_or.ValueOrDie(), key_format);
}

TEST(RsaSsaPkcs1SignKeyManagerTest, CreateKeyLargeKey) {
  RsaSsaPkcs1KeyFormat key_format =
      CreateKeyFormat(HashType::SHA512, 4096, RSA_F4);

  StatusOr<RsaSsaPkcs1PrivateKey> private_key_or =
      RsaSsaPkcs1SignKeyManager().CreateKey(key_format);
  ASSERT_THAT(private_key_or.status(), IsOk());
  CheckNewKey(private_key_or.ValueOrDie(), key_format);
}

TEST(RsaSsaPkcs1SignKeyManagerTest, CreateKeyValid) {
  StatusOr<RsaSsaPkcs1PrivateKey> key_or =
      RsaSsaPkcs1SignKeyManager().CreateKey(ValidKeyFormat());
  ASSERT_THAT(key_or.status(), IsOk());
  EXPECT_THAT(RsaSsaPkcs1SignKeyManager().ValidateKey(key_or.ValueOrDie()),
              IsOk());
}

// Check that in a bunch of CreateKey calls all generated primes are distinct.
TEST(RsaSsaPkcs1SignKeyManagerTest, CreateKeyAlwaysNewRsaPair) {
  absl::flat_hash_set<std::string> keys;
  // This test takes about a second per key.
  int num_generated_keys = 5;
  for (int i = 0; i < num_generated_keys; ++i) {
    StatusOr<RsaSsaPkcs1PrivateKey> key_or =
        RsaSsaPkcs1SignKeyManager().CreateKey(ValidKeyFormat());
    ASSERT_THAT(key_or.status(), IsOk());
    keys.insert(key_or.ValueOrDie().p());
    keys.insert(key_or.ValueOrDie().q());
  }
  EXPECT_THAT(keys, SizeIs(2 * num_generated_keys));
}

TEST(RsaSsaPkcs1SignKeyManagerTest, GetPublicKey) {
  StatusOr<RsaSsaPkcs1PrivateKey> key_or =
      RsaSsaPkcs1SignKeyManager().CreateKey(ValidKeyFormat());
  ASSERT_THAT(key_or.status(), IsOk());
  StatusOr<RsaSsaPkcs1PublicKey> public_key_or =
      RsaSsaPkcs1SignKeyManager().GetPublicKey(key_or.ValueOrDie());
  ASSERT_THAT(public_key_or.status(), IsOk());
  EXPECT_THAT(public_key_or.ValueOrDie().version(),
              Eq(key_or.ValueOrDie().public_key().version()));
  EXPECT_THAT(public_key_or.ValueOrDie().params().hash_type(),
              Eq(key_or.ValueOrDie().public_key().params().hash_type()));
  EXPECT_THAT(public_key_or.ValueOrDie().n(),
              Eq(key_or.ValueOrDie().public_key().n()));
  EXPECT_THAT(public_key_or.ValueOrDie().e(),
              Eq(key_or.ValueOrDie().public_key().e()));
}

TEST(EcdsaSignKeyManagerTest, Create) {
  RsaSsaPkcs1KeyFormat key_format =
      CreateKeyFormat(HashType::SHA256, 3072, RSA_F4);
  StatusOr<RsaSsaPkcs1PrivateKey> key_or =
      RsaSsaPkcs1SignKeyManager().CreateKey(key_format);
  ASSERT_THAT(key_or.status(), IsOk());
  RsaSsaPkcs1PrivateKey key = key_or.ValueOrDie();

  auto signer_or = RsaSsaPkcs1SignKeyManager().GetPrimitive<PublicKeySign>(key);
  ASSERT_THAT(signer_or.status(), IsOk());

  auto direct_verifier_or = subtle::RsaSsaPkcs1VerifyBoringSsl::New(
      {key.public_key().n(), key.public_key().e()}, {subtle::HashType::SHA256});
  ASSERT_THAT(direct_verifier_or.status(), IsOk());

  std::string message = "Some message";
  EXPECT_THAT(direct_verifier_or.ValueOrDie()->Verify(
                  signer_or.ValueOrDie()->Sign(message).ValueOrDie(), message),
              IsOk());
}

}  // namespace
}  // namespace tink
}  // namespace crypto


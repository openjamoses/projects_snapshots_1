// Copyright 2017 Google Inc.
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

#include "cc/hybrid/hybrid_decrypt_factory.h"

#include "cc/hybrid_decrypt.h"
#include "cc/hybrid_encrypt.h"
#include "cc/crypto_format.h"
#include "cc/keyset_handle.h"
#include "cc/hybrid/ecies_aead_hkdf_public_key_manager.h"
#include "cc/hybrid/hybrid_decrypt_config.h"
#include "cc/util/ptr_util.h"
#include "cc/util/status.h"
#include "cc/util/test_util.h"
#include "gtest/gtest.h"
#include "proto/ecies_aead_hkdf.pb.h"
#include "proto/tink.pb.h"

using crypto::tink::test::AddRawKey;
using crypto::tink::test::AddTinkKey;
using google::crypto::tink::EciesAeadHkdfKeyFormat;
using google::crypto::tink::EciesAeadHkdfPrivateKey;
using google::crypto::tink::EcPointFormat;
using google::crypto::tink::EllipticCurveType;
using google::crypto::tink::HashType;
using google::crypto::tink::KeyData;
using google::crypto::tink::Keyset;
using google::crypto::tink::KeyStatusType;
using google::crypto::tink::KeyTemplate;

namespace crypto {
namespace tink {
namespace {

class HybridDecryptFactoryTest : public ::testing::Test {
};

EciesAeadHkdfPrivateKey GetNewEciesPrivateKey() {
  return test::GetEciesAesGcmHkdfTestKey(
      EllipticCurveType::NIST_P256, EcPointFormat::UNCOMPRESSED,
      HashType::SHA256, 24);
}

TEST_F(HybridDecryptFactoryTest, testBasic) {
  Keyset keyset;
  KeysetHandle keyset_handle(keyset);
  auto hybrid_decrypt_result =
      HybridDecryptFactory::GetPrimitive(keyset_handle);
  EXPECT_FALSE(hybrid_decrypt_result.ok());
  EXPECT_EQ(util::error::INVALID_ARGUMENT,
      hybrid_decrypt_result.status().error_code());
  EXPECT_PRED_FORMAT2(testing::IsSubstring, "at least one key",
      hybrid_decrypt_result.status().error_message());
}

TEST_F(HybridDecryptFactoryTest, testPrimitive) {
  // Prepare a Keyset.
  Keyset keyset;
  std::string key_type =
      "type.googleapis.com/google.crypto.tink.EciesAeadHkdfPrivateKey";

  uint32_t key_id_1 = 1234543;
  auto ecies_key_1 = GetNewEciesPrivateKey();
  AddTinkKey(key_type, key_id_1, ecies_key_1, KeyStatusType::ENABLED,
             KeyData::ASYMMETRIC_PRIVATE, &keyset);

  uint32_t key_id_2 = 726329;
  auto ecies_key_2 = GetNewEciesPrivateKey();
  AddRawKey(key_type, key_id_2, ecies_key_2, KeyStatusType::ENABLED,
            KeyData::ASYMMETRIC_PRIVATE, &keyset);

  uint32_t key_id_3 = 7213743;
  auto ecies_key_3 = GetNewEciesPrivateKey();
  AddTinkKey(key_type, key_id_3, ecies_key_3, KeyStatusType::ENABLED,
             KeyData::ASYMMETRIC_PRIVATE, &keyset);

  keyset.set_primary_key_id(key_id_3);

  // Initialize the registry.
  ASSERT_TRUE(HybridDecryptConfig::RegisterStandardKeyTypes().ok());;

  // Prepare HybridEncrypt-instances.
  auto ecies_key_manager = util::make_unique<EciesAeadHkdfPublicKeyManager>();
  std::unique_ptr<HybridEncrypt> ecies_1 = std::move(
      ecies_key_manager->GetPrimitive(ecies_key_1.public_key()).ValueOrDie());
  std::unique_ptr<HybridEncrypt> ecies_2 = std::move(
      ecies_key_manager->GetPrimitive(ecies_key_2.public_key()).ValueOrDie());

  // Create a KeysetHandle and use it with the factory.
  KeysetHandle keyset_handle(keyset);
  auto hybrid_decrypt_result =
      HybridDecryptFactory::GetPrimitive(keyset_handle);
  EXPECT_TRUE(hybrid_decrypt_result.ok()) << hybrid_decrypt_result.status();
  auto hybrid_decrypt = std::move(hybrid_decrypt_result.ValueOrDie());

  // Test the resulting HybridDecrypt-instance.
  std::string plaintext = "some plaintext";
  std::string context_info = "some context info";
  auto ciphertext_1 =
      CryptoFormat::get_output_prefix(keyset.key(0)).ValueOrDie() +
      ecies_1->Encrypt(plaintext, context_info).ValueOrDie();
  auto ciphertext_2 =
      CryptoFormat::get_output_prefix(keyset.key(1)).ValueOrDie() +
      ecies_2->Encrypt(plaintext, context_info).ValueOrDie();

  {  // Regular decryption with key_1.
    auto decrypt_result = hybrid_decrypt->Decrypt(ciphertext_1, context_info);
    EXPECT_TRUE(decrypt_result.ok()) << decrypt_result.status();
    EXPECT_EQ(plaintext, decrypt_result.ValueOrDie());
  }

  {  // Regular decryption with key_2.
    auto decrypt_result = hybrid_decrypt->Decrypt(ciphertext_2, context_info);
    EXPECT_TRUE(decrypt_result.ok()) << decrypt_result.status();
    EXPECT_EQ(plaintext, decrypt_result.ValueOrDie());
  }

  {  // Wrong context_info.
    auto decrypt_result = hybrid_decrypt->Decrypt(ciphertext_1, "bad context");
    EXPECT_FALSE(decrypt_result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT,
              decrypt_result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "decryption failed",
                        decrypt_result.status().error_message());
  }
}

}  // namespace
}  // namespace tink
}  // namespace crypto


int main(int ac, char* av[]) {
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}

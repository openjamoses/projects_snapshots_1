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

#include "cc/hybrid/hybrid_decrypt_set_wrapper.h"
#include "cc/hybrid_decrypt.h"
#include "cc/primitive_set.h"
#include "cc/util/status.h"
#include "cc/util/test_util.h"
#include "gtest/gtest.h"

using crypto::tink::test::DummyHybridDecrypt;
using google::crypto::tink::OutputPrefixType;
using google::crypto::tink::Keyset;

namespace crypto {
namespace tink {
namespace {

class HybridDecryptSetWrapperTest : public ::testing::Test {
 protected:
  void SetUp() override {
  }
  void TearDown() override {
  }
};

TEST_F(HybridDecryptSetWrapperTest, testBasic) {
  { // hybrid_decrypt_set is nullptr.
    auto hybrid_decrypt_result =
        HybridDecryptSetWrapper::NewHybridDecrypt(nullptr);
    EXPECT_FALSE(hybrid_decrypt_result.ok());
    EXPECT_EQ(util::error::INTERNAL,
        hybrid_decrypt_result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "non-NULL",
        hybrid_decrypt_result.status().error_message());
  }

  { // hybrid_decrypt_set has no primary primitive.
    std::unique_ptr<PrimitiveSet<HybridDecrypt>>
        hybrid_decrypt_set(new PrimitiveSet<HybridDecrypt>());
    auto hybrid_decrypt_result = HybridDecryptSetWrapper::NewHybridDecrypt(
        std::move(hybrid_decrypt_set));
    EXPECT_FALSE(hybrid_decrypt_result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT,
        hybrid_decrypt_result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "no primary",
        hybrid_decrypt_result.status().error_message());
  }

  { // Correct hybrid_decrypt_set;
    Keyset::Key* key;
    Keyset keyset;

    uint32_t key_id_0 = 1234543;
    key = keyset.add_key();
    key->set_output_prefix_type(OutputPrefixType::RAW);
    key->set_key_id(key_id_0);

    uint32_t key_id_1 = 726329;
    key = keyset.add_key();
    key->set_output_prefix_type(OutputPrefixType::LEGACY);
    key->set_key_id(key_id_1);

    uint32_t key_id_2 = 7213743;
    key = keyset.add_key();
    key->set_output_prefix_type(OutputPrefixType::TINK);
    key->set_key_id(key_id_2);

    std::string hybrid_name_0 = "hybrid_0";
    std::string hybrid_name_1 = "hybrid_1";
    std::string hybrid_name_2 = "hybrid_2";
    std::unique_ptr<PrimitiveSet<HybridDecrypt>> hybrid_decrypt_set(
        new PrimitiveSet<HybridDecrypt>());
    std::unique_ptr<HybridDecrypt> hybrid_decrypt(
        new DummyHybridDecrypt(hybrid_name_0));
    auto entry_result = hybrid_decrypt_set->AddPrimitive(
        std::move(hybrid_decrypt), keyset.key(0));
    ASSERT_TRUE(entry_result.ok());
    hybrid_decrypt.reset(new DummyHybridDecrypt(hybrid_name_1));
    entry_result = hybrid_decrypt_set->AddPrimitive(
        std::move(hybrid_decrypt), keyset.key(1));
    ASSERT_TRUE(entry_result.ok());
    std::string prefix_id_1 = entry_result.ValueOrDie()->get_identifier();
    hybrid_decrypt.reset(new DummyHybridDecrypt(hybrid_name_2));
    entry_result = hybrid_decrypt_set->AddPrimitive(
        std::move(hybrid_decrypt), keyset.key(2));
    ASSERT_TRUE(entry_result.ok());
    // The last key is the primary.
    hybrid_decrypt_set->set_primary(entry_result.ValueOrDie());

    // Wrap hybrid_decrypt_set and test the resulting HybridDecrypt.
    auto hybrid_decrypt_result = HybridDecryptSetWrapper::NewHybridDecrypt(
        std::move(hybrid_decrypt_set));
    EXPECT_TRUE(hybrid_decrypt_result.ok()) << hybrid_decrypt_result.status();
    hybrid_decrypt = std::move(hybrid_decrypt_result.ValueOrDie());
    std::string plaintext = "some_plaintext";
    std::string context_info = "some_context";

    {  // RAW key
      std::string ciphertext = plaintext + hybrid_name_0;
      auto decrypt_result = hybrid_decrypt->Decrypt(ciphertext, context_info);
      EXPECT_TRUE(decrypt_result.ok()) << decrypt_result.status();
      EXPECT_EQ(plaintext, decrypt_result.ValueOrDie());
    }

    {  // No ciphertext prefix.
      std::string ciphertext =  plaintext + hybrid_name_1;
      auto decrypt_result = hybrid_decrypt->Decrypt(ciphertext, context_info);
      EXPECT_FALSE(decrypt_result.ok());
      EXPECT_EQ(util::error::INVALID_ARGUMENT,
          decrypt_result.status().error_code());
      EXPECT_PRED_FORMAT2(testing::IsSubstring, "decryption failed",
          decrypt_result.status().error_message());
    }

    {  // Correct ciphertext prefix.
      std::string ciphertext = prefix_id_1 + plaintext + hybrid_name_1;
      auto decrypt_result = hybrid_decrypt->Decrypt(ciphertext, context_info);
      EXPECT_TRUE(decrypt_result.ok()) << decrypt_result.status();
      EXPECT_EQ(plaintext, decrypt_result.ValueOrDie());
    }

    {  // Bad ciphertext.
      std::string ciphertext = "some bad ciphertext";
      auto decrypt_result = hybrid_decrypt->Decrypt(ciphertext, context_info);
      EXPECT_FALSE(decrypt_result.ok());
      EXPECT_EQ(util::error::INVALID_ARGUMENT,
          decrypt_result.status().error_code());
      EXPECT_PRED_FORMAT2(testing::IsSubstring, "decryption failed",
          decrypt_result.status().error_message());
    }
  }
}

}  // namespace
}  // namespace tink
}  // namespace crypto


int main(int ac, char* av[]) {
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}

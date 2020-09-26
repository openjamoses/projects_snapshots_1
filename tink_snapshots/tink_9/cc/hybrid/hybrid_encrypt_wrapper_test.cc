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

#include "tink/hybrid/hybrid_encrypt_wrapper.h"
#include "tink/hybrid_encrypt.h"
#include "tink/primitive_set.h"
#include "tink/util/status.h"
#include "tink/util/test_util.h"
#include "gtest/gtest.h"

using crypto::tink::test::DummyHybridEncrypt;
using google::crypto::tink::Keyset;
using google::crypto::tink::KeyStatusType;
using google::crypto::tink::OutputPrefixType;

namespace crypto {
namespace tink {
namespace {

class HybridEncryptSetWrapperTest : public ::testing::Test {
 protected:
  void SetUp() override {
  }
  void TearDown() override {
  }
};

TEST_F(HybridEncryptSetWrapperTest, testBasic) {
  { // hybrid_encrypt_set is nullptr.
    auto hybrid_encrypt_result =
        HybridEncryptWrapper().Wrap(nullptr);
    EXPECT_FALSE(hybrid_encrypt_result.ok());
    EXPECT_EQ(util::error::INTERNAL,
        hybrid_encrypt_result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "non-NULL",
        hybrid_encrypt_result.status().error_message());
  }

  { // hybrid_encrypt_set has no primary primitive.
    std::unique_ptr<PrimitiveSet<HybridEncrypt>>
        hybrid_encrypt_set(new PrimitiveSet<HybridEncrypt>());
    auto hybrid_encrypt_result = HybridEncryptWrapper().Wrap(
        std::move(hybrid_encrypt_set));
    EXPECT_FALSE(hybrid_encrypt_result.ok());
    EXPECT_EQ(util::error::INVALID_ARGUMENT,
        hybrid_encrypt_result.status().error_code());
    EXPECT_PRED_FORMAT2(testing::IsSubstring, "no primary",
        hybrid_encrypt_result.status().error_message());
  }

  { // Correct hybrid_encrypt_set;
    Keyset::Key* key;
    Keyset keyset;

    uint32_t key_id_0 = 1234543;
    key = keyset.add_key();
    key->set_output_prefix_type(OutputPrefixType::TINK);
    key->set_key_id(key_id_0);
    key->set_status(KeyStatusType::ENABLED);

    uint32_t key_id_1 = 726329;
    key = keyset.add_key();
    key->set_output_prefix_type(OutputPrefixType::LEGACY);
    key->set_key_id(key_id_1);
    key->set_status(KeyStatusType::ENABLED);

    uint32_t key_id_2 = 7213743;
    key = keyset.add_key();
    key->set_output_prefix_type(OutputPrefixType::TINK);
    key->set_key_id(key_id_2);
    key->set_status(KeyStatusType::ENABLED);

    std::string hybrid_name_0 = "hybrid_0";
    std::string hybrid_name_1 = "hybrid_1";
    std::string hybrid_name_2 = "hybrid_2";
    std::unique_ptr<PrimitiveSet<HybridEncrypt>> hybrid_encrypt_set(
        new PrimitiveSet<HybridEncrypt>());
    std::unique_ptr<HybridEncrypt> hybrid_encrypt(
        new DummyHybridEncrypt(hybrid_name_0));
    auto entry_result = hybrid_encrypt_set->AddPrimitive(
        std::move(hybrid_encrypt), keyset.key(0));
    ASSERT_TRUE(entry_result.ok());
    hybrid_encrypt.reset(new DummyHybridEncrypt(hybrid_name_1));
    entry_result = hybrid_encrypt_set->AddPrimitive(
        std::move(hybrid_encrypt), keyset.key(1));
    ASSERT_TRUE(entry_result.ok());
    hybrid_encrypt.reset(new DummyHybridEncrypt(hybrid_name_2));
    entry_result = hybrid_encrypt_set->AddPrimitive(
        std::move(hybrid_encrypt), keyset.key(2));
    ASSERT_TRUE(entry_result.ok());
    // The last key is the primary.
    hybrid_encrypt_set->set_primary(entry_result.ValueOrDie());

    // Wrap hybrid_encrypt_set and test the resulting HybridEncrypt.
    auto hybrid_encrypt_result = HybridEncryptWrapper().Wrap(
        std::move(hybrid_encrypt_set));
    EXPECT_TRUE(hybrid_encrypt_result.ok()) << hybrid_encrypt_result.status();
    hybrid_encrypt = std::move(hybrid_encrypt_result.ValueOrDie());
    std::string plaintext = "some_plaintext";
    std::string context_info = "some_context";

    auto encrypt_result = hybrid_encrypt->Encrypt(plaintext, context_info);
    EXPECT_TRUE(encrypt_result.ok()) << encrypt_result.status();
    std::string ciphertext = encrypt_result.ValueOrDie();
    EXPECT_PRED_FORMAT2(testing::IsSubstring,
        hybrid_name_2, ciphertext);
  }
}

}  // namespace
}  // namespace tink
}  // namespace crypto

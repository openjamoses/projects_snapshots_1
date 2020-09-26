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

#include "tink/subtle/ecies_hkdf_sender_kem_boringssl.h"

#include <iostream>

#include "gtest/gtest.h"
#include "tink/subtle/common_enums.h"
#include "tink/subtle/ecies_hkdf_recipient_kem_boringssl.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/secret_data.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "tink/util/test_util.h"

// TODO(quannguyen): Add extensive tests.
// It's important to test compatibility with Java.
namespace crypto {
namespace tink {
namespace subtle {
namespace {

class EciesHkdfSenderKemBoringSslTest : public ::testing::Test {};

struct TestVector {
  EllipticCurveType curve;
  HashType hash;
  EcPointFormat point_format;
  std::string salt_hex;
  std::string info_hex;
  int out_len;
};

static const std::vector<TestVector> test_vector(
    {{
         EllipticCurveType::NIST_P256,
         HashType::SHA256,
         EcPointFormat::UNCOMPRESSED,
         "0b0b0b0b",
         "0b0b0b0b0b0b0b0b",
         32,
     },
     {
         EllipticCurveType::NIST_P256,
         HashType::SHA256,
         EcPointFormat::COMPRESSED,
         "0b0b0b0b",
         "0b0b0b0b0b0b0b0b",
         32,
     },
     {
         EllipticCurveType::CURVE25519,
         HashType::SHA256,
         EcPointFormat::COMPRESSED,
         "0b0b0b0b",
         "0b0b0b0b0b0b0b0b",
         32,
     }});

TEST_F(EciesHkdfSenderKemBoringSslTest, testSenderRecipientBasic) {
  for (const TestVector& test : test_vector) {
    auto status_or_test_key = SubtleUtilBoringSSL::GetNewEcKey(test.curve);
    ASSERT_TRUE(status_or_test_key.ok());
    auto test_key = status_or_test_key.ValueOrDie();
    auto status_or_sender_kem = EciesHkdfSenderKemBoringSsl::New(
        test.curve, test_key.pub_x, test_key.pub_y);
    ASSERT_TRUE(status_or_sender_kem.ok());
    auto sender_kem = std::move(status_or_sender_kem.ValueOrDie());
    auto status_or_kem_key = sender_kem->GenerateKey(
        test.hash, test::HexDecodeOrDie(test.salt_hex),
        test::HexDecodeOrDie(test.info_hex), test.out_len, test.point_format);
    ASSERT_TRUE(status_or_kem_key.ok());
    auto kem_key = std::move(status_or_kem_key.ValueOrDie());
    auto ecies_recipient(
        std::move(EciesHkdfRecipientKemBoringSsl::New(
                      test.curve, util::SecretDataFromStringView(test_key.priv))
                      .ValueOrDie()));
    auto status_or_shared_secret = ecies_recipient->GenerateKey(
        kem_key->get_kem_bytes(), test.hash,
        test::HexDecodeOrDie(test.salt_hex),
        test::HexDecodeOrDie(test.info_hex),
        test.out_len, test.point_format);
    std::cout << test::HexEncode(kem_key->get_kem_bytes()) << std::endl;
    EXPECT_EQ(test::HexEncode(
                  util::SecretDataAsStringView(kem_key->get_symmetric_key())),
              test::HexEncode(util::SecretDataAsStringView(
                  status_or_shared_secret.ValueOrDie())));
  }
}

TEST_F(EciesHkdfSenderKemBoringSslTest, testNewUnknownCurve) {
  auto status_or_sender_kem = EciesHkdfSenderKemBoringSsl::New(
      EllipticCurveType::UNKNOWN_CURVE, "", "");
  EXPECT_EQ(util::error::UNIMPLEMENTED,
            status_or_sender_kem.status().error_code());
}

class EciesHkdfNistPCurveSendKemBoringSslTest : public ::testing::Test {};

TEST_F(EciesHkdfNistPCurveSendKemBoringSslTest, testNew) {
  EllipticCurveType curve = EllipticCurveType::NIST_P256;
  auto status_or_test_key = SubtleUtilBoringSSL::GetNewEcKey(curve);
  ASSERT_TRUE(status_or_test_key.ok());
  auto test_key = status_or_test_key.ValueOrDie();
  auto status_or_sender_kem = EciesHkdfNistPCurveSendKemBoringSsl::New(
      curve, test_key.pub_x, test_key.pub_y);
  ASSERT_TRUE(status_or_sender_kem.ok());
}

TEST_F(EciesHkdfNistPCurveSendKemBoringSslTest, testNewInvalidCurve) {
  EllipticCurveType curve = EllipticCurveType::NIST_P256;
  auto status_or_test_key = SubtleUtilBoringSSL::GetNewEcKey(curve);
  ASSERT_TRUE(status_or_test_key.ok());
  auto test_key = status_or_test_key.ValueOrDie();
  auto status_or_sender_kem = EciesHkdfNistPCurveSendKemBoringSsl::New(
      EllipticCurveType::CURVE25519, test_key.pub_x, test_key.pub_y);
  EXPECT_EQ(status_or_sender_kem.status().error_code(),
            util::error::UNIMPLEMENTED);
}

TEST_F(EciesHkdfNistPCurveSendKemBoringSslTest, testGenerateKey) {
  EllipticCurveType curve = EllipticCurveType::NIST_P256;
  auto status_or_test_key = SubtleUtilBoringSSL::GetNewEcKey(curve);
  ASSERT_TRUE(status_or_test_key.ok());
  auto test_key = status_or_test_key.ValueOrDie();
  auto status_or_sender_kem = EciesHkdfNistPCurveSendKemBoringSsl::New(
      curve, test_key.pub_x, test_key.pub_y);
  ASSERT_TRUE(status_or_sender_kem.ok());
  auto sender_kem = std::move(status_or_sender_kem.ValueOrDie());

  uint32_t key_size_in_bytes = 128;
  auto status_or_kem_key =
      sender_kem->GenerateKey(HashType::SHA256, "hkdf_salt", "hkdf_info",
                              key_size_in_bytes, EcPointFormat::COMPRESSED);
  ASSERT_TRUE(status_or_kem_key.ok());
  auto kem_key = std::move(status_or_kem_key.ValueOrDie());
  EXPECT_FALSE(kem_key->get_kem_bytes().empty());
  EXPECT_EQ(kem_key->get_symmetric_key().size(), key_size_in_bytes);
}

class EciesHkdfX25519SendKemBoringSslTest : public ::testing::Test {};

TEST_F(EciesHkdfX25519SendKemBoringSslTest, testNew) {
  EllipticCurveType curve = EllipticCurveType::CURVE25519;
  auto status_or_test_key = SubtleUtilBoringSSL::GetNewEcKey(curve);
  ASSERT_TRUE(status_or_test_key.ok());
  auto test_key = status_or_test_key.ValueOrDie();
  auto status_or_sender_kem = EciesHkdfX25519SendKemBoringSsl::New(
      curve, test_key.pub_x, test_key.pub_y);
  ASSERT_TRUE(status_or_sender_kem.ok());
}

TEST_F(EciesHkdfX25519SendKemBoringSslTest, testNewInvalidCurve) {
  EllipticCurveType curve = EllipticCurveType::CURVE25519;
  auto status_or_test_key = SubtleUtilBoringSSL::GetNewEcKey(curve);
  ASSERT_TRUE(status_or_test_key.ok());
  auto test_key = status_or_test_key.ValueOrDie();
  auto status_or_sender_kem = EciesHkdfX25519SendKemBoringSsl::New(
      EllipticCurveType::NIST_P256, test_key.pub_x, test_key.pub_y);
  EXPECT_EQ(status_or_sender_kem.status().error_code(),
            util::error::INVALID_ARGUMENT);
}

TEST_F(EciesHkdfX25519SendKemBoringSslTest, testNewPubxTooLong) {
  EllipticCurveType curve = EllipticCurveType::CURVE25519;
  auto status_or_test_key = SubtleUtilBoringSSL::GetNewEcKey(curve);
  ASSERT_TRUE(status_or_test_key.ok());
  auto test_key = status_or_test_key.ValueOrDie();
  test_key.pub_x.resize(test_key.pub_x.size() / 2);
  auto status_or_sender_kem = EciesHkdfX25519SendKemBoringSsl::New(
      curve, test_key.pub_x, test_key.pub_y);
  EXPECT_EQ(status_or_sender_kem.status().error_code(),
            util::error::INVALID_ARGUMENT);
}

TEST_F(EciesHkdfX25519SendKemBoringSslTest, testNewPubyNotEmpty) {
  EllipticCurveType curve = EllipticCurveType::CURVE25519;
  auto status_or_test_key = SubtleUtilBoringSSL::GetNewEcKey(curve);
  ASSERT_TRUE(status_or_test_key.ok());
  auto test_key = status_or_test_key.ValueOrDie();
  test_key.pub_y = test_key.pub_x;
  auto status_or_sender_kem = EciesHkdfX25519SendKemBoringSsl::New(
      curve, test_key.pub_x, test_key.pub_y);
  EXPECT_EQ(status_or_sender_kem.status().error_code(),
            util::error::INVALID_ARGUMENT);
}

TEST_F(EciesHkdfX25519SendKemBoringSslTest, testGenerateKey) {
  EllipticCurveType curve = EllipticCurveType::CURVE25519;
  auto status_or_test_key = SubtleUtilBoringSSL::GetNewEcKey(curve);
  ASSERT_TRUE(status_or_test_key.ok());
  auto test_key = status_or_test_key.ValueOrDie();
  auto status_or_sender_kem = EciesHkdfX25519SendKemBoringSsl::New(
      curve, test_key.pub_x, test_key.pub_y);
  ASSERT_TRUE(status_or_sender_kem.ok());
  auto sender_kem = std::move(status_or_sender_kem.ValueOrDie());

  uint32_t key_size_in_bytes = 128;
  auto status_or_kem_key =
      sender_kem->GenerateKey(HashType::SHA256, "hkdf_salt", "hkdf_info",
                              key_size_in_bytes, EcPointFormat::COMPRESSED);
  ASSERT_TRUE(status_or_kem_key.ok());
  auto kem_key = std::move(status_or_kem_key.ValueOrDie());
  EXPECT_EQ(kem_key->get_kem_bytes().size(), X25519_PUBLIC_VALUE_LEN);
  EXPECT_EQ(kem_key->get_symmetric_key().size(), key_size_in_bytes);
}

TEST_F(EciesHkdfX25519SendKemBoringSslTest, testGenerateKeyUncompressed) {
  EllipticCurveType curve = EllipticCurveType::CURVE25519;
  auto status_or_test_key = SubtleUtilBoringSSL::GetNewEcKey(curve);
  ASSERT_TRUE(status_or_test_key.ok());
  auto test_key = status_or_test_key.ValueOrDie();
  auto status_or_sender_kem = EciesHkdfX25519SendKemBoringSsl::New(
      curve, test_key.pub_x, test_key.pub_y);
  ASSERT_TRUE(status_or_sender_kem.ok());
  auto sender_kem = std::move(status_or_sender_kem.ValueOrDie());

  auto status_or_kem_key =
      sender_kem->GenerateKey(HashType::SHA256, "hkdf_salt", "hkdf_info", 32,
                              EcPointFormat::UNCOMPRESSED);
  EXPECT_EQ(status_or_kem_key.status().error_code(),
            util::error::INVALID_ARGUMENT);
}

}  // namespace
}  // namespace subtle
}  // namespace tink
}  // namespace crypto

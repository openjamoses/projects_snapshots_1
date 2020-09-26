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

#include "tink/subtle/ecies_hkdf_recipient_kem_boringssl.h"

#include "gtest/gtest.h"
#include "tink/subtle/common_enums.h"
#include "tink/util/secret_data.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "tink/util/test_util.h"

namespace crypto {
namespace tink {
namespace subtle {
namespace {

class EciesHkdfRecipientKemBoringSslTest : public ::testing::Test {};

struct TestVector {
  EllipticCurveType curve;
  HashType hash;
  EcPointFormat point_format;
  std::string pub_encoded_hex;
  std::string priv_hex;
  std::string salt_hex;
  std::string info_hex;
  int out_len;
  std::string out_key_hex;
};

static const char kSaltHex[] = "0b0b0b0b";
static const char kInfoHex[] = "0b0b0b0b0b0b0b0b";

static const char kNistP256PublicValueHex[] =
    "04700c48f77f56584c5cc632ca65640db91b6bacce3a4df6b42ce7cc838833d287db71e509"
    "e3fd9b060ddb20ba5c51dcc5948d46fbf640dfe0441782cab85fa4ac";
static const char kNistP256PrivateKeyHex[] =
    "7d7dc5f71eb29ddaf80d6214632eeae03d9058af1fb6d22ed80badb62bc1a534";
static const char kNistP256SharedKeyHex[] =
    "0f19c0f322fc0a4b73b32bac6a66baa274de261db38a57f11ee4896ede24dbba";

static const char kX25519PublicValueHex[] =
    "bef00c1a15e0601678ef4899a8506f751cd0c1f4d210a2852ac9d42151d0e160";
static const char kX25519PrivateKeyHex[] =
    "df4320cecfd87a5a928355241c9d0e491be499cedf7b2b70687193124039eb92";
static const char kX25519SharedKeyHex[] =
    "4c77c4d086e2d267052bad906f8c00092f8ea944fc1dc69eb2fe8bb29df400cc";

static const std::vector<TestVector> test_vector(
    {{EllipticCurveType::NIST_P256, HashType::SHA256,
      EcPointFormat::UNCOMPRESSED, kNistP256PublicValueHex,
      kNistP256PrivateKeyHex, kSaltHex, kInfoHex, 32, kNistP256SharedKeyHex},
     {EllipticCurveType::CURVE25519, HashType::SHA256,
      EcPointFormat::COMPRESSED, kX25519PublicValueHex, kX25519PrivateKeyHex,
      kSaltHex, kInfoHex, 32, kX25519SharedKeyHex}});

TEST_F(EciesHkdfRecipientKemBoringSslTest, testBasic) {
  for (const TestVector& test : test_vector) {
    auto ecies_kem_or = EciesHkdfRecipientKemBoringSsl::New(
        test.curve,
        util::SecretDataFromStringView(test::HexDecodeOrDie(test.priv_hex)));
    ASSERT_TRUE(ecies_kem_or.ok());
    auto ecies_kem = std::move(ecies_kem_or).ValueOrDie();
    auto kem_key_or = ecies_kem->GenerateKey(
        test::HexDecodeOrDie(test.pub_encoded_hex), test.hash,
        test::HexDecodeOrDie(test.salt_hex),
        test::HexDecodeOrDie(test.info_hex), test.out_len, test.point_format);
    ASSERT_TRUE(kem_key_or.ok());
    EXPECT_EQ(
        test.out_key_hex,
        test::HexEncode(util::SecretDataAsStringView(kem_key_or.ValueOrDie())));
  }
}

TEST_F(EciesHkdfRecipientKemBoringSslTest, testNewUnimplementedCurve) {
  auto status_or_recipient_kem =
      EciesHkdfRecipientKemBoringSsl::New(EllipticCurveType::UNKNOWN_CURVE, {});
  EXPECT_EQ(status_or_recipient_kem.status().error_code(),
            util::error::UNIMPLEMENTED);
}

class EciesHkdfNistPCurveRecipientKemBoringSslTest : public ::testing::Test {};

TEST_F(EciesHkdfNistPCurveRecipientKemBoringSslTest, testNew) {
  auto status_or_recipient_kem = EciesHkdfNistPCurveRecipientKemBoringSsl::New(
      EllipticCurveType::NIST_P256,
      util::SecretDataFromStringView(
          test::HexDecodeOrDie(kNistP256PrivateKeyHex)));
  ASSERT_TRUE(status_or_recipient_kem.ok());
}

TEST_F(EciesHkdfNistPCurveRecipientKemBoringSslTest, testNewInvalidCurve) {
  auto status_or_recipient_kem = EciesHkdfNistPCurveRecipientKemBoringSsl::New(
      EllipticCurveType::CURVE25519,
      util::SecretDataFromStringView(
          test::HexDecodeOrDie(kNistP256PrivateKeyHex)));
  EXPECT_EQ(status_or_recipient_kem.status().error_code(),
            util::error::UNIMPLEMENTED);
}

TEST_F(EciesHkdfNistPCurveRecipientKemBoringSslTest, testNewEmptyPrivateKey) {
  auto status_or_recipient_kem = EciesHkdfNistPCurveRecipientKemBoringSsl::New(
      EllipticCurveType::CURVE25519, {});
  EXPECT_EQ(status_or_recipient_kem.status().error_code(),
            util::error::INVALID_ARGUMENT);
}

TEST_F(EciesHkdfNistPCurveRecipientKemBoringSslTest, testGenerateKey) {
  auto status_or_recipient_kem = EciesHkdfNistPCurveRecipientKemBoringSsl::New(
      EllipticCurveType::NIST_P256,
      util::SecretDataFromStringView(
          test::HexDecodeOrDie(kNistP256PrivateKeyHex)));
  ASSERT_TRUE(status_or_recipient_kem.ok());
  auto recipient_kem = std::move(status_or_recipient_kem.ValueOrDie());

  auto status_or_shared_key = recipient_kem->GenerateKey(
      test::HexDecodeOrDie(kNistP256PublicValueHex), HashType::SHA256,
      test::HexDecodeOrDie(kSaltHex), test::HexDecodeOrDie(kInfoHex), 32,
      EcPointFormat::UNCOMPRESSED);
  ASSERT_TRUE(status_or_shared_key.ok());

  EXPECT_EQ(test::HexEncode(util::SecretDataAsStringView(
                status_or_shared_key.ValueOrDie())),
            kNistP256SharedKeyHex);
}

class EciesHkdfX25519RecipientKemBoringSslTest : public ::testing::Test {};

TEST_F(EciesHkdfX25519RecipientKemBoringSslTest, testNew) {
  auto status_or_recipient_kem = EciesHkdfX25519RecipientKemBoringSsl::New(
      EllipticCurveType::CURVE25519,
      util::SecretDataFromStringView(
          test::HexDecodeOrDie(kX25519PrivateKeyHex)));
  ASSERT_TRUE(status_or_recipient_kem.ok());
}

TEST_F(EciesHkdfX25519RecipientKemBoringSslTest, testNewInvalidCurve) {
  auto status_or_recipient_kem = EciesHkdfX25519RecipientKemBoringSsl::New(
      EllipticCurveType::NIST_P256,
      util::SecretDataFromStringView(
          test::HexDecodeOrDie(kX25519PrivateKeyHex)));
  EXPECT_EQ(status_or_recipient_kem.status().error_code(),
            util::error::INVALID_ARGUMENT);
}

TEST_F(EciesHkdfX25519RecipientKemBoringSslTest, testNewShortKey) {
  util::SecretData private_key = util::SecretDataFromStringView(
      test::HexDecodeOrDie(kX25519PrivateKeyHex));
  private_key.resize(private_key.size() / 2);
  auto status_or_recipient_kem = EciesHkdfX25519RecipientKemBoringSsl::New(
      EllipticCurveType::CURVE25519, private_key);
  EXPECT_EQ(status_or_recipient_kem.status().error_code(),
            util::error::INVALID_ARGUMENT);
}

}  // namespace
}  // namespace subtle
}  // namespace tink
}  // namespace crypto

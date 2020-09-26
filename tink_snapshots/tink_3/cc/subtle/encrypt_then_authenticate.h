// Copyright 2017 Google Inc.
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

#ifndef TINK_SUBTLE_ENCRYPT_THEN_AUTHENTICATE_H_
#define TINK_SUBTLE_ENCRYPT_THEN_AUTHENTICATE_H_

#include <memory>

#include "cc/aead.h"
#include "cc/mac.h"
#include "cc/subtle/ind_cpa_cipher.h"
#include "cc/util/status.h"
#include "cc/util/statusor.h"
#include "google/protobuf/stubs/stringpiece.h"

namespace crypto {
namespace tink {

// This primitive performs an encrypt-then-Mac operation on plaintext and
// additional authenticated data (aad). The Mac is computed over (aad ||
// ciphertext || size of aad). This implementation is based on
// http://tools.ietf.org/html/draft-mcgrew-aead-aes-cbc-hmac-sha2-05.
class EncryptThenAuthenticate : public Aead {
 public:
  static crypto::tink::util::StatusOr<std::unique_ptr<Aead>> New(
      std::unique_ptr<IndCpaCipher> ind_cpa_cipher, std::unique_ptr<Mac> mac,
      uint8_t tag_size);

  // Encrypts 'plaintext' with 'additional_data' as additional authenticated
  // data. The resulting ciphertext allows for checking authenticity and
  // integrity of additional data ({@code aad}), but does not guarantee its
  // secrecy.
  //
  // The plaintext is encrypted with an IndCpaCipher, then MAC is computed over
  // (additional_data || ciphertext || t) where t is additional_data's length
  // in bits represented as 64-bit bigendian unsigned integer. The final
  // ciphertext format is (ind-cpa ciphertext || mac).
  crypto::tink::util::StatusOr<std::string> Encrypt(
      google::protobuf::StringPiece plaintext,
      google::protobuf::StringPiece additional_data) const override;

  crypto::tink::util::StatusOr<std::string> Decrypt(
      google::protobuf::StringPiece ciphertext,
      google::protobuf::StringPiece additional_data) const override;

  virtual ~EncryptThenAuthenticate() {}

 private:
  static const int MIN_TAG_SIZE_IN_BYTES = 10;

  EncryptThenAuthenticate() {}
  EncryptThenAuthenticate(std::unique_ptr<IndCpaCipher> ind_cpa_cipher,
                          std::unique_ptr<Mac> mac, uint8_t tag_size)
      : ind_cpa_cipher_(std::move(ind_cpa_cipher)),
        mac_(std::move(mac)),
        tag_size_(tag_size) {}

  std::unique_ptr<IndCpaCipher> ind_cpa_cipher_;
  std::unique_ptr<Mac> mac_;
  uint8_t tag_size_;
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_SUBTLE_ENCRYPT_THEN_AUTHENTICATE_H_

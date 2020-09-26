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

#include "tink/subtle/aes_gcm_boringssl.h"

#include <utility>

#include "absl/memory/memory.h"
#include "openssl/aead.h"
#include "tink/config/tink_fips.h"
#include "tink/subtle/random.h"
#include "tink/subtle/subtle_util.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/status.h"

namespace crypto {
namespace tink {
namespace subtle {

util::StatusOr<std::unique_ptr<Aead>> AesGcmBoringSsl::New(
    const util::SecretData& key) {
  auto status = CheckFipsCompatibility<AesGcmBoringSsl>();
  if (!status.ok()) return status;

  const EVP_AEAD* aead =
      SubtleUtilBoringSSL::GetAesGcmAeadForKeySize(key.size());
  if (aead == nullptr) {
    return util::Status(util::error::INVALID_ARGUMENT, "invalid key size");
  }
  bssl::UniquePtr<EVP_AEAD_CTX> ctx(EVP_AEAD_CTX_new(
      aead, key.data(), key.size(), EVP_AEAD_DEFAULT_TAG_LENGTH));
  if (!ctx) {
    return util::Status(util::error::INTERNAL,
                        "could not initialize EVP_AEAD_CTX");
  }
  return {absl::WrapUnique(new AesGcmBoringSsl(std::move(ctx)))};
}

util::StatusOr<std::string> AesGcmBoringSsl::Encrypt(
    absl::string_view plaintext, absl::string_view additional_data) const {
  std::string result = Random::GetRandomBytes(kIvSizeInBytes);
  ResizeStringUninitialized(
      &result, kIvSizeInBytes + plaintext.size() + kTagSizeInBytes);
  size_t len;
  if (EVP_AEAD_CTX_seal(
          ctx_.get(), reinterpret_cast<uint8_t*>(&result[kIvSizeInBytes]), &len,
          plaintext.size() + kTagSizeInBytes,
          reinterpret_cast<const uint8_t*>(&result[0]), kIvSizeInBytes,
          reinterpret_cast<const uint8_t*>(plaintext.data()), plaintext.size(),
          reinterpret_cast<const uint8_t*>(additional_data.data()),
          additional_data.size()) != 1) {
    return util::Status(util::error::INTERNAL, "Encryption failed");
  }
  return result;
}

util::StatusOr<std::string> AesGcmBoringSsl::Decrypt(
    absl::string_view ciphertext, absl::string_view additional_data) const {
  if (ciphertext.size() < kIvSizeInBytes + kTagSizeInBytes) {
    return util::Status(util::error::INVALID_ARGUMENT, "Ciphertext too short");
  }

  std::string result;
  ResizeStringUninitialized(
      &result, ciphertext.size() - kIvSizeInBytes - kTagSizeInBytes);
  size_t len;
  if (EVP_AEAD_CTX_open(
          ctx_.get(), reinterpret_cast<uint8_t*>(&result[0]), &len,
          result.size(),
          // The nonce is the first |kIvSizeInBytes| bytes of |ciphertext|.
          reinterpret_cast<const uint8_t*>(ciphertext.data()), kIvSizeInBytes,
          // The input is the remainder.
          reinterpret_cast<const uint8_t*>(ciphertext.data()) + kIvSizeInBytes,
          ciphertext.size() - kIvSizeInBytes,
          reinterpret_cast<const uint8_t*>(additional_data.data()),
          additional_data.size()) != 1) {
    return util::Status(util::error::INTERNAL, "Authentication failed");
  }
  return result;
}

}  // namespace subtle
}  // namespace tink
}  // namespace crypto

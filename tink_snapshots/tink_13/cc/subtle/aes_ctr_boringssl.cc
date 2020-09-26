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

#include "tink/subtle/aes_ctr_boringssl.h"

#include <string>

#include "absl/memory/memory.h"
#include "openssl/evp.h"
#include "tink/subtle/random.h"
#include "tink/subtle/subtle_util.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/status.h"

namespace crypto {
namespace tink {
namespace subtle {

util::StatusOr<std::unique_ptr<IndCpaCipher>> AesCtrBoringSsl::New(
    util::SecretData key, int iv_size) {
  const EVP_CIPHER* cipher =
      SubtleUtilBoringSSL::GetAesCtrCipherForKeySize(key.size());
  if (cipher == nullptr) {
    return util::Status(util::error::INVALID_ARGUMENT, "invalid key size");
  }
  if (iv_size < kMinIvSizeInBytes || iv_size > kBlockSize) {
    return util::Status(util::error::INVALID_ARGUMENT, "invalid iv size");
  }
  return {
      absl::WrapUnique(new AesCtrBoringSsl(std::move(key), iv_size, cipher))};
}

util::StatusOr<std::string> AesCtrBoringSsl::Encrypt(
    absl::string_view plaintext) const {
  // BoringSSL expects a non-null pointer for plaintext, regardless of whether
  // the size is 0.
  plaintext = SubtleUtilBoringSSL::EnsureNonNull(plaintext);

  bssl::UniquePtr<EVP_CIPHER_CTX> ctx(EVP_CIPHER_CTX_new());
  if (ctx.get() == nullptr) {
    return util::Status(util::error::INTERNAL,
                        "could not initialize EVP_CIPHER_CTX");
  }
  std::string ciphertext = Random::GetRandomBytes(iv_size_);
  // OpenSSL expects that the IV must be a full block. We pad with zeros.
  std::string iv_block = ciphertext;
  // Note that kBlockSize >= iv_size_ is checked in the factory method.
  // We explicitly add the '\0' argument to stress that we need to initialize
  // the new memory.
  iv_block.resize(kBlockSize, '\0');

  int ret =
      EVP_EncryptInit_ex(ctx.get(), cipher_, nullptr /* engine */, key_.data(),
                         reinterpret_cast<const uint8_t*>(&iv_block[0]));
  if (ret != 1) {
    return util::Status(util::error::INTERNAL, "could not initialize ctx");
  }
  ResizeStringUninitialized(&ciphertext, iv_size_ + plaintext.size());
  int len;
  ret = EVP_EncryptUpdate(
      ctx.get(), reinterpret_cast<uint8_t*>(&ciphertext[iv_size_]), &len,
      reinterpret_cast<const uint8_t*>(plaintext.data()), plaintext.size());
  if (ret != 1) {
    return util::Status(util::error::INTERNAL, "encryption failed");
  }
  if (len != plaintext.size()) {
    return util::Status(util::error::INTERNAL, "incorrect ciphertext size");
  }
  return ciphertext;
}

util::StatusOr<std::string> AesCtrBoringSsl::Decrypt(
    absl::string_view ciphertext) const {
  if (ciphertext.size() < iv_size_) {
    return util::Status(util::error::INVALID_ARGUMENT, "ciphertext too short");
  }

  bssl::UniquePtr<EVP_CIPHER_CTX> ctx(EVP_CIPHER_CTX_new());
  if (ctx.get() == nullptr) {
    return util::Status(util::error::INTERNAL,
                        "could not initialize EVP_CIPHER_CTX");
  }

  // Initialise key and IV
  std::string iv_block = std::string(ciphertext.substr(0, iv_size_));
  iv_block.resize(kBlockSize, '\0');
  int ret = EVP_DecryptInit_ex(ctx.get(), cipher_, nullptr /* engine */,
                               reinterpret_cast<const uint8_t*>(key_.data()),
                               reinterpret_cast<const uint8_t*>(&iv_block[0]));
  if (ret != 1) {
    return util::Status(util::error::INTERNAL,
                        "could not initialize key or iv");
  }

  size_t plaintext_size = ciphertext.size() - iv_size_;
  std::string plaintext;
  ResizeStringUninitialized(&plaintext, plaintext_size);
  size_t read = iv_size_;
  int len;
  ret = EVP_DecryptUpdate(
      ctx.get(), reinterpret_cast<uint8_t*>(&plaintext[0]), &len,
      reinterpret_cast<const uint8_t*>(&ciphertext.data()[read]),
      plaintext_size);
  if (ret != 1) {
    return util::Status(util::error::INTERNAL, "decryption failed");
  }

  if (len != plaintext_size) {
    return util::Status(util::error::INTERNAL, "incorrect plaintext size");
  }
  return plaintext;
}

}  // namespace subtle
}  // namespace tink
}  // namespace crypto

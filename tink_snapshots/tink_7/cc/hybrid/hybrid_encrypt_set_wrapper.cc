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

#include "tink/hybrid/hybrid_encrypt_set_wrapper.h"

#include "tink/crypto_format.h"
#include "tink/hybrid_encrypt.h"
#include "tink/primitive_set.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"

namespace crypto {
namespace tink {

namespace {

util::Status Validate(PrimitiveSet<HybridEncrypt>* hybrid_encrypt_set) {
  if (hybrid_encrypt_set == nullptr) {
    return util::Status(util::error::INTERNAL,
                        "hybrid_encrypt_set must be non-NULL");
  }
  if (hybrid_encrypt_set->get_primary() == nullptr) {
    return util::Status(util::error::INVALID_ARGUMENT,
                        "hybrid_encrypt_set has no primary");
  }
  return util::Status::OK;
}

}  // anonymous namespace

// static
util::StatusOr<std::unique_ptr<HybridEncrypt>>
HybridEncryptSetWrapper::NewHybridEncrypt(
    std::unique_ptr<PrimitiveSet<HybridEncrypt>> hybrid_encrypt_set) {
  util::Status status = Validate(hybrid_encrypt_set.get());
  if (!status.ok()) return status;
  std::unique_ptr<HybridEncrypt> hybrid_encrypt(
      new HybridEncryptSetWrapper(std::move(hybrid_encrypt_set)));
  return std::move(hybrid_encrypt);
}

util::StatusOr<std::string> HybridEncryptSetWrapper::Encrypt(
    absl::string_view plaintext,
    absl::string_view context_info) const {
  // BoringSSL expects a non-null pointer for plaintext and context_info,
  // regardless of whether the size is 0.
  plaintext = subtle::SubtleUtilBoringSSL::EnsureNonNull(plaintext);
  context_info = subtle::SubtleUtilBoringSSL::EnsureNonNull(context_info);

  auto primary = hybrid_encrypt_set_->get_primary();
  auto encrypt_result =
      primary->get_primitive().Encrypt(plaintext, context_info);
  if (!encrypt_result.ok()) return encrypt_result.status();
  const std::string& key_id = primary->get_identifier();
  return key_id + encrypt_result.ValueOrDie();
}

}  // namespace tink
}  // namespace crypto

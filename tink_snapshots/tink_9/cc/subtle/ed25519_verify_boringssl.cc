// Copyright 2019 Google Inc.
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

#include "tink/subtle/ed25519_verify_boringssl.h"

#include <cstring>

#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "openssl/curve25519.h"
#include "tink/public_key_verify.h"
#include "tink/subtle/subtle_util_boringssl.h"
#include "tink/util/statusor.h"

namespace crypto {
namespace tink {
namespace subtle {

// static
util::StatusOr<std::unique_ptr<PublicKeyVerify>> Ed25519VerifyBoringSsl::New(
    absl::string_view public_key) {
  if (public_key.length() != ED25519_PUBLIC_KEY_LEN) {
    return util::Status(
        util::error::INVALID_ARGUMENT,
        absl::StrFormat("Invalid ED25519 public key size (%d). "
                        "The only valid size is %d.",
                        public_key.length(), ED25519_PUBLIC_KEY_LEN));
  }
  std::unique_ptr<PublicKeyVerify> verify(
      new Ed25519VerifyBoringSsl(public_key));
  return std::move(verify);
}

Ed25519VerifyBoringSsl::Ed25519VerifyBoringSsl(absl::string_view public_key)
    : public_key_(public_key) {}

util::Status Ed25519VerifyBoringSsl::Verify(absl::string_view signature,
                                            absl::string_view data) const {
  signature = SubtleUtilBoringSSL::EnsureNonNull(signature);
  data = SubtleUtilBoringSSL::EnsureNonNull(data);

  if (signature.size() != ED25519_SIGNATURE_LEN) {
    return util::Status(
        util::error::INVALID_ARGUMENT,
        absl::StrFormat("Invalid ED25519 signature size (%d). "
                        "The signature must be %d bytes long.",
                        signature.size(), ED25519_SIGNATURE_LEN));
  }

  if (1 != ED25519_verify(
               reinterpret_cast<const uint8_t *>(data.data()), data.size(),
               reinterpret_cast<const uint8_t *>(signature.data()),
               reinterpret_cast<const uint8_t *>(public_key_.data()))) {
    return util::Status(util::error::INVALID_ARGUMENT,
                        "Signature is not valid.");
  }

  return util::Status::OK;
}

}  // namespace subtle
}  // namespace tink
}  // namespace crypto

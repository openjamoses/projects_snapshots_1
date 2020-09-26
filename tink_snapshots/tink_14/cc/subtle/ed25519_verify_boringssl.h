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

#ifndef TINK_SUBTLE_ED25519_VERIFY_BORINGSSL_H_
#define TINK_SUBTLE_ED25519_VERIFY_BORINGSSL_H_

#include <memory>
#include <string>

#include "absl/strings/string_view.h"
#include "tink/config/tink_fips.h"
#include "tink/public_key_verify.h"
#include "tink/util/statusor.h"

namespace crypto {
namespace tink {
namespace subtle {

class Ed25519VerifyBoringSsl : public PublicKeyVerify {
 public:
  static crypto::tink::util::StatusOr<std::unique_ptr<PublicKeyVerify>> New(
      absl::string_view public_key);

  // Verifies that 'signature' is a digital signature for 'data'.
  crypto::tink::util::Status Verify(absl::string_view signature,
                                    absl::string_view data) const override;

  static constexpr crypto::tink::FipsCompatibility kFipsStatus =
      crypto::tink::FipsCompatibility::kNotFips;

 private:
  const std::string public_key_;

  explicit Ed25519VerifyBoringSsl(absl::string_view public_key)
      : public_key_(public_key) {}
};

}  // namespace subtle
}  // namespace tink
}  // namespace crypto

#endif  // TINK_SUBTLE_ED25519_VERIFY_BORINGSSL_H_

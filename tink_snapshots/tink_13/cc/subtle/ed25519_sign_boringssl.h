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

#ifndef TINK_SUBTLE_ED25519_SIGN_BORINGSSL_H_
#define TINK_SUBTLE_ED25519_SIGN_BORINGSSL_H_

#include <memory>
#include <utility>

#include "tink/public_key_sign.h"
#include "tink/util/secret_data.h"
#include "tink/util/statusor.h"

namespace crypto {
namespace tink {
namespace subtle {

class Ed25519SignBoringSsl : public PublicKeySign {
 public:
  static crypto::tink::util::StatusOr<std::unique_ptr<PublicKeySign>> New(
      util::SecretData private_key);

  // Computes the signature for 'data'.
  crypto::tink::util::StatusOr<std::string> Sign(
      absl::string_view data) const override;

 private:
  explicit Ed25519SignBoringSsl(util::SecretData private_key)
      : private_key_(std::move(private_key)) {}

  const util::SecretData private_key_;
};

}  // namespace subtle
}  // namespace tink
}  // namespace crypto

#endif  // TINK_SUBTLE_ED25519_SIGN_BORINGSSL_H_

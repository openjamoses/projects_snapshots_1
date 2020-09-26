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
#ifndef TINK_AEAD_KMS_AEAD_KEY_MANAGER_H_
#define TINK_AEAD_KMS_AEAD_KEY_MANAGER_H_

#include <algorithm>
#include <vector>

#include "absl/strings/string_view.h"
#include "tink/aead.h"
#include "tink/core/key_manager_base.h"
#include "tink/key_manager.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "proto/kms_aead.pb.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

class KmsAeadKeyManager
    : public KeyManagerBase<Aead, google::crypto::tink::KmsAeadKey> {
 public:
  static constexpr uint32_t kVersion = 0;

  KmsAeadKeyManager();

  // Returns the version of this key manager.
  uint32_t get_version() const override;

  // Returns a factory that generates keys of the key type
  // handled by this manager.
  const KeyFactory& get_key_factory() const override;

  ~KmsAeadKeyManager() override {}

 protected:
  crypto::tink::util::StatusOr<std::unique_ptr<Aead>> GetPrimitiveFromKey(
      const google::crypto::tink::KmsAeadKey& key) const override;

 private:
  friend class KmsAeadKeyFactory;

  std::unique_ptr<KeyFactory> key_factory_;

  static crypto::tink::util::Status Validate(
      const google::crypto::tink::KmsAeadKey& key);
  static crypto::tink::util::Status Validate(
      const google::crypto::tink::KmsAeadKeyFormat& key_format);
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_AEAD_KMS_AEAD_KEY_MANAGER_H_

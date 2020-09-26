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

#ifndef TINK_HYBRID_HYBRID_DECRYPT_FACTORY_H_
#define TINK_HYBRID_HYBRID_DECRYPT_FACTORY_H_

#include "tink/hybrid_decrypt.h"
#include "tink/key_manager.h"
#include "tink/keyset_handle.h"
#include "tink/util/statusor.h"

namespace crypto {
namespace tink {


///////////////////////////////////////////////////////////////////////////////
// HybridDecryptFactory allows for obtaining an HybridDecrypt primitive
// from a KeysetHandle.
//
// HybridDecryptFactory gets primitives from the Registry, which can
// be initialized via a convenience method from HybridConfig-class.
//  Here is an example how one can obtain and use a HybridDecrypt primitive:
//
//   auto status = HybridConfig::Register();
//   if (!status.ok()) { /* fail with error */ }
//   KeysetHandle keyset_handle = ...;
//   std::unique_ptr<HybridDecrypt> hybrid_decrypt = std::move(
//           HybridDecryptFactory::GetPrimitive(keyset_handle).ValueOrDie());
//   std::string ciphertext = ...;
//   std::string context_info = ...;
//   std::string plaintext =
//       hybrid_decrypt.Decrypt(ciphertext, context_info).ValueOrDie();
//
class HybridDecryptFactory {
 public:
  // Returns a HybridDecrypt-primitive that uses key material from the keyset
  // specified via 'keyset_handle'.
  static crypto::tink::util::StatusOr<std::unique_ptr<HybridDecrypt>>
      GetPrimitive(const KeysetHandle& keyset_handle);

  // Returns a HybridDecrypt-primitive that uses key material from the keyset
  // specified via 'keyset_handle' and is instantiated by the given
  // 'custom_key_manager' (instead of the key manager from the Registry).
  static crypto::tink::util::StatusOr<std::unique_ptr<HybridDecrypt>>
      GetPrimitive(const KeysetHandle& keyset_handle,
                   const KeyManager<HybridDecrypt>* custom_key_manager);

 private:
  HybridDecryptFactory() {}
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_HYBRID_HYBRID_DECRYPT_FACTORY_H_

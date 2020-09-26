// Copyright 2018 Google Inc.
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

#ifndef TINK_HYBRID_HYBRID_KEY_TEMPLATES_H_
#define TINK_HYBRID_HYBRID_KEY_TEMPLATES_H_

#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

///////////////////////////////////////////////////////////////////////////////
// Pre-generated KeyTemplate for hybrid key types. One can use these templates
// to generate a new KeysetHandle object with fresh keys.
// To generate a new keyset that contains a single EciesAeadHkdfPrivateKey,
// one can do:
//   auto status = HybridConfig::Init();
//   if (!status.ok()) { /* fail with error */ }
//   status = Config::Register(HybridConfig::Tink_1_1_0());
//   if (!status.ok()) { /* fail with error */ }
//   auto handle_result = KeysetHandle.GenerateNew(
//        HybridKeyTemplates.EciesP256HkdfHmacSha256Aes128Gcm());
//   if (!handle_result.ok()) { /* fail with error */ }
//   auto keyset_handle = std::move(handle_result.ValueOrDie());
class HybridKeyTemplates {
 public:
  // Returns a KeyTemplate that generates new instances of
  // EciesAeadHkdfPrivateKey with the following parameters:
  //   - KEM: ECDH over NIST P-256
  //   - DEM: AES128-GCM
  //   - KDF: HKDF-HMAC-SHA256 with an empty salt
  //   - OutputPrefixType: TINK
  static const google::crypto::tink::KeyTemplate&
  EciesP256HkdfHmacSha256Aes128Gcm();

  // Returns a KeyTemplate that generates new instances of
  // EciesAeadHkdfPrivateKey with the following parameters:
  //   - KEM: ECDH over NIST P-256
  //   - DEM: AES128-CTR-HMAC-SHA256 with the following parameters:
  //          * AES key size: 16 bytes
  //          * AES CTR IV size: 16 bytes
  //          * HMAC key size: 32 bytes
  //          * HMAC tag size: 16 bytes
  //   - KDF: HKDF-HMAC-SHA256 with an empty salt
  //   - OutputPrefixType: TINK
  static const google::crypto::tink::KeyTemplate&
  EciesP256HkdfHmacSha256Aes128CtrHmacSha256();
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_HYBRID_HYBRID_KEY_TEMPLATES_H_

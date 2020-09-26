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

#include "tink/hybrid/hybrid_key_templates.h"

#include "absl/strings/string_view.h"
#include "tink/aead/aead_key_templates.h"
#include "proto/ecies_aead_hkdf.pb.h"
#include "proto/common.pb.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {
namespace {

using google::crypto::tink::EciesAeadHkdfKeyFormat;
using google::crypto::tink::EcPointFormat;
using google::crypto::tink::EllipticCurveType;
using google::crypto::tink::HashType;
using google::crypto::tink::KeyTemplate;
using google::crypto::tink::OutputPrefixType;

KeyTemplate* NewEciesAeadHkdfKeyTemplate(
    EllipticCurveType curve_type,
    HashType hkdf_hash_type,
    EcPointFormat ec_point_format,
    const KeyTemplate& dem_key_template,
    absl::string_view hkdf_salt) {
  KeyTemplate* key_template = new KeyTemplate;
  key_template->set_type_url(
      "type.googleapis.com/google.crypto.tink.EciesAeadHkdfPrivateKey");
  key_template->set_output_prefix_type(OutputPrefixType::TINK);
  EciesAeadHkdfKeyFormat key_format;
  key_format.mutable_params()->set_ec_point_format(ec_point_format);
  auto dem_params = key_format.mutable_params()->mutable_dem_params();
  *(dem_params->mutable_aead_dem()) = dem_key_template;
  auto kem_params = key_format.mutable_params()->mutable_kem_params();
  kem_params->set_curve_type(curve_type);
  kem_params->set_hkdf_hash_type(hkdf_hash_type);
  kem_params->set_hkdf_salt(std::string(hkdf_salt));
  key_format.SerializeToString(key_template->mutable_value());
  return key_template;
}

}  // anonymous namespace

// static
const KeyTemplate& HybridKeyTemplates::EciesP256HkdfHmacSha256Aes128Gcm() {
  static const KeyTemplate* key_template =
      NewEciesAeadHkdfKeyTemplate(EllipticCurveType::NIST_P256,
                                  HashType::SHA256,
                                  EcPointFormat::UNCOMPRESSED,
                                  AeadKeyTemplates::Aes128Gcm(),
                                  /* hkdf_salt= */ "");
  return *key_template;
}

// static
const KeyTemplate&
HybridKeyTemplates::EciesP256HkdfHmacSha256Aes128CtrHmacSha256() {
  static const KeyTemplate* key_template =
      NewEciesAeadHkdfKeyTemplate(EllipticCurveType::NIST_P256,
                                  HashType::SHA256,
                                  EcPointFormat::UNCOMPRESSED,
                                  AeadKeyTemplates::Aes128CtrHmacSha256(),
                                  /* hkdf_salt= */ "");
  return *key_template;
}


}  // namespace tink
}  // namespace crypto

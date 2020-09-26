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

#ifndef TINK_DAEAD_DETERMINISTIC_AEAD_CATALOGUE_H_
#define TINK_DAEAD_DETERMINISTIC_AEAD_CATALOGUE_H_

#include "tink/catalogue.h"
#include "tink/deterministic_aead.h"
#include "tink/key_manager.h"
#include "tink/util/statusor.h"

namespace crypto {
namespace tink {

///////////////////////////////////////////////////////////////////////////////
// A catalogue of Tink DeterministicAead key mangers.
class DeterministicAeadCatalogue : public Catalogue<DeterministicAead> {
 public:
  DeterministicAeadCatalogue() {}

  crypto::tink::util::StatusOr<std::unique_ptr<KeyManager<DeterministicAead>>>
  GetKeyManager(const std::string& type_url, const std::string& primitive_name,
                uint32_t min_version) const;
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_DAEAD_DETERMINISTIC_AEAD_CATALOGUE_H_

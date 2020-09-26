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

#ifndef TINK_HYBRID_HYBRID_CONFIG_H_
#define TINK_HYBRID_HYBRID_CONFIG_H_

#include "tink/util/status.h"
#include "proto/config.pb.h"

namespace crypto {
namespace tink {

///////////////////////////////////////////////////////////////////////////////
// Static methods and constants for registering with the Registry
// all instances of hybrid encryption key types supported in a particular
// release of Tink, i.e. key types that correspond to primitives
// HybridEncrypt and HybridDecrypt.
//
// To register all hybrid encryption key types from the current Tink release
// one can do:
//
//   auto status = HybridConfig::Register();
//
// For more information on creation and usage of instances of HybridDecrypt
// and HybridDecrypt see HybridEncryptFactory resp. HybridDecryptFactory.
class HybridConfig {
 public:
  static constexpr char kHybridDecryptCatalogueName[] = "TinkHybridDecrypt";
  static constexpr char kHybridDecryptPrimitiveName[] = "HybridDecrypt";
  static constexpr char kHybridEncryptCatalogueName[] = "TinkHybridEncrypt";
  static constexpr char kHybridEncryptPrimitiveName[] = "HybridEncrypt";

  // Returns config with implementations of HybridEncrypt and HybridDecrypt
  // supported in the current Tink release.
  static const google::crypto::tink::RegistryConfig& Latest();

  // Registers key managers for all implementations of HybridEncrypt
  // and HybridDecrypt from the current Tink release.
  static crypto::tink::util::Status Register();

 private:
  HybridConfig() {}
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_HYBRID_HYBRID_CONFIG_H_

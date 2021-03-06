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

#ifndef TINK_MAC_MAC_FACTORY_H_
#define TINK_MAC_MAC_FACTORY_H_

#include "cc/key_manager.h"
#include "cc/keyset_handle.h"
#include "cc/mac.h"
#include "cc/util/status.h"
#include "cc/util/statusor.h"

namespace cloud {
namespace crypto {
namespace tink {

///////////////////////////////////////////////////////////////////////////////
// MacFactory allows obtaining a primitive from a KeysetHandle.
//
// MacFactory gets primitives from the Registry. The factory allows
// initalizing the Registry with native key types and their managers
// that Tink supports out of the box. These key types are divided in
// two groups:
//
// - standard: secure and safe to use in new code. Over time, with new
//   developments in cryptanalysis and computing power, some standard
//   key types might become legacy.
//
// - legacy: deprecated and insecure or obsolete, should not be used
//   in new code.  Existing users should upgrade to one of the
//   standard key types.
//
// This divison allows for gradual retiring insecure or obsolete key types.
//
// For example, here is how one can obtain and use a Mac primitive:
//
//   MacFactory.RegisterStandardKeyTypes();
//   KeysetHandle keyset_handle = ...;
//   Mac mac = MacFactory.GetPrimitive(keyset_handle);
//   string data = ...;
//   string tag = mac.ComputeMac(data).ValueOrDie();
//
class MacFactory {
 public:
  // Registers standard Mac key types and their managers with the Registry.
  static util::Status RegisterStandardKeyTypes();

  // Registers legacy Mac key types and their managers with the Registry.
  static util::Status RegisterLegacyKeyTypes();

  // Returns a Mac-primitive that uses key material from the keyset
  // specified via 'keyset_handle'.
  static util::StatusOr<std::unique_ptr<Mac>> GetPrimitive(
      const KeysetHandle& keyset_handle) {
    return util::Status::UNKNOWN;
  }

  // Returns a Mac-primitive that uses key material from the keyset
  // specified via 'keyset_handle' and is instantiated by the given
  // 'custom_key_manager' (instead of the key manager from the Registry).
  static util::StatusOr<std::unique_ptr<Mac>> GetPrimitive(
      const KeysetHandle& keyset_handle,
      const KeyManager<Mac>& custom_key_manager) {
    return util::Status::UNKNOWN;
  }
};

}  // namespace tink
}  // namespace crypto
}  // namespace cloud

#endif  // TINK_MAC_MAC_FACTORY_H_

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

#ifndef TINK_UTIL_VALIDATION_H_
#define TINK_UTIL_VALIDATION_H_

#include "cc/util/errors.h"
#include "cc/util/status.h"
#include "proto/tink.pb.h"

namespace cloud {
namespace crypto {
namespace tink {

// Various validation helpers.
// TODO(przydatek): write real implementations.

util::Status ValidateKeyset(const google::cloud::crypto::tink::Keyset& keyset) {
  return util::Status::OK;
}

util::Status ValidateVersion(int candidate, int max_expected) {
  if (candidate < 0 || candidate > max_expected) {
    return ToStatusF(util::error::INVALID_ARGUMENT,
                     "Key has version '%d'; "
                     "only keys with version in range [0..%d] are supported.",
                     candidate, max_expected);
  }
  return util::Status::OK;
}


}  // namespace tink
}  // namespace crypto
}  // namespace cloud

#endif  // TINK_UTIL_VALIDATION_H_

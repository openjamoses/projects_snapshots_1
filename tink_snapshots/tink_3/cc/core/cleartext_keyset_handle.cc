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

#include "cc/cleartext_keyset_handle.h"

#include <istream>

#include "cc/keyset_handle.h"
#include "cc/keyset_reader.h"
#include "cc/util/errors.h"
#include "cc/util/ptr_util.h"
#include "cc/util/status.h"
#include "cc/util/statusor.h"
#include "proto/tink.pb.h"

using google::crypto::tink::Keyset;

namespace util = crypto::tink::util;

namespace crypto {
namespace tink {

// static
util::StatusOr<std::unique_ptr<KeysetHandle>> CleartextKeysetHandle::Read(
    std::unique_ptr<KeysetReader> reader) {
  auto keyset_result = reader->Read();
  if (!keyset_result.ok()) {
    return ToStatusF(util::error::INVALID_ARGUMENT,
                     "Error reading keyset data: %s",
                     keyset_result.status().error_message().c_str());
  }
  std::unique_ptr<KeysetHandle> handle(
      new KeysetHandle(std::move(keyset_result.ValueOrDie())));
  return std::move(handle);
}


}  // namespace tink
}  // namespace crypto

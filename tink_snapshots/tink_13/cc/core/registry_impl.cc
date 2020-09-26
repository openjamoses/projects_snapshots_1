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
#include "tink/core/registry_impl.h"

#include "tink/util/errors.h"
#include "tink/util/statusor.h"
#include "proto/tink.pb.h"

using crypto::tink::util::StatusOr;
using google::crypto::tink::KeyData;
using google::crypto::tink::KeyTemplate;

namespace crypto {
namespace tink {

StatusOr<const RegistryImpl::KeyTypeInfo*> RegistryImpl::get_key_type_info(
    const std::string& type_url) const {
  absl::MutexLock lock(&maps_mutex_);
  auto it = type_url_to_info_.find(type_url);
  if (it == type_url_to_info_.end()) {
    return ToStatusF(util::error::NOT_FOUND,
                     "No manager for type '%s' has been registered.", type_url);
  }
  return &it->second;
}

StatusOr<std::unique_ptr<KeyData>> RegistryImpl::NewKeyData(
    const KeyTemplate& key_template) const {
  auto key_type_info_or = get_key_type_info(key_template.type_url());
  if (!key_type_info_or.ok()) return key_type_info_or.status();
  if (!key_type_info_or.ValueOrDie()->new_key_allowed()) {
    return crypto::tink::util::Status(
        util::error::INVALID_ARGUMENT,
        absl::StrCat("KeyManager for type ", key_template.type_url(),
                     " does not allow for creation of new keys."));
  }
  return key_type_info_or.ValueOrDie()->key_factory().NewKeyData(
      key_template.value());
}

StatusOr<std::unique_ptr<KeyData>> RegistryImpl::GetPublicKeyData(
    const std::string& type_url,
    const std::string& serialized_private_key) const {
  auto key_type_info_or = get_key_type_info(type_url);
  if (!key_type_info_or.ok()) return key_type_info_or.status();
  auto factory = dynamic_cast<const PrivateKeyFactory*>(
      &key_type_info_or.ValueOrDie()->key_factory());
  if (factory == nullptr) {
    return ToStatusF(util::error::INVALID_ARGUMENT,
                     "KeyManager for type '%s' does not have "
                     "a PrivateKeyFactory.",
                     type_url);
  }
  auto result = factory->GetPublicKeyData(serialized_private_key);
  return result;
}

crypto::tink::util::Status RegistryImpl::CheckInsertable(
    const std::string& type_url, const std::type_index& key_manager_type_index,
    bool new_key_allowed) const {
  auto it = type_url_to_info_.find(type_url);

  if (it == type_url_to_info_.end()) {
    return crypto::tink::util::Status::OK;
  }
  if (it->second.key_manager_type_index() != key_manager_type_index) {
    return ToStatusF(crypto::tink::util::error::ALREADY_EXISTS,
                     "A manager for type '%s' has been already registered.",
                     type_url);
  }
  if (!it->second.new_key_allowed() && new_key_allowed) {
    return ToStatusF(crypto::tink::util::error::ALREADY_EXISTS,
                     "A manager for type '%s' has been already registered "
                     "with forbidden new key operation.",
                     type_url);
  }
  return crypto::tink::util::Status::OK;
}

crypto::tink::util::StatusOr<google::crypto::tink::KeyData>
RegistryImpl::DeriveKey(const google::crypto::tink::KeyTemplate& key_template,
                        InputStream* randomness) const {
  auto key_type_info_or = get_key_type_info(key_template.type_url());
  if (!key_type_info_or.ok()) return key_type_info_or.status();
  if (!key_type_info_or.ValueOrDie()->key_deriver()) {
    return crypto::tink::util::Status(
        crypto::tink::util::error::INVALID_ARGUMENT,
        absl::StrCat("Manager for type '", key_template.type_url(),
                     "' cannot derive keys."));
  }
  return key_type_info_or.ValueOrDie()->key_deriver()(key_template.value(),
                                                      randomness);
}

void RegistryImpl::Reset() {
  absl::MutexLock lock(&maps_mutex_);
  type_url_to_info_.clear();
  name_to_catalogue_map_.clear();
  primitive_to_wrapper_.clear();
}

}  // namespace tink
}  // namespace crypto

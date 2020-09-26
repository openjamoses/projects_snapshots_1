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
#include "tink/prf/prf_set_wrapper.h"

#include "absl/memory/memory.h"
#include "tink/util/status.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

using google::crypto::tink::OutputPrefixType;

namespace {

class PrfSetPrimitiveWrapper : public PrfSet {
 public:
  explicit PrfSetPrimitiveWrapper(std::unique_ptr<PrimitiveSet<PrfSet>> prf_set)
      : prf_set_(std::move(prf_set)) {
    auto prf_sets = prf_set_->get_raw_primitives().ValueOrDie();
    for (const auto& prf : *prf_sets) {
      prfs_.insert(
          {prf->get_key_id(), prf->get_primitive().GetPrfs().begin()->second});
    }
  }

  uint32_t GetPrimaryId() const override {
    return prf_set_->get_primary()->get_key_id();
  }
  const std::map<uint32_t, Prf*>& GetPrfs() const override { return prfs_; }

  ~PrfSetPrimitiveWrapper() override {}

 private:
  std::unique_ptr<PrimitiveSet<PrfSet>> prf_set_;
  std::map<uint32_t, Prf*> prfs_;
};

util::Status Validate(PrimitiveSet<PrfSet>* prf_set) {
  if (prf_set == nullptr) {
    return util::Status(util::error::INTERNAL, "prf_set must be non-NULL");
  }
  if (prf_set->get_primary() == nullptr) {
    return util::Status(util::error::INVALID_ARGUMENT,
                        "prf_set has no primary");
  }
  for (auto prf : prf_set->get_all()) {
    if (prf->get_primitive().GetPrfs().size() != 1) {
      return util::Status(util::error::INTERNAL,
                          "prf_set contains prfs with more than one function");
    }
    if (prf->get_output_prefix_type() != OutputPrefixType::RAW) {
      return util::Status(util::error::INVALID_ARGUMENT,
                          "PrfSet should only be used with prefix type RAW");
    }
  }
  return util::Status::OK;
}

}  // namespace

util::StatusOr<std::unique_ptr<PrfSet>> PrfSetWrapper::Wrap(
    std::unique_ptr<PrimitiveSet<PrfSet>> prf_set) const {
  util::Status status = Validate(prf_set.get());
  if (!status.ok()) return status;
  return {absl::make_unique<PrfSetPrimitiveWrapper>(std::move(prf_set))};
}

}  // namespace tink
}  // namespace crypto

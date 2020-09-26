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

#ifndef TINK_OUTPUT_STREAM_WITH_RESULT_H_
#define TINK_OUTPUT_STREAM_WITH_RESULT_H_

#include <type_traits>

#include "tink/output_stream.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"

namespace crypto {
namespace tink {

// An abstract OutputStream subclass that acts as a sink for data and returns a
// result after the stream has been closed.
// The result is only available if the stream was able to close with an ok
// status. Otherwise, the status of the GetResult call will be the same status
// as the Close call had. If Close is successful, GetResult is guaranteed to
// return a valid result.
// The return type of GetResult will be StatusOr<T>, except for T == Status, in
// which case the return type is Status.
template <class T>
class OutputStreamWithResult : public OutputStream {
 public:
  OutputStreamWithResult() : closed_(false) {}
  ~OutputStreamWithResult() override {}

  // The return type is StatusOr<T> if T != Status, and Status otherwise.
  using ResultType =
      typename std::conditional<std::is_same<T, util::Status>::value,
                                util::Status, util::StatusOr<T>>::type;

  // Get the result associated with this OutputStream. Can only be called on
  // closed streams, and will otherwise fail with FAILED_PRECONDITION as error
  // code.
  // If Close() returned an ok status, this method is guaranteed to contain a
  // valid result.
  // The return type is StatusOr<T> if T != Status, and Status otherwise.
  ResultType GetResult() {
    if (!closed_) {
      return util::Status(util::error::FAILED_PRECONDITION,
                          "Stream is not closed");
    }
    return result_;
  }

  // Close the stream and return the computed result. Equivalent to calling
  // Close() and GetResult() if Close() returned an OK status.
  // The return type is StatusOr<T> if T != Status, and Status otherwise.
  ResultType CloseAndGetResult() {
    util::Status closing_status = Close();
    if (!closing_status.ok()) {
      return closing_status;
    }
    return GetResult();
  }

  // Closes the OutputStream.
  util::Status Close() final {
    if (closed_) {
      return util::Status(util::error::FAILED_PRECONDITION, "Stream closed");
    }
    result_ = CloseStreamAndComputeResult();
    closed_ = true;
    return result_.status();
  }

  // Getting the next OutputStream buffer. See OutputStream for detailed
  // description.
  crypto::tink::util::StatusOr<int> Next(void** data) final {
    if (closed_) {
      return util::Status(util::error::FAILED_PRECONDITION,
                          "Write on closed Stream");
    }
    return NextBuffer(data);
  }

 protected:
  // Compute the result for this OutputStream. Safe for thread safety problems,
  // this method will only be called once.
  // The return type is StatusOr<T> if T != Status, and Status otherwise.
  virtual ResultType CloseStreamAndComputeResult() = 0;
  // Getting the next OutputStream buffer. See OutputStream for detailed
  // description.
  virtual util::StatusOr<int> NextBuffer(void** data) = 0;

 private:
  bool closed_;
  ResultType result_;
};

}  // namespace tink
}  // namespace crypto

#endif  // TINK_OUTPUT_STREAM_WITH_RESULT_H_

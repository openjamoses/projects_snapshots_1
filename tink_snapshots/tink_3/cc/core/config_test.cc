// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "cc/config.h"
#include "cc/mac.h"
#include "gtest/gtest.h"
#include "proto/config.pb.h"

using google::crypto::tink::KeyTypeEntry;
using google::protobuf::StringPiece;

namespace crypto {
namespace tink {
namespace {

class ConfigTest : public ::testing::Test {
};

TEST_F(ConfigTest, testValidation) {
  KeyTypeEntry entry;

  auto status = Config::Register<Mac>(entry);
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(util::error::INVALID_ARGUMENT, status.error_code());

  entry.set_type_url("some key type");
  entry.set_catalogue_name("some catalogue");
  status = Config::Register<Mac>(entry);
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(util::error::INVALID_ARGUMENT, status.error_code());

  entry.set_primitive_name("some primitive");
  status = Config::Register<Mac>(entry);
  EXPECT_FALSE(status.ok());
  EXPECT_EQ(util::error::NOT_FOUND, status.error_code());
}


// TODO(przydatek): add more tests.

}  // namespace
}  // namespace tink
}  // namespace crypto


int main(int ac, char* av[]) {
  testing::InitGoogleTest(&ac, av);
  return RUN_ALL_TESTS();
}

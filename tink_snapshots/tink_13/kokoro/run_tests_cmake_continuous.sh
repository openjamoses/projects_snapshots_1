#!/bin/bash

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

set -e

cd git*/tink

echo "========================================================= Running cmake"
cmake --version
cmake . -DTINK_BUILD_TESTS=ON -DCMAKE_CXX_STANDARD=11
echo "==================================================== Building with make"
make -j8 all
echo "===================================================== Testing with make"
CTEST_OUTPUT_ON_FAILURE=1 make test
echo "================================================ Done testing with make"

export TINK_SRC_DIR=$(pwd)
export TEST_TMPDIR=$(mktemp -d)
export TEST_SRCDIR=$(cd ..; pwd)
cd examples/cc/helloworld
./cmake_build_test.sh

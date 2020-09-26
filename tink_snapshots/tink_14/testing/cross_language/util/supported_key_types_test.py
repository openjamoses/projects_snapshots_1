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
"""Tests for tink.testing.cross_language.supported_key_types."""

from absl.testing import absltest
from util import supported_key_types


class SupportedKeyTypesTest(absltest.TestCase):

  def test_all_key_types_present(self):
    self.assertEqual(
        list(supported_key_types.SUPPORTED_LANGUAGES.keys()),
        supported_key_types.ALL_KEY_TYPES)
    self.assertEqual(
        list(supported_key_types.KEY_TEMPLATE_NAMES.keys()),
        supported_key_types.ALL_KEY_TYPES)

  def test_all_key_templates_present(self):
    def all_key_template_names():
      for _, names in supported_key_types.KEY_TEMPLATE_NAMES.items():
        for name in names:
          yield name
    self.assertEqual(
        list(all_key_template_names()),
        list(supported_key_types.KEY_TEMPLATE.keys()))

  def test_test_cases(self):
    self.assertEqual(
        list(supported_key_types.test_cases(
            ['AesEaxKey', 'ChaCha20Poly1305Key'])),
        [('AES128_EAX', ['cc', 'java', 'python']),
         ('AES256_EAX', ['cc', 'java', 'python']),
         ('CHACHA20_POLY1305', ['java', 'go'])])

if __name__ == '__main__':
  absltest.main()

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS-IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""Tests for tink.testing.cross_language.util.testing_server."""

import io

from absl.testing import absltest
from absl.testing import parameterized

import tink
from tink import aead
from tink import daead
from tink import hybrid
from tink import mac
from tink import prf
from tink import signature
from tink import streaming_aead

from util import testing_servers

_SUPPORTED_LANGUAGES = testing_servers.SUPPORTED_LANGUAGES_BY_PRIMITIVE


class TestingServersConfigTest(absltest.TestCase):

  def test_primitives(self):
    self.assertEqual(
        testing_servers._PRIMITIVE_STUBS.keys(),
        _SUPPORTED_LANGUAGES.keys(),
        msg=(
            'The primitives specified as keys in '
            'testing_servers._PRIMITIVE_STUBS must match the primitives '
            ' specified as keys in '
            'testing_servers.SUPPORTED_LANGUAGES_BY_PRIMITIVE.'
        ))

  def test_languages(self):
    for primitive in _SUPPORTED_LANGUAGES:
      languages = set(testing_servers.LANGUAGES)
      supported_languages = set(_SUPPORTED_LANGUAGES[primitive])
      self.assertContainsSubset(supported_languages, languages, msg=(
          'The languages specified in '
          'testing_servers.SUPPORTED_LANGUAGES_BY_PRIMITIVE must be a subset '
          'of the languages specified in testing_servers.LANGUAGES.'
      ))


class TestingServersTest(parameterized.TestCase):

  @classmethod
  def setUpClass(cls):
    super(TestingServersTest, cls).setUpClass()
    testing_servers.start()

  @classmethod
  def tearDownClass(cls):
    testing_servers.stop()
    super(TestingServersTest, cls).tearDownClass()

  @parameterized.parameters(_SUPPORTED_LANGUAGES['aead'])
  def test_aead(self, lang):
    keyset = testing_servers.new_keyset(lang,
                                        aead.aead_key_templates.AES128_GCM)
    plaintext = b'The quick brown fox jumps over the lazy dog'
    associated_data = b'associated_data'
    aead_primitive = testing_servers.aead(lang, keyset)
    ciphertext = aead_primitive.encrypt(plaintext, associated_data)
    output = aead_primitive.decrypt(ciphertext, associated_data)
    self.assertEqual(output, plaintext)

    with self.assertRaises(tink.TinkError):
      aead_primitive.decrypt(b'foo', associated_data)

  @parameterized.parameters(_SUPPORTED_LANGUAGES['daead'])
  def test_daead(self, lang):
    keyset = testing_servers.new_keyset(
        lang, daead.deterministic_aead_key_templates.AES256_SIV)
    plaintext = b'The quick brown fox jumps over the lazy dog'
    associated_data = b'associated_data'
    daead_primitive = testing_servers.deterministic_aead(lang, keyset)
    ciphertext = daead_primitive.encrypt_deterministically(
        plaintext, associated_data)
    output = daead_primitive.decrypt_deterministically(
        ciphertext, associated_data)
    self.assertEqual(output, plaintext)

    with self.assertRaises(tink.TinkError):
      daead_primitive.decrypt_deterministically(b'foo', associated_data)

  @parameterized.parameters(_SUPPORTED_LANGUAGES['streaming_aead'])
  def test_streaming_aead(self, lang):
    keyset = testing_servers.new_keyset(
        lang, streaming_aead.streaming_aead_key_templates.AES128_GCM_HKDF_4KB)
    plaintext = b'The quick brown fox jumps over the lazy dog'
    plaintext_stream = io.BytesIO(plaintext)
    associated_data = b'associated_data'
    streaming_aead_primitive = testing_servers.streaming_aead(lang, keyset)
    ciphertext_stream = streaming_aead_primitive.new_encrypting_stream(
        plaintext_stream, associated_data)
    output_stream = streaming_aead_primitive.new_decrypting_stream(
        ciphertext_stream, associated_data)
    self.assertEqual(output_stream.read(), plaintext)

    with self.assertRaises(tink.TinkError):
      streaming_aead_primitive.new_decrypting_stream(io.BytesIO(b'foo'),
                                                     associated_data)

  @parameterized.parameters(_SUPPORTED_LANGUAGES['mac'])
  def test_mac(self, lang):
    keyset = testing_servers.new_keyset(
        lang, mac.mac_key_templates.HMAC_SHA256_128BITTAG)
    data = b'The quick brown fox jumps over the lazy dog'
    mac_primitive = testing_servers.mac(lang, keyset)
    mac_value = mac_primitive.compute_mac(data)
    mac_primitive.verify_mac(mac_value, data)

    with self.assertRaises(tink.TinkError):
      mac_primitive.verify_mac(b'foo', data)

  @parameterized.parameters(_SUPPORTED_LANGUAGES['hybrid'])
  def test_hybrid(self, lang):
    private_handle = testing_servers.new_keyset(
        lang,
        hybrid.hybrid_key_templates.ECIES_P256_HKDF_HMAC_SHA256_AES128_GCM)
    public_handle = testing_servers.public_keyset(lang, private_handle)
    enc_primitive = testing_servers.hybrid_encrypt(lang, public_handle)
    data = b'The quick brown fox jumps over the lazy dog'
    context_info = b'context'
    ciphertext = enc_primitive.encrypt(data, context_info)
    dec_primitive = testing_servers.hybrid_decrypt(lang, private_handle)
    output = dec_primitive.decrypt(ciphertext, context_info)
    self.assertEqual(output, data)

    with self.assertRaises(tink.TinkError):
      dec_primitive.decrypt(b'foo', context_info)

  @parameterized.parameters(_SUPPORTED_LANGUAGES['signature'])
  def test_signature(self, lang):
    private_handle = testing_servers.new_keyset(
        lang, signature.signature_key_templates.ED25519)
    public_handle = testing_servers.public_keyset(lang, private_handle)
    sign_primitive = testing_servers.public_key_sign(lang, private_handle)
    data = b'The quick brown fox jumps over the lazy dog'
    signature_value = sign_primitive.sign(data)
    verify_primitive = testing_servers.public_key_verify(lang, public_handle)
    verify_primitive.verify(signature_value, data)

    with self.assertRaises(tink.TinkError):
      verify_primitive.verify(b'foo', data)

  @parameterized.parameters(_SUPPORTED_LANGUAGES['prf'])
  def test_prf(self, lang):
    keyset = testing_servers.new_keyset(lang,
                                        prf.prf_key_templates.HMAC_SHA256)
    input_data = b'The quick brown fox jumps over the lazy dog'
    prf_set_primitive = testing_servers.prf_set(lang, keyset)
    output = prf_set_primitive.primary().compute(input_data, output_length=15)
    self.assertLen(output, 15)

    with self.assertRaises(tink.TinkError):
      prf_set_primitive.primary().compute(input_data, output_length=123456)


if __name__ == '__main__':
  absltest.main()

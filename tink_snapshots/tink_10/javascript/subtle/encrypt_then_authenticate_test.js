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

goog.module('tink.subtle.EncryptThenAuthenticateTest');
goog.setTestOnly('tink.subtle.EncryptThenAuthenticateTest');

const Bytes = goog.require('tink.subtle.Bytes');
const EncryptThenAuthenticate = goog.require('tink.subtle.EncryptThenAuthenticate');
const Random = goog.require('tink.subtle.Random');
const TestCase = goog.require('goog.testing.TestCase');
const testSuite = goog.require('goog.testing.testSuite');

testSuite({
  setUp() {
    // Use a generous promise timeout for running continuously.
    TestCase.getActiveTestCase().promiseTimeout = 1000 * 1000;  // 1000s
  },

  tearDown() {
    // Reset the promise timeout to default value.
    TestCase.getActiveTestCase().promiseTimeout = 1000;  // 1s
  },

  async testBasic() {
    const aead = await EncryptThenAuthenticate.newAesCtrHmac(
        Random.randBytes(16) /* aesKey */, 12 /* ivSize */, 'SHA-256',
        Random.randBytes(16) /* hmacKey */, 10 /* tagSize */);
    for (let i = 0; i < 100; i++) {
      const msg = Random.randBytes(20);
      let ciphertext = await aead.encrypt(msg);
      let plaintext = await aead.decrypt(ciphertext);
      assertEquals(Bytes.toHex(msg), Bytes.toHex(plaintext));

      let aad = null;
      ciphertext = await aead.encrypt(msg, aad);
      plaintext = await aead.decrypt(ciphertext, aad);
      assertEquals(Bytes.toHex(msg), Bytes.toHex(plaintext));

      aad = Random.randBytes(20);
      ciphertext = await aead.encrypt(msg, aad);
      plaintext = await aead.decrypt(ciphertext, aad);
      assertEquals(Bytes.toHex(msg), Bytes.toHex(plaintext));
    }
  },

  async testProbabilisticEncryption() {
    const aead = await EncryptThenAuthenticate.newAesCtrHmac(
        Random.randBytes(16) /* aesKey */, 12 /* ivSize */, 'SHA-256',
        Random.randBytes(16) /* hmacKey */, 10 /* tagSize */);
    const msg = Random.randBytes(20);
    const aad = Random.randBytes(20);
    const results = new Set();
    for (let i = 0; i < 100; i++) {
      const ciphertext = await aead.encrypt(msg, aad);
      results.add(Bytes.toHex(ciphertext));
    }
    assertEquals(100, results.size);
  },

  async testType() {
    try {
      await EncryptThenAuthenticate.newAesCtrHmac(
          'blah' /* aesKey */, 12 /* ivSize */, 'SHA-256',
          Random.randBytes(16) /* hmacKey */, 10 /* tagSize */);
      fail('Should throw an exception.');
    } catch (e) {
      assertEquals(
          'CustomError: input must be a non null Uint8Array', e.toString());
    }
    try {
      await EncryptThenAuthenticate.newAesCtrHmac(
          Random.randBytes(16) /* aesKey */, 12 /* ivSize */, 'SHA-256',
          'blah' /* hmacKey */, 10 /* tagSize */);
      fail('Should throw an exception.');
    } catch (e) {
      assertEquals(
          'CustomError: input must be a non null Uint8Array', e.toString());
    }

    const aead = await EncryptThenAuthenticate.newAesCtrHmac(
        Random.randBytes(16) /* aesKey */, 12 /* ivSize */, 'SHA-256',
        Random.randBytes(16) /* hmacKey */, 10 /* tagSize */);
    try {
      await aead.encrypt('blah');
      fail('Should throw an exception.');
    } catch (e) {
      assertEquals(
          'CustomError: input must be a non null Uint8Array', e.toString());
    }
    try {
      await aead.encrypt(Random.randBytes(20), 'blah');
      fail('Should throw an exception.');
    } catch (e) {
      assertEquals(
          'CustomError: input must be a non null Uint8Array', e.toString());
    }
    try {
      await aead.decrypt('blah');
      fail('Should throw an exception.');
    } catch (e) {
      assertEquals(
          'CustomError: input must be a non null Uint8Array', e.toString());
    }
    try {
      await aead.decrypt(Random.randBytes(32), 'blah');
      fail('Should throw an exception.');
    } catch (e) {
      assertEquals(
          'CustomError: input must be a non null Uint8Array', e.toString());
    }
  },

  async testBitFlipCiphertext() {
    const aead = await EncryptThenAuthenticate.newAesCtrHmac(
        Random.randBytes(16) /* aesKey */, 16 /* ivSize */, 'SHA-256',
        Random.randBytes(16) /* hmacKey */, 16 /* tagSize */);
    const plaintext = Random.randBytes(8);
    const aad = Random.randBytes(8);
    const ciphertext = await aead.encrypt(plaintext, aad);
    for (let i = 0; i < ciphertext.length; i++) {
      for (let j = 0; j < 8; j++) {
        const c1 = new Uint8Array(ciphertext);
        c1[i] = (c1[i] ^ (1 << j));
        try {
          await aead.decrypt(c1, aad);
          fail('Should throw an exception.');
        } catch (e) {
          assertEquals('CustomError: invalid MAC', e.toString());
        }
      }
    }
  },

  async testBitFlipAad() {
    const aead = await EncryptThenAuthenticate.newAesCtrHmac(
        Random.randBytes(16) /* aesKey */, 16 /* ivSize */, 'SHA-256',
        Random.randBytes(16) /* hmacKey */, 16 /* tagSize */);
    const plaintext = Random.randBytes(8);
    const aad = Random.randBytes(8);
    const ciphertext = await aead.encrypt(plaintext, aad);
    for (let i = 0; i < aad.length; i++) {
      for (let j = 0; j < 8; j++) {
        const aad1 = new Uint8Array(aad);
        aad1[i] = (aad1[i] ^ (1 << j));
        try {
          await aead.decrypt(ciphertext, aad1);
          fail('Should throw an exception.');
        } catch (e) {
          assertEquals('CustomError: invalid MAC', e.toString());
        }
      }
    }
  },

  async testTruncation() {
    const aead = await EncryptThenAuthenticate.newAesCtrHmac(
        Random.randBytes(16) /* aesKey */, 16 /* ivSize */, 'SHA-256',
        Random.randBytes(16) /* hmacKey */, 16 /* tagSize */);
    const plaintext = Random.randBytes(8);
    const aad = Random.randBytes(8);
    const ciphertext = await aead.encrypt(plaintext, aad);
    for (let i = 1; i <= ciphertext.length; i++) {
      const c1 = new Uint8Array(ciphertext.buffer, 0, ciphertext.length - i);
      try {
        await aead.decrypt(c1, aad);
        fail('Should throw an exception.');
      } catch (e) {
        if (c1.length < 32) {
          assertEquals('CustomError: ciphertext too short', e.toString());
        } else {
          assertEquals('CustomError: invalid MAC', e.toString());
        }
      }
    }
  },

  async testWithRfcTestVectors() {
    // Test data from
    // https://tools.ietf.org/html/draft-mcgrew-aead-aes-cbc-hmac-sha2-05. As we
    // use CTR while RFC uses CBC mode, it's not possible to compare plaintexts.
    // However, the test is still valueable to make sure that we correcly
    // compute HMAC over ciphertext and aad.
    const RFC_TEST_VECTORS = [
      {
        'macKey': '000102030405060708090a0b0c0d0e0f',
        'encryptionKey': '101112131415161718191a1b1c1d1e1f',
        'ciphertext': '1af38c2dc2b96ffdd86694092341bc04' +
            'c80edfa32ddf39d5ef00c0b468834279' +
            'a2e46a1b8049f792f76bfe54b903a9c9' +
            'a94ac9b47ad2655c5f10f9aef71427e2' +
            'fc6f9b3f399a221489f16362c7032336' +
            '09d45ac69864e3321cf82935ac4096c8' +
            '6e133314c54019e8ca7980dfa4b9cf1b' +
            '384c486f3a54c51078158ee5d79de59f' +
            'bd34d848b3d69550a67646344427ade5' +
            '4b8851ffb598f7f80074b9473c82e2db' +
            '652c3fa36b0a7c5b3219fab3a30bc1c4',
        'aad': '546865207365636f6e64207072696e63' +
            '69706c65206f66204175677573746520' +
            '4b6572636b686f666673',
        'hashAlgoName': 'SHA-256',
        'ivSize': 16,
        'tagSize': 16
      },
      {
        'macKey':
            '000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f',
        'encryptionKey':
            '202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f',
        'ciphertext': '1af38c2dc2b96ffdd86694092341bc04' +
            '4affaaadb78c31c5da4b1b590d10ffbd' +
            '3dd8d5d302423526912da037ecbcc7bd' +
            '822c301dd67c373bccb584ad3e9279c2' +
            'e6d12a1374b77f077553df829410446b' +
            '36ebd97066296ae6427ea75c2e0846a1' +
            '1a09ccf5370dc80bfecbad28c73f09b3' +
            'a3b75e662a2594410ae496b2e2e6609e' +
            '31e6e02cc837f053d21f37ff4f51950b' +
            'be2638d09dd7a4930930806d0703b1f6' +
            '4dd3b4c088a7f45c216839645b2012bf' +
            '2e6269a8c56a816dbc1b267761955bc5',
        'aad':
            '546865207365636f6e64207072696e6369706c65206f662041756775737465204b6572636b686f666673',
        'hashAlgoName': 'SHA-512',
        'ivSize': 16,
        'tagSize': 32
      },
    ];
    for (let i = 0; i < RFC_TEST_VECTORS.length; i++) {
      const testVector = RFC_TEST_VECTORS[i];
      const aead = await EncryptThenAuthenticate.newAesCtrHmac(
          Bytes.fromHex(testVector['encryptionKey']), testVector['ivSize'],
          testVector['hashAlgoName'], Bytes.fromHex(testVector['macKey']),
          testVector['tagSize']);
      const ciphertext = Bytes.fromHex(testVector['ciphertext']);
      const aad = Bytes.fromHex(testVector['aad']);
      try {
        await aead.decrypt(ciphertext, aad);
      } catch (e) {
        fail(e);
      }
    }
  },
});

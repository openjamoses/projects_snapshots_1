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

goog.module('tink.subtle.EcdsaSignTest');
goog.setTestOnly('tink.subtle.EcdsaSignTest');

const EcdsaSign = goog.require('tink.subtle.EcdsaSign');
const EllipticCurves = goog.require('tink.subtle.EllipticCurves');
const Random = goog.require('tink.subtle.Random');

describe('ecdsa sign test', function() {
  beforeEach(function() {
    // Use a generous promise timeout for running continuously.
    jasmine.DEFAULT_TIMEOUT_INTERVAL = 1000 * 1000;  // 1000s
  });

  afterEach(function() {
    // Reset the promise timeout to default value.
    jasmine.DEFAULT_TIMEOUT_INTERVAL = 1000;  // 1s
  });

  it('sign', async function() {
    const keyPair = await EllipticCurves.generateKeyPair('ECDSA', 'P-256');
    const signer = await EcdsaSign.newInstance(
        await EllipticCurves.exportCryptoKey(keyPair.privateKey), 'SHA-256');
    for (let i = 0; i < 100; i++) {
      const data = Random.randBytes(i);
      const signature = await signer.sign(data);
      const isValid = await window.crypto.subtle.verify(
          {
            name: 'ECDSA',
            hash: {
              name: 'SHA-256',
            },
          },
          keyPair.publicKey, signature, data);
      expect(isValid).toBe(true);
    }
  });

  it('sign with der encoding', async function() {
    const keyPair = await EllipticCurves.generateKeyPair('ECDSA', 'P-256');
    const signer = await EcdsaSign.newInstance(
        await EllipticCurves.exportCryptoKey(keyPair.privateKey), 'SHA-256',
        EllipticCurves.EcdsaSignatureEncodingType.DER);
    for (let i = 0; i < 100; i++) {
      const data = Random.randBytes(i);
      let signature = await signer.sign(data);
      // Should fail WebCrypto only accepts IEEE encoding.
      let isValid = await window.crypto.subtle.verify(
          {
            name: 'ECDSA',
            hash: {
              name: 'SHA-256',
            },
          },
          keyPair.publicKey, signature, data);
      expect(isValid).toBe(false);
      // Convert the signature to IEEE encoding.
      signature = EllipticCurves.ecdsaDer2Ieee(signature, 64);
      isValid = await window.crypto.subtle.verify(
          {
            name: 'ECDSA',
            hash: {
              name: 'SHA-256',
            },
          },
          keyPair.publicKey, signature, data);
      expect(isValid).toBe(true);
    }
  });

  it('sign always generate new signatures', async function() {
    const keyPair = await EllipticCurves.generateKeyPair('ECDSA', 'P-256');
    const signer = await EcdsaSign.newInstance(
        await EllipticCurves.exportCryptoKey(keyPair.privateKey), 'SHA-256');
    const signatures = new Set();
    for (let i = 0; i < 100; i++) {
      const data = Random.randBytes(i);
      const signature = await signer.sign(data);
      signatures.add(signature);
    }
    expect(signatures.size).toBe(100);
  });

  it('constructor with invalid hash', async function() {
    try {
      const keyPair = await EllipticCurves.generateKeyPair('ECDSA', 'P-256');
      await EcdsaSign.newInstance(
          await EllipticCurves.exportCryptoKey(keyPair.privateKey), 'SHA-1');
      fail('Should throw an exception.');
    } catch (e) {
      expect(e.toString())
          .toBe(
              'SecurityException: expected SHA-256 (because curve is P-256) but ' +
              'got SHA-1');
    }

    try {
      const keyPair = await EllipticCurves.generateKeyPair('ECDSA', 'P-384');
      await EcdsaSign.newInstance(
          await EllipticCurves.exportCryptoKey(keyPair.privateKey), 'SHA-256');
      fail('Should throw an exception.');
    } catch (e) {
      expect(e.toString())
          .toBe(
              'SecurityException: expected SHA-384 or SHA-512 (because curve is P-384) but got SHA-256');
    }

    try {
      const keyPair = await EllipticCurves.generateKeyPair('ECDSA', 'P-521');
      await EcdsaSign.newInstance(
          await EllipticCurves.exportCryptoKey(keyPair.privateKey), 'SHA-256');
      fail('Should throw an exception.');
    } catch (e) {
      expect(e.toString())
          .toBe(
              'SecurityException: expected SHA-512 (because curve is P-521) but got SHA-256');
    }
  });

  it('constructor with invalid curve', async function() {
    try {
      const keyPair = await EllipticCurves.generateKeyPair('ECDSA', 'P-256');
      const jwk = await EllipticCurves.exportCryptoKey(keyPair.privateKey);
      jwk.crv = 'blah';
      await EcdsaSign.newInstance(jwk, 'SHA-256');
      fail('Should throw an exception.');
    } catch (e) {
      expect(e.toString()).toBe('SecurityException: unsupported curve: blah');
    }
  });
});

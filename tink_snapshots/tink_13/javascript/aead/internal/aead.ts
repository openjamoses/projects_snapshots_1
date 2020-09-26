// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//      http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
////////////////////////////////////////////////////////////////////////////////

/**
 * Interface for Authenticated Encryption with Associated Data (AEAD).
 *
 * Security guarantees: Implementations of this interface are secure against
 * adaptive chosen ciphertext attacks. Encryption with associated data ensures
 * authenticity (who the sender is) and integrity (the data has not been
 * tampered with) of that data, but not its secrecy.
 *
 * @see https://tools.ietf.org/html/rfc5116
 */
export abstract class Aead {
  /**
   * Encrypts `plaintext` with `opt_associatedData` as associated authenticated
   * data. The resulting ciphertext allows for checking authenticity and
   * integrity of associated data, but does not guarantee its secrecy.
   *
   * @param plaintext the plaintext to be encrypted. It must be
   *     non-null, but can also be an empty (zero-length) byte array.
   * @param opt_associatedData  optional associated data to be
   *     authenticated, but not encrypted. A null value is equivalent to an
   *     empty (zero-length) byte array. For successful decryption the same
   *     associated data must be provided along with the ciphertext.
   * @return resulting ciphertext
   *
   */
  abstract encrypt(plaintext: Uint8Array, opt_associatedData?: Uint8Array|null):
      Promise<Uint8Array>;

  /**
   * Decrypts ciphertext with associated authenticated data.
   * The decryption verifies the authenticity and integrity of the associated
   * data, but there are no guarantees wrt. secrecy of that data.
   *
   * @param ciphertext the ciphertext to be decrypted, must be
   *     non-null.
   * @param opt_associatedData  optional associated data to be
   *     authenticated. A null value is equivalent to an empty (zero-length)
   *     byte array. For successful decryption the same associated data must be
   *     provided along with the ciphertext.
   * @return resulting plaintext
   */
  abstract decrypt(
      ciphertext: Uint8Array,
      opt_associatedData?: Uint8Array|null): Promise<Uint8Array>;
}

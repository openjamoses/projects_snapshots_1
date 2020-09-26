// Copyright 2018 Google Inc.
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

package com.google.crypto.tink.subtle;

import java.security.InvalidKeyException;
import java.util.Arrays;

/**
 * {@link XChaCha20} stream cipher based on
 * https://download.libsodium.org/doc/advanced/xchacha20.html and
 * https://tools.ietf.org/html/draft-arciszewski-xchacha-01.
 *
 * <p>This cipher is meant to be used to construct an AEAD with Poly1305.
 */
class XChaCha20 extends ChaCha20Base {
  /**
   * Constructs a new XChaCha20 cipher with the supplied {@code key}.
   *
   * @throws IllegalArgumentException when {@code key} length is not {@link
   *     ChaCha20Base#KEY_SIZE_IN_BYTES}.
   */
  XChaCha20(byte[] key, int initialCounter) throws InvalidKeyException {
    super(key, initialCounter);
  }

  @Override
  int[] createInitialState(final int[] nonce, int counter) {
    if (nonce.length != nonceSizeInBytes() / 4) {
      throw new IllegalArgumentException(
          String.format(
              "XChaCha20 uses 192-bit nonces, but got a %d-bit nonce", nonce.length * 32));
    }
    // Set the initial state based on
    // https://tools.ietf.org/html/draft-arciszewski-xchacha-01#section-2.3.
    int[] state = new int[ChaCha20Base.BLOCK_SIZE_IN_INTS];
    ChaCha20Base.setSigmaAndKey(state, hChaCha20(this.key, nonce));
    state[12] = counter;
    state[13] = 0;
    state[14] = nonce[4];
    state[15] = nonce[5];
    return state;
  }

  @Override
  int nonceSizeInBytes() {
    return 24;
  }

  // See https://tools.ietf.org/html/draft-arciszewski-xchacha-01#section-2.2.
  static int[] hChaCha20(final int[] key, final int[] nonce) {
    int[] state = new int[ChaCha20Base.BLOCK_SIZE_IN_INTS];
    ChaCha20Base.setSigmaAndKey(state, key);
    state[12] = nonce[0];
    state[13] = nonce[1];
    state[14] = nonce[2];
    state[15] = nonce[3];
    ChaCha20Base.shuffleState(state);
    // state[0] = state[0], state[1] = state[1], state[2] = state[2], state[3] = state[3]
    state[4] = state[12];
    state[5] = state[13];
    state[6] = state[14];
    state[7] = state[15];
    return Arrays.copyOf(state, ChaCha20Base.KEY_SIZE_IN_INTS);
  }
}

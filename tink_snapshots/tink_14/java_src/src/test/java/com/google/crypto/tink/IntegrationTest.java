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

package com.google.crypto.tink;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;

import com.google.crypto.tink.config.TinkConfig;
import com.google.crypto.tink.subtle.Random;
import com.google.crypto.tink.testing.TestUtil;
import java.io.File;
import java.security.GeneralSecurityException;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/**
 * Integration tests for Tink and Tinkey.
 */
@RunWith(JUnit4.class)
public class IntegrationTest {
  @BeforeClass
  public static void setUp() throws GeneralSecurityException {
    Config.register(TinkConfig.TINK_1_0_0);
  }

  /**
   * Tests with EciesAesGcmHkdf keys generated by Tinkey.
   */
  @Test
  public void testWithTinkeyEciesAesGcmHkdf() throws Exception {
    if (TestUtil.isAndroid()) {
      System.out.println("testWithTinkeyEciesAesGcmHkdf doesn't work on Android, skipping");
      return;
    }
    HybridDecrypt hybridDecrypt =
        CleartextKeysetHandle.read(
                BinaryKeysetReader.withFile(
                    new File("../tink_base/testdata/ecies_private_keyset2.bin")))
            .getPrimitive(HybridDecrypt.class);

    HybridEncrypt hybridEncrypt =
        CleartextKeysetHandle.read(
                BinaryKeysetReader.withFile(
                    new File("../tink_base/testdata/ecies_public_keyset2.bin")))
            .getPrimitive(HybridEncrypt.class);

    byte[] plaintext = Random.randBytes(20);
    byte[] contextInfo = Random.randBytes(20);
    byte[] ciphertext = hybridEncrypt.encrypt(plaintext, contextInfo);
    assertEquals(
        CryptoFormat.TINK_PREFIX_SIZE
        + 65 /* ephemeral key */
        + 12 /* IV */
        + plaintext.length
        + 16 /* tag length */,
        ciphertext.length);
    assertArrayEquals(plaintext, hybridDecrypt.decrypt(ciphertext, contextInfo));
  }

  /**
   * Tests with EciesAesCtrHmacAead keys generated by Tinkey.
   */
  @Test
  public void testWithTinkeyEciesAesCtrHmacAead() throws Exception {
    if (TestUtil.isAndroid()) {
      System.out.println("testWithTinkeyEciesAesGcmHkdf doesn't work on Android, skipping");
      return;
    }

    HybridDecrypt hybridDecrypt =
        CleartextKeysetHandle.read(
                BinaryKeysetReader.withFile(
                    new File("../tink_base/testdata/ecies_private_keyset.bin")))
            .getPrimitive(HybridDecrypt.class);

    HybridEncrypt hybridEncrypt =
        CleartextKeysetHandle.read(
                BinaryKeysetReader.withFile(
                    new File("../tink_base/testdata/ecies_public_keyset.bin")))
            .getPrimitive(HybridEncrypt.class);

    byte[] plaintext = Random.randBytes(20);
    byte[] contextInfo = Random.randBytes(20);
    byte[] ciphertext = hybridEncrypt.encrypt(plaintext, contextInfo);
    assertEquals(
        CryptoFormat.TINK_PREFIX_SIZE
        + 65 /* ephemeral key */
        + 16 /* IV */
        + plaintext.length
        + 16 /* tag length */,
        ciphertext.length);
    assertArrayEquals(plaintext, hybridDecrypt.decrypt(ciphertext, contextInfo));
  }
}

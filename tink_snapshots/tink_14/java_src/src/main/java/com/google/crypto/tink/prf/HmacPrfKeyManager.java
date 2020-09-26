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

package com.google.crypto.tink.prf;

import com.google.crypto.tink.KeyTemplate;
import com.google.crypto.tink.KeyTypeManager;
import com.google.crypto.tink.Registry;
import com.google.crypto.tink.proto.HashType;
import com.google.crypto.tink.proto.HmacPrfKey;
import com.google.crypto.tink.proto.HmacPrfKeyFormat;
import com.google.crypto.tink.proto.HmacPrfParams;
import com.google.crypto.tink.proto.KeyData.KeyMaterialType;
import com.google.crypto.tink.subtle.PrfHmacJce;
import com.google.crypto.tink.subtle.Random;
import com.google.crypto.tink.subtle.Validators;
import com.google.crypto.tink.subtle.prf.SingletonPrfSet;
import com.google.protobuf.ByteString;
import com.google.protobuf.ExtensionRegistryLite;
import com.google.protobuf.InvalidProtocolBufferException;
import java.io.IOException;
import java.io.InputStream;
import java.security.GeneralSecurityException;
import javax.crypto.spec.SecretKeySpec;

/**
 * This key manager generates new {@code HmacPrfKey} keys and produces new instances of {@code
 * PrfHmacJce}.
 */
public final class HmacPrfKeyManager extends KeyTypeManager<HmacPrfKey> {
  public HmacPrfKeyManager() {
    super(
        HmacPrfKey.class,
        new PrimitiveFactory<PrfSet, HmacPrfKey>(PrfSet.class) {
          @Override
          public PrfSet getPrimitive(HmacPrfKey key) throws GeneralSecurityException {
            HashType hash = key.getParams().getHash();
            byte[] keyValue = key.getKeyValue().toByteArray();
            SecretKeySpec keySpec = new SecretKeySpec(keyValue, "HMAC");
            switch (hash) {
              case SHA1:
                return new SingletonPrfSet(new PrfHmacJce("HMACSHA1", keySpec));
              case SHA256:
                return new SingletonPrfSet(new PrfHmacJce("HMACSHA256", keySpec));
              case SHA512:
                return new SingletonPrfSet(new PrfHmacJce("HMACSHA512", keySpec));
              default:
                throw new GeneralSecurityException("unknown hash");
            }
          }
        });
  }

  /** Minimum key size in bytes. */
  private static final int MIN_KEY_SIZE_IN_BYTES = 16;

  @Override
  public String getKeyType() {
    return "type.googleapis.com/google.crypto.tink.HmacPrfKey";
  }

  @Override
  public int getVersion() {
    return 0;
  }

  @Override
  public KeyMaterialType keyMaterialType() {
    return KeyMaterialType.SYMMETRIC;
  }

  @Override
  public void validateKey(HmacPrfKey key) throws GeneralSecurityException {
    Validators.validateVersion(key.getVersion(), getVersion());
    if (key.getKeyValue().size() < MIN_KEY_SIZE_IN_BYTES) {
      throw new GeneralSecurityException("key too short");
    }
    validateParams(key.getParams());
  }

  @Override
  public HmacPrfKey parseKey(ByteString byteString) throws InvalidProtocolBufferException {
    return HmacPrfKey.parseFrom(byteString, ExtensionRegistryLite.getEmptyRegistry());
  }

  private static void validateParams(HmacPrfParams params) throws GeneralSecurityException {
    if (params.getHash() != HashType.SHA1
        && params.getHash() != HashType.SHA256
        && params.getHash() != HashType.SHA512) {
      throw new GeneralSecurityException("unknown hash type");
    }
  }

  @Override
  public KeyFactory<HmacPrfKeyFormat, HmacPrfKey> keyFactory() {
    return new KeyFactory<HmacPrfKeyFormat, HmacPrfKey>(HmacPrfKeyFormat.class) {
      @Override
      public void validateKeyFormat(HmacPrfKeyFormat format) throws GeneralSecurityException {
        if (format.getKeySize() < MIN_KEY_SIZE_IN_BYTES) {
          throw new GeneralSecurityException("key too short");
        }
        validateParams(format.getParams());
      }

      @Override
      public HmacPrfKeyFormat parseKeyFormat(ByteString byteString)
          throws InvalidProtocolBufferException {
        return HmacPrfKeyFormat.parseFrom(byteString, ExtensionRegistryLite.getEmptyRegistry());
      }

      @Override
      public HmacPrfKey createKey(HmacPrfKeyFormat format) {
        return HmacPrfKey.newBuilder()
            .setVersion(getVersion())
            .setParams(format.getParams())
            .setKeyValue(ByteString.copyFrom(Random.randBytes(format.getKeySize())))
            .build();
      }

      @Override
      public HmacPrfKey deriveKey(HmacPrfKeyFormat format, InputStream inputStream)
          throws GeneralSecurityException {
        Validators.validateVersion(format.getVersion(), getVersion());
        byte[] pseudorandomness = new byte[format.getKeySize()];
        try {
          int read = inputStream.read(pseudorandomness);
          if (read != format.getKeySize()) {
            throw new GeneralSecurityException("Not enough pseudorandomness given");
          }
          return HmacPrfKey.newBuilder()
              .setVersion(getVersion())
              .setParams(format.getParams())
              .setKeyValue(ByteString.copyFrom(pseudorandomness))
              .build();
        } catch (IOException e) {
          throw new GeneralSecurityException("Reading pseudorandomness failed", e);
        }
      }
    };
  }

  public static void register(boolean newKeyAllowed) throws GeneralSecurityException {
    Registry.registerKeyManager(new HmacPrfKeyManager(), newKeyAllowed);
  }

  /**
   * Returns a {@link KeyTemplate} that generates new instances of HMAC keys with the following
   * parameters:
   *
   * <ul>
   *   <li>Key size: 32 bytes
   *   <li>Hash function: SHA256
   *   <li>Prefix type: {@link KeyTemplate.OutputPrefixType#RAW}
   * </ul>
   *
   */
  public static final KeyTemplate hmacSha256Template() {
    return createTemplate(32, HashType.SHA256);
  }

  /**
   * Returns a {@link KeyTemplate} that generates new instances of HMAC keys with the following
   * parameters:
   *
   * <ul>
   *   <li>Key size: 64 bytes
   *   <li>Hash function: SHA512
   *   <li>Prefix type: {@link KeyTemplate.OutputPrefixType#RAW}
   * </ul>
   *
   */
  public static final KeyTemplate hmacSha512Template() {
    return createTemplate(64, HashType.SHA512);
  }

  /**
   * @return a {@link KeyTemplate} containing a {@link HmacKeyFormat} with some specified
   *     parameters.
   */
  private static KeyTemplate createTemplate(int keySize, HashType hashType) {
    HmacPrfParams params = HmacPrfParams.newBuilder().setHash(hashType).build();
    HmacPrfKeyFormat format =
        HmacPrfKeyFormat.newBuilder().setParams(params).setKeySize(keySize).build();
    return KeyTemplate.create(
        new HmacPrfKeyManager().getKeyType(),
        format.toByteArray(),
        KeyTemplate.OutputPrefixType.RAW);
  }
}

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

package com.google.crypto.tink.mac;

import com.google.crypto.tink.KeyTypeManager;
import com.google.crypto.tink.Mac;
import com.google.crypto.tink.Registry;
import com.google.crypto.tink.proto.AesCmacKey;
import com.google.crypto.tink.proto.AesCmacKeyFormat;
import com.google.crypto.tink.proto.AesCmacParams;
import com.google.crypto.tink.proto.KeyData.KeyMaterialType;
import com.google.crypto.tink.subtle.AesCmac;
import com.google.crypto.tink.subtle.Random;
import com.google.crypto.tink.subtle.Validators;
import com.google.protobuf.ByteString;
import com.google.protobuf.ExtensionRegistryLite;
import com.google.protobuf.InvalidProtocolBufferException;
import java.security.GeneralSecurityException;

/**
 * This key manager generates new {@code AesCmacKey} keys and produces new instances of {@code
 * AesCmac}.
 */
public class AesCmacKeyManager extends KeyTypeManager<AesCmacKey> {

  AesCmacKeyManager() {
    super(
        AesCmacKey.class,
        new PrimitiveFactory<Mac, AesCmacKey>(Mac.class) {
          @Override
          public Mac getPrimitive(AesCmacKey key) throws GeneralSecurityException {
            return new AesCmac(key.getKeyValue().toByteArray(), key.getParams().getTagSize());
          }
        });
  }

  private static final int VERSION = 0;
  private static final int KEY_SIZE_IN_BYTES = 32;
  private static final int MIN_TAG_SIZE_IN_BYTES = 10;
  private static final int MAX_TAG_SIZE_IN_BYTES = 16;

  @Override
  public String getKeyType() {
    return "type.googleapis.com/google.crypto.tink.AesCmacKey";
  }

  @Override
  public int getVersion() {
    return VERSION;
  }

  @Override
  public KeyMaterialType keyMaterialType() {
    return KeyMaterialType.SYMMETRIC;
  }

  @Override
  public void validateKey(AesCmacKey key) throws GeneralSecurityException {
    Validators.validateVersion(key.getVersion(), getVersion());
    validateSize(key.getKeyValue().size());
    validateParams(key.getParams());
  }

  @Override
  public AesCmacKey parseKey(ByteString byteString) throws InvalidProtocolBufferException {
    return AesCmacKey.parseFrom(byteString, ExtensionRegistryLite.getEmptyRegistry());
  }

  private static void validateParams(AesCmacParams params) throws GeneralSecurityException {
    if (params.getTagSize() < MIN_TAG_SIZE_IN_BYTES) {
      throw new GeneralSecurityException("tag size too short");
    }
    if (params.getTagSize() > MAX_TAG_SIZE_IN_BYTES) {
      throw new GeneralSecurityException("tag size too long");
    }
  }

  private static void validateSize(int size) throws GeneralSecurityException {
    if (size != KEY_SIZE_IN_BYTES) {
      throw new GeneralSecurityException("AesCmacKey size wrong, must be 16 bytes");
    }
  }

  @Override
  public KeyFactory<AesCmacKeyFormat, AesCmacKey> keyFactory() {
    return new KeyFactory<AesCmacKeyFormat, AesCmacKey>(AesCmacKeyFormat.class) {
      @Override
      public void validateKeyFormat(AesCmacKeyFormat format) throws GeneralSecurityException {
        validateParams(format.getParams());
        validateSize(format.getKeySize());
      }

      @Override
      public AesCmacKeyFormat parseKeyFormat(ByteString byteString)
          throws InvalidProtocolBufferException {
        return AesCmacKeyFormat.parseFrom(byteString, ExtensionRegistryLite.getEmptyRegistry());
      }

      @Override
      public AesCmacKey createKey(AesCmacKeyFormat format) throws GeneralSecurityException {
        return AesCmacKey.newBuilder()
            .setVersion(VERSION)
            .setKeyValue(ByteString.copyFrom(Random.randBytes(format.getKeySize())))
            .setParams(format.getParams())
            .build();
      }
    };
  }

  public static void register(boolean newKeyAllowed) throws GeneralSecurityException {
    Registry.registerKeyManager(new AesCmacKeyManager(), newKeyAllowed);
  }
}

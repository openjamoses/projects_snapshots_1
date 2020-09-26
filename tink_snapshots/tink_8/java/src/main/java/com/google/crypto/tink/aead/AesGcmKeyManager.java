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

package com.google.crypto.tink.aead;

import com.google.crypto.tink.Aead;
import com.google.crypto.tink.KeyManagerBase;
import com.google.crypto.tink.proto.AesGcmKey;
import com.google.crypto.tink.proto.AesGcmKeyFormat;
import com.google.crypto.tink.proto.KeyData.KeyMaterialType;
import com.google.crypto.tink.subtle.AesGcmJce;
import com.google.crypto.tink.subtle.Random;
import com.google.crypto.tink.subtle.Validators;
import com.google.protobuf.ByteString;
import com.google.protobuf.InvalidProtocolBufferException;
import java.security.GeneralSecurityException;

/**
 * This key manager generates new {@code AesGcmKey} keys and produces new instances of {@code
 * AesGcmJce}.
 */
class AesGcmKeyManager extends KeyManagerBase<Aead, AesGcmKey, AesGcmKeyFormat> {
  public AesGcmKeyManager() {
    super(Aead.class, AesGcmKey.class, AesGcmKeyFormat.class, TYPE_URL);
  }

  private static final int VERSION = 0;

  public static final String TYPE_URL = "type.googleapis.com/google.crypto.tink.AesGcmKey";

  /** @param key {@code AesGcmKey} proto */
  @Override
  protected Aead getPrimitiveFromKey(AesGcmKey key) throws GeneralSecurityException {
    return new AesGcmJce(key.getKeyValue().toByteArray());
  }

  @Override
  protected AesGcmKey newKeyFromFormat(AesGcmKeyFormat format) throws GeneralSecurityException {
    return AesGcmKey.newBuilder()
        .setKeyValue(ByteString.copyFrom(Random.randBytes(format.getKeySize())))
        .setVersion(VERSION)
        .build();
  }

  @Override
  public int getVersion() {
    return VERSION;
  }

  @Override
  protected KeyMaterialType keyMaterialType() {
    return KeyMaterialType.SYMMETRIC;
  }

  @Override
  protected AesGcmKey parseKeyProto(ByteString byteString)
      throws InvalidProtocolBufferException {
    return AesGcmKey.parseFrom(byteString);
  }

  @Override
  protected AesGcmKeyFormat parseKeyFormatProto(ByteString byteString)
      throws InvalidProtocolBufferException {
    return AesGcmKeyFormat.parseFrom(byteString);
  }

  @Override
  protected void validateKey(AesGcmKey key) throws GeneralSecurityException {
    Validators.validateVersion(key.getVersion(), VERSION);
    Validators.validateAesKeySize(key.getKeyValue().size());
  }

  @Override
  protected void validateKeyFormat(AesGcmKeyFormat format) throws GeneralSecurityException {
    Validators.validateAesKeySize(format.getKeySize());
  }
}

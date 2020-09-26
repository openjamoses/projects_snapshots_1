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

import com.google.crypto.tink.KeyManagerBase;
import com.google.crypto.tink.proto.AesCtrKey;
import com.google.crypto.tink.proto.AesCtrKeyFormat;
import com.google.crypto.tink.proto.AesCtrParams;
import com.google.crypto.tink.proto.KeyData.KeyMaterialType;
import com.google.crypto.tink.subtle.AesCtrJceCipher;
import com.google.crypto.tink.subtle.IndCpaCipher;
import com.google.crypto.tink.subtle.Random;
import com.google.crypto.tink.subtle.Validators;
import com.google.protobuf.ByteString;
import com.google.protobuf.InvalidProtocolBufferException;
import java.security.GeneralSecurityException;

/**
 * This key manager generates new {@code AesCtrKey} keys and produces new instances of {@code
 * AesCtrJceCipher}.
 */
class AesCtrKeyManager extends KeyManagerBase<IndCpaCipher, AesCtrKey, AesCtrKeyFormat> {
  public AesCtrKeyManager() {
    super(IndCpaCipher.class, AesCtrKey.class, AesCtrKeyFormat.class, TYPE_URL);
  }

  private static final int VERSION = 0;

  static final String TYPE_URL = "type.googleapis.com/google.crypto.tink.AesCtrKey";

  // In counter mode each message is encrypted with an initialization vector (IV) that must be
  // unique. If one single IV is ever used to encrypt two or more messages, the confidentiality of
  // these messages might be lost. This cipher uses a randomly generated IV for each message. The
  // birthday paradox says that if one encrypts 2^k messages, the probability that the random IV
  // will repeat is roughly 2^{2k - t}, where t is the size in bits of the IV. Thus with 96-bit
  // (12-byte) IV, if one encrypts 2^32 messages the probability of IV collision is less than
  // 2^-33 (i.e., less than one in eight billion).
  private static final int MIN_IV_SIZE_IN_BYTES = 12;

  @Override
  public IndCpaCipher getPrimitiveFromKey(AesCtrKey keyProto) throws GeneralSecurityException {
    return new AesCtrJceCipher(
        keyProto.getKeyValue().toByteArray(), keyProto.getParams().getIvSize());
  }

  @Override
  public AesCtrKey newKeyFromFormat(AesCtrKeyFormat format) throws GeneralSecurityException {
    return AesCtrKey.newBuilder()
        .setParams(format.getParams())
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
  protected AesCtrKey parseKeyProto(ByteString byteString)
      throws InvalidProtocolBufferException {
    return AesCtrKey.parseFrom(byteString);
  }

  @Override
  protected AesCtrKeyFormat parseKeyFormatProto(ByteString byteString)
      throws InvalidProtocolBufferException {
    return AesCtrKeyFormat.parseFrom(byteString);
  }

  @Override
  protected void validateKey(AesCtrKey key) throws GeneralSecurityException {
    Validators.validateVersion(key.getVersion(), VERSION);
    Validators.validateAesKeySize(key.getKeyValue().size());
    validate(key.getParams());
  }

  @Override
  protected void validateKeyFormat(AesCtrKeyFormat format) throws GeneralSecurityException {
    Validators.validateAesKeySize(format.getKeySize());
    validate(format.getParams());
  }

  private void validate(AesCtrParams params) throws GeneralSecurityException {
    if (params.getIvSize() < MIN_IV_SIZE_IN_BYTES || params.getIvSize() > 16) {
      throw new GeneralSecurityException("invalid IV size");
    }
  }
}

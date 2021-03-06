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

package com.google.cloud.crypto.tink.subtle;

import com.google.cloud.crypto.tink.HybridEncrypt;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.ListeningExecutorService;
import com.google.common.util.concurrent.MoreExecutors;
import java.security.GeneralSecurityException;
import java.util.concurrent.Executors;

/**
 * Base class for all HybridEncrypt primitives, implements async methods that are the same
 * for all subclasses.
 */
public abstract class HybridEncryptBase implements HybridEncrypt {
  @Override
  public abstract byte[] encrypt(final byte[] plaintext, final byte[] contextInfo)
      throws GeneralSecurityException;

  @Override
  public ListenableFuture<byte[]> asyncEncrypt(final byte[] plaintext, final byte[] contextInfo) {
    ListeningExecutorService service =
        MoreExecutors.listeningDecorator(Executors.newSingleThreadExecutor());
    return service.submit(() -> encrypt(plaintext, contextInfo));
  }
}

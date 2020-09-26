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

import java.security.GeneralSecurityException;

/**
 * Static methods and registering with the {@link Registry} all instances of {@link
 * com.google.crypto.tink.subtle.prf.StreamingPrf} key types supported in a particular release of
 * Tink.
 */
public final class PrfConfig {
  /**
   * Tries to register with the {@link Registry} all instances of {@link
   * com.google.crypto.tink.KeyManager} needed to handle Prf key types supported in Tink.
   */
  public static void register() throws GeneralSecurityException {
    HkdfPrfKeyManager.register(/*newKeyAllowed=*/ true);
  }

  private PrfConfig() {}
}

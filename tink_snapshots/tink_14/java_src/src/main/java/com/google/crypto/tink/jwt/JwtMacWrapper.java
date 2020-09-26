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

package com.google.crypto.tink.jwt;

import com.google.crypto.tink.PrimitiveSet;
import com.google.crypto.tink.PrimitiveWrapper;
import com.google.crypto.tink.Registry;
import com.google.errorprone.annotations.Immutable;
import java.security.GeneralSecurityException;
import java.util.List;

/**
 * JwtMacWrapper is the implementation of {@link PrimitiveWrapper} for the {@link JwtMac} primitive.
 */
class JwtMacWrapper implements PrimitiveWrapper<JwtMac> {
  private static void validate(PrimitiveSet<JwtMac> primitiveSet) throws GeneralSecurityException {
    if (primitiveSet.getPrimary() == null) {
      throw new GeneralSecurityException("Primitive set has no primary.");
    }
    if (primitiveSet.getAll().size() != 1 || primitiveSet.getRawPrimitives().isEmpty()) {
      throw new GeneralSecurityException("All JWT MAC keys must be raw.");
    }
  }

  @Immutable
  private static class WrappedJwtMac implements JwtMac {
    @SuppressWarnings("Immutable") // We do not mutate the primitive set.
    private final PrimitiveSet<JwtMac> primitives;

    private WrappedJwtMac(PrimitiveSet<JwtMac> primitives) {
      this.primitives = primitives;
    }

    @Override
    public String createCompact(ToBeSignedJwt token) throws GeneralSecurityException {
      return primitives.getPrimary().getPrimitive().createCompact(token);
    }

    @Override
    public Jwt verifyCompact(String compact, JwtValidator validator)
        throws GeneralSecurityException {
      // All JWT keys are raw.
      List<PrimitiveSet.Entry<JwtMac>> entries = primitives.getRawPrimitives();
      for (PrimitiveSet.Entry<JwtMac> entry : entries) {
        try {
          return entry.getPrimitive().verifyCompact(compact, validator);
        } catch (GeneralSecurityException ignored) {
          // Ignored as we want to continue verification with other raw keys.
        }
      }
      // nothing works.
      throw new GeneralSecurityException("invalid MAC");
    }
  }

  JwtMacWrapper() {}

  @Override
  public JwtMac wrap(final PrimitiveSet<JwtMac> primitives) throws GeneralSecurityException {
    validate(primitives);
    return new WrappedJwtMac(primitives);
  }

  @Override
  public Class<JwtMac> getPrimitiveClass() {
    return JwtMac.class;
  }

 public static void register() throws GeneralSecurityException {
    Registry.registerPrimitiveWrapper(new JwtMacWrapper());
  }
}

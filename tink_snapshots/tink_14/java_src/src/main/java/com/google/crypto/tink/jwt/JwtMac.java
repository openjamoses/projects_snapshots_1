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

import com.google.errorprone.annotations.Immutable;
import java.security.GeneralSecurityException;

/**
 * Interface for authenticating and verifying JWT with JWS MAC, as described in RFC 7519 and RFC
 * 7515.
 *
 * <h3>Security guarantees: similar to {@link com.google.crypto.tink.Mac}.</h3>
 */
@Immutable
public interface JwtMac {
  /** Computes a MAC, and encodes the JWT and the tag in the JWS compact serialization format. */
  String createCompact(ToBeSignedJwt token) throws GeneralSecurityException;

  /**
   * Verifies and decodes a JWT in the JWS compact serialization format.
   *
   * <p>The JWT is validated against the rules in {@code validator}. That is, every header or claim
   * in {@code validator} must also be present in the JWT. For example, if {@code validator}
   * contains an {@code iss} claim, the JWT must contain an identical claim. Validation for custom
   * claims are also supported. The JWT can contain claims or headers that are {@code NOT} in the
   * {@code validator}. However, if the JWT contains a list of audiences, the validator must also
   * contain an audience in the list.
   *
   * <p>If the JWT contains timestamp claims such as {@code exp}, {@code iat} or {@code nbf}, they
   * will also be validated. {@code validator} allows to set a clock skew, to deal with small clock
   * differences among different machines.
   *
   * @throws InvalidAlgorithmParameterException if {@code compact} was signed with an invalid
   *     algorithm
   * @throws JwtExpiredException when the token has been expired
   * @throws JwtNotBeforeException when the token can't be used yet
   * @throws JwtInvalidException when the token contains an invalid claim or header
   */
  Jwt verifyCompact(String compact, JwtValidator validator) throws GeneralSecurityException;
}

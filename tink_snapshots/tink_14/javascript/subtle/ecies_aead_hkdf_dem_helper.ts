// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//      http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
////////////////////////////////////////////////////////////////////////////////
import {Aead} from '../aead/internal/aead';

/**
 * A helper for DEM (data encapsulation mechanism) of ECIES-AEAD-HKDF.
 */
export interface EciesAeadHkdfDemHelper {
  /**
   * @return the size of the DEM key in bytes
   */
  getDemKeySizeInBytes(): number;

  /**
   * Creates a new `Aead` primitive that uses the key material given in
   * `demKey`, which must be of length `getDemKeySizeInBytes()`.
   *
   * @param demKey the DEM key.
   * @return the newly created `Aead` primitive.
   */
  getAead(demKey: Uint8Array): Promise<Aead>;
}

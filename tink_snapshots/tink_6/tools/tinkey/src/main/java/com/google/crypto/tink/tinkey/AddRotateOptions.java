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

package com.google.crypto.tink.tinkey;

import com.google.crypto.tink.proto.KeyTemplate;
import org.kohsuke.args4j.Option;

/**
 * Options for add-key or rotate-keyset command.
 */
class AddRotateOptions extends OutOptions {
  @Option(
      name = "--key-template",
      metaVar = "AES128_GCM",
      required = true,
      handler = KeyTemplateHandler.class,
      usage =
          "The key template name. Run list-key-templates to get supported names."
  )
  KeyTemplate keyTemplate;
}

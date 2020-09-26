"""
Dependencies of Tink base.
"""

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

def tink_base_deps():
    """ Loads dependencies of Tink base.

    """

    # ----- Go
    # Release from 2019-10-31
    http_archive(
        name = "io_bazel_rules_go",
        urls = [
            "https://storage.googleapis.com/bazel-mirror/github.com/bazelbuild/rules_go/releases/download/v0.20.2/rules_go-v0.20.2.tar.gz",
            "https://github.com/bazelbuild/rules_go/releases/download/v0.20.2/rules_go-v0.20.2.tar.gz",
        ],
        sha256 = "b9aa86ec08a292b97ec4591cf578e020b35f98e12173bbd4a921f84f583aebd9",
    )

    # Release from 2019-11-07
    http_archive(
        name = "bazel_gazelle",
        urls = [
            "https://storage.googleapis.com/bazel-mirror/github.com/bazelbuild/bazel-gazelle/releases/download/v0.19.1/bazel-gazelle-v0.19.1.tar.gz",
            "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.19.1/bazel-gazelle-v0.19.1.tar.gz",
        ],
        sha256 = "86c6d481b3f7aedc1d60c1c211c6f76da282ae197c3b3160f54bd3a8f847896f",
    )

    #-----------------------------------------------------------------------------
    # Actual tink base deps.
    #-----------------------------------------------------------------------------
    # Basic rules we need to add to bazel.
    if not native.existing_rule("bazel_skylib"):
        # Release from 2019-10-09
        http_archive(
            name = "bazel_skylib",
            url = "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
            sha256 = "97e70364e9249702246c0e9444bccdc4b847bed1eb03c5a3ece4f83dfe6abc44",
        )

    # Google PKI certs for connecting to GCP KMS
    if not native.existing_rule("google_root_pem"):
        http_file(
            name = "google_root_pem",
            executable = 0,
            urls = ["https://pki.goog/roots.pem"],
            sha256 = "7f03c894282e3fc39105466a8ee5055ffd05e79dfd4010360117078afbfa68bd",
        )

    # proto
    # proto_library, cc_proto_library and java_proto_library rules implicitly depend
    # on @com_google_protobuf//:proto, @com_google_protobuf//:cc_toolchain and
    # @com_google_protobuf//:java_toolchain, respectively.
    # This statement defines the @com_google_protobuf repo.
    # Release from 2019-12-02
    if not native.existing_rule("com_google_protobuf"):
        http_archive(
            name = "com_google_protobuf",
            strip_prefix = "protobuf-3.11.1",
            urls = ["https://github.com/google/protobuf/archive/v3.11.1.zip"],
            sha256 = "20e55e7dc9ebbb5800072fff25fd56d7c0a168493ef4652e78910566fa6b45f5",
        )

    # Remote Build Execution
    if not native.existing_rule("bazel_toolchains"):
        # Latest bazel_toolchains package on 2020-02-21
        http_archive(
            name = "bazel_toolchains",
            sha256 = "4d348abfaddbcee0c077fc51bb1177065c3663191588ab3d958f027cbfe1818b",
            strip_prefix = "bazel-toolchains-2.1.0",
            urls = [
                "https://mirror.bazel.build/github.com/bazelbuild/bazel-toolchains/archive/2.1.0.tar.gz",
                "https://github.com/bazelbuild/bazel-toolchains/archive/2.1.0.tar.gz",
            ],
        )
    if not native.existing_rule("wycheproof"):
        # Commit from 2019-12-17
        http_archive(
            name = "wycheproof",
            strip_prefix = "wycheproof-d8ed1ba95ac4c551db67f410c06131c3bc00a97c",
            url = "https://github.com/google/wycheproof/archive/d8ed1ba95ac4c551db67f410c06131c3bc00a97c.zip",
            sha256 = "eb1d558071acf1aa6d677d7f1cabec2328d1cf8381496c17185bd92b52ce7545",
        )

package(default_visibility = ["//tools/build_defs:internal_pkg"])

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "mac_set_wrapper",
    srcs = ["mac_set_wrapper.cc"],
    hdrs = ["mac_set_wrapper.h"],
    visibility = ["//visibility:private"],
    deps = [
        "//cc:crypto_format",
        "//cc:mac",
        "//cc:primitive_set",
        "//cc/util:status",
        "//cc/util:statusor",
        "//proto:tink_cc_proto",
    ],
)

cc_library(
    name = "mac_config",
    srcs = ["mac_config.cc"],
    hdrs = ["mac_config.h"],
    deps = [
        ":mac_catalogue",
        "//cc:config",
        "//cc/util:status",
        "//proto:config_cc_proto",
    ],
)

cc_library(
    name = "mac_catalogue",
    srcs = ["mac_catalogue.cc"],
    hdrs = ["mac_catalogue.h"],
    deps = [
        ":hmac_key_manager",
        "//cc:catalogue",
        "//cc/util:status",
        "//cc/util:strings",
    ],
)

cc_library(
    name = "mac_factory",
    srcs = ["mac_factory.cc"],
    hdrs = ["mac_factory.h"],
    deps = [
        ":mac_set_wrapper",
        "//cc:key_manager",
        "//cc:keyset_handle",
        "//cc:mac",
        "//cc:primitive_set",
        "//cc:registry",
        "//cc/util:status",
        "//cc/util:statusor",
    ],
)

cc_library(
    name = "hmac_key_manager",
    srcs = ["hmac_key_manager.cc"],
    hdrs = ["hmac_key_manager.h"],
    deps = [
        "//cc:key_manager",
        "//cc:mac",
        "//cc/subtle:hmac_boringssl",
        "//cc/subtle:random",
        "//cc/util:errors",
        "//cc/util:status",
        "//cc/util:statusor",
        "//cc/util:validation",
        "//proto:common_cc_proto",
        "//proto:hmac_cc_proto",
        "//proto:tink_cc_proto",
        "@com_google_protobuf//:protobuf_lite",
    ],
)

# tests

cc_test(
    name = "mac_set_wrapper_test",
    size = "small",
    srcs = ["mac_set_wrapper_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = [
        ":mac_set_wrapper",
        "//cc:mac",
        "//cc:primitive_set",
        "//cc/util:status",
        "//cc/util:test_util",
        "//proto:tink_cc_proto",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "mac_catalogue_test",
    size = "small",
    srcs = ["mac_catalogue_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = [
        ":mac_catalogue",
        ":mac_config",
        "//cc:catalogue",
        "//cc:config",
        "//cc/util:status",
        "//cc/util:statusor",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "mac_config_test",
    size = "small",
    srcs = ["mac_config_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = [
        ":mac_config",
        "//cc:catalogue",
        "//cc:config",
        "//cc:mac",
        "//cc:registry",
        "//cc/util:status",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "mac_factory_test",
    size = "small",
    srcs = ["mac_factory_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = [
        ":mac_config",
        ":mac_factory",
        ":mac_set_wrapper",
        "//cc:config",
        "//cc:crypto_format",
        "//cc:keyset_handle",
        "//cc:mac",
        "//cc/mac:hmac_key_manager",
        "//cc/util:status",
        "//cc/util:test_util",
        "//proto:common_cc_proto",
        "//proto:hmac_cc_proto",
        "//proto:tink_cc_proto",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "hmac_key_manager_test",
    size = "small",
    srcs = ["hmac_key_manager_test.cc"],
    copts = ["-Iexternal/gtest/include"],
    deps = [
        ":hmac_key_manager",
        "//cc:mac",
        "//cc/util:status",
        "//cc/util:statusor",
        "//proto:aes_ctr_cc_proto",
        "//proto:aes_ctr_hmac_aead_cc_proto",
        "//proto:common_cc_proto",
        "//proto:hmac_cc_proto",
        "//proto:tink_cc_proto",
        "@com_google_googletest//:gtest_main",
    ],
)

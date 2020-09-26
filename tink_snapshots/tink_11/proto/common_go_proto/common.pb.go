// Copyright 2018 Google Inc.
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

// Code generated by protoc-gen-go. DO NOT EDIT.
// source: third_party/tink/proto/common.proto

package common_go_proto

import (
	fmt "fmt"
	proto "github.com/golang/protobuf/proto"
	math "math"
)

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion3 // please upgrade the proto package

type EllipticCurveType int32

const (
	EllipticCurveType_UNKNOWN_CURVE EllipticCurveType = 0
	EllipticCurveType_NIST_P256     EllipticCurveType = 2
	EllipticCurveType_NIST_P384     EllipticCurveType = 3
	EllipticCurveType_NIST_P521     EllipticCurveType = 4
)

var EllipticCurveType_name = map[int32]string{
	0: "UNKNOWN_CURVE",
	2: "NIST_P256",
	3: "NIST_P384",
	4: "NIST_P521",
}

var EllipticCurveType_value = map[string]int32{
	"UNKNOWN_CURVE": 0,
	"NIST_P256":     2,
	"NIST_P384":     3,
	"NIST_P521":     4,
}

func (x EllipticCurveType) String() string {
	return proto.EnumName(EllipticCurveType_name, int32(x))
}

func (EllipticCurveType) EnumDescriptor() ([]byte, []int) {
	return fileDescriptor_51c37496ff2054f5, []int{0}
}

type EcPointFormat int32

const (
	EcPointFormat_UNKNOWN_FORMAT EcPointFormat = 0
	EcPointFormat_UNCOMPRESSED   EcPointFormat = 1
	EcPointFormat_COMPRESSED     EcPointFormat = 2
	// Like UNCOMPRESSED but without the \x04 prefix. Crunchy uses this format.
	// DO NOT USE unless you are a Crunchy user moving to Tink.
	EcPointFormat_DO_NOT_USE_CRUNCHY_UNCOMPRESSED EcPointFormat = 3
)

var EcPointFormat_name = map[int32]string{
	0: "UNKNOWN_FORMAT",
	1: "UNCOMPRESSED",
	2: "COMPRESSED",
	3: "DO_NOT_USE_CRUNCHY_UNCOMPRESSED",
}

var EcPointFormat_value = map[string]int32{
	"UNKNOWN_FORMAT":                  0,
	"UNCOMPRESSED":                    1,
	"COMPRESSED":                      2,
	"DO_NOT_USE_CRUNCHY_UNCOMPRESSED": 3,
}

func (x EcPointFormat) String() string {
	return proto.EnumName(EcPointFormat_name, int32(x))
}

func (EcPointFormat) EnumDescriptor() ([]byte, []int) {
	return fileDescriptor_51c37496ff2054f5, []int{1}
}

type HashType int32

const (
	HashType_UNKNOWN_HASH HashType = 0
	HashType_SHA1         HashType = 1
	HashType_SHA384       HashType = 2
	HashType_SHA256       HashType = 3
	HashType_SHA512       HashType = 4
)

var HashType_name = map[int32]string{
	0: "UNKNOWN_HASH",
	1: "SHA1",
	2: "SHA384",
	3: "SHA256",
	4: "SHA512",
}

var HashType_value = map[string]int32{
	"UNKNOWN_HASH": 0,
	"SHA1":         1,
	"SHA384":       2,
	"SHA256":       3,
	"SHA512":       4,
}

func (x HashType) String() string {
	return proto.EnumName(HashType_name, int32(x))
}

func (HashType) EnumDescriptor() ([]byte, []int) {
	return fileDescriptor_51c37496ff2054f5, []int{2}
}

func init() {
	proto.RegisterEnum("google.crypto.tink.EllipticCurveType", EllipticCurveType_name, EllipticCurveType_value)
	proto.RegisterEnum("google.crypto.tink.EcPointFormat", EcPointFormat_name, EcPointFormat_value)
	proto.RegisterEnum("google.crypto.tink.HashType", HashType_name, HashType_value)
}

func init() {
	proto.RegisterFile("proto/common.proto", fileDescriptor_51c37496ff2054f5)
}

var fileDescriptor_51c37496ff2054f5 = []byte{
	// 314 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x64, 0x90, 0x41, 0x6b, 0xc2, 0x40,
	0x14, 0x84, 0x35, 0x8a, 0xd8, 0x47, 0x95, 0x75, 0xcf, 0x85, 0x1e, 0xbc, 0x49, 0x49, 0x50, 0x6b,
	0xe9, 0x35, 0xc6, 0x48, 0xac, 0xb8, 0x09, 0xd9, 0xa4, 0xd2, 0x5e, 0x16, 0x4d, 0x25, 0x6e, 0x6b,
	0x7c, 0x61, 0xbb, 0x16, 0xfc, 0x3b, 0xfd, 0xa5, 0xc5, 0x68, 0xb1, 0xc5, 0xdb, 0x7c, 0x30, 0x3b,
	0x3b, 0x6f, 0xa0, 0xad, 0xd7, 0x52, 0xbd, 0x89, 0x7c, 0xa1, 0xf4, 0xde, 0xd2, 0x72, 0xfb, 0x61,
	0xe5, 0x0a, 0x35, 0x5a, 0x09, 0x66, 0x19, 0x6e, 0xcd, 0x02, 0x28, 0x4d, 0x11, 0xd3, 0xcd, 0xca,
	0x4c, 0xd4, 0x3e, 0xd7, 0x68, 0x1e, 0x6c, 0x1d, 0x0e, 0x2d, 0x77, 0xb3, 0x91, 0xb9, 0x96, 0x89,
	0xb3, 0x53, 0x5f, 0xab, 0x68, 0x9f, 0xaf, 0x68, 0x0b, 0x1a, 0x31, 0x9b, 0x32, 0x7f, 0xce, 0x84,
	0x13, 0x87, 0xcf, 0x2e, 0x29, 0xd1, 0x06, 0x5c, 0xb1, 0x09, 0x8f, 0x44, 0xd0, 0x1b, 0x3c, 0x10,
	0xe3, 0x8c, 0xfd, 0xc7, 0x7b, 0x52, 0x39, 0xe3, 0xa0, 0xd7, 0x25, 0xd5, 0xce, 0x3b, 0x34, 0xdc,
	0x24, 0x40, 0xb9, 0xd5, 0x63, 0x54, 0xd9, 0x42, 0x53, 0x0a, 0xcd, 0xdf, 0xc0, 0xb1, 0x1f, 0xce,
	0xec, 0x88, 0x94, 0x28, 0x81, 0xeb, 0x98, 0x39, 0xfe, 0x2c, 0x08, 0x5d, 0xce, 0xdd, 0x11, 0x29,
	0xd3, 0x26, 0xc0, 0x1f, 0x36, 0x68, 0x1b, 0x6e, 0x47, 0xbe, 0x60, 0x7e, 0x24, 0x62, 0xee, 0x0a,
	0x27, 0x8c, 0x99, 0xe3, 0xbd, 0x88, 0x7f, 0x8f, 0x2a, 0x9d, 0x27, 0xa8, 0x7b, 0x8b, 0xcf, 0x75,
	0xd1, 0xbb, 0x88, 0x3c, 0x7e, 0xe3, 0xd9, 0xdc, 0x23, 0x25, 0x5a, 0x87, 0x2a, 0xf7, 0xec, 0x2e,
	0x29, 0x53, 0x80, 0x1a, 0xf7, 0xec, 0x43, 0x5d, 0xe3, 0xa4, 0x0f, 0x97, 0x54, 0x4e, 0x7a, 0xd0,
	0xed, 0x91, 0xea, 0x70, 0x0e, 0x37, 0x09, 0x66, 0xe6, 0xe5, 0x4c, 0xc7, 0x01, 0x83, 0xf2, 0xeb,
	0x5d, 0x2a, 0xf5, 0x7a, 0xb7, 0x34, 0x13, 0xcc, 0xac, 0xa3, 0xed, 0x72, 0x6d, 0x91, 0xa2, 0x28,
	0xf8, 0xdb, 0xa8, 0x45, 0x13, 0x36, 0x0d, 0x86, 0xcb, 0x5a, 0xc1, 0xfd, 0x9f, 0x00, 0x00, 0x00,
	0xff, 0xff, 0x7a, 0x9b, 0x0f, 0x8e, 0xa7, 0x01, 0x00, 0x00,
}

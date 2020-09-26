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
// source: third_party/tink/proto/aes_siv.proto

package aes_siv_go_proto

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

type AesSivKeyFormat struct {
	// Only valid value is: 64.
	KeySize              uint32   `protobuf:"varint,1,opt,name=key_size,json=keySize,proto3" json:"key_size,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *AesSivKeyFormat) Reset()         { *m = AesSivKeyFormat{} }
func (m *AesSivKeyFormat) String() string { return proto.CompactTextString(m) }
func (*AesSivKeyFormat) ProtoMessage()    {}
func (*AesSivKeyFormat) Descriptor() ([]byte, []int) {
	return fileDescriptor_43b3629c0a9079e5, []int{0}
}

func (m *AesSivKeyFormat) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_AesSivKeyFormat.Unmarshal(m, b)
}
func (m *AesSivKeyFormat) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_AesSivKeyFormat.Marshal(b, m, deterministic)
}
func (m *AesSivKeyFormat) XXX_Merge(src proto.Message) {
	xxx_messageInfo_AesSivKeyFormat.Merge(m, src)
}
func (m *AesSivKeyFormat) XXX_Size() int {
	return xxx_messageInfo_AesSivKeyFormat.Size(m)
}
func (m *AesSivKeyFormat) XXX_DiscardUnknown() {
	xxx_messageInfo_AesSivKeyFormat.DiscardUnknown(m)
}

var xxx_messageInfo_AesSivKeyFormat proto.InternalMessageInfo

func (m *AesSivKeyFormat) GetKeySize() uint32 {
	if m != nil {
		return m.KeySize
	}
	return 0
}

// key_type: type.googleapis.com/google.crypto.tink.AesSivKey
type AesSivKey struct {
	Version uint32 `protobuf:"varint,1,opt,name=version,proto3" json:"version,omitempty"`
	// First half is AES-CTR key, second is AES-SIV.
	KeyValue             []byte   `protobuf:"bytes,2,opt,name=key_value,json=keyValue,proto3" json:"key_value,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *AesSivKey) Reset()         { *m = AesSivKey{} }
func (m *AesSivKey) String() string { return proto.CompactTextString(m) }
func (*AesSivKey) ProtoMessage()    {}
func (*AesSivKey) Descriptor() ([]byte, []int) {
	return fileDescriptor_43b3629c0a9079e5, []int{1}
}

func (m *AesSivKey) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_AesSivKey.Unmarshal(m, b)
}
func (m *AesSivKey) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_AesSivKey.Marshal(b, m, deterministic)
}
func (m *AesSivKey) XXX_Merge(src proto.Message) {
	xxx_messageInfo_AesSivKey.Merge(m, src)
}
func (m *AesSivKey) XXX_Size() int {
	return xxx_messageInfo_AesSivKey.Size(m)
}
func (m *AesSivKey) XXX_DiscardUnknown() {
	xxx_messageInfo_AesSivKey.DiscardUnknown(m)
}

var xxx_messageInfo_AesSivKey proto.InternalMessageInfo

func (m *AesSivKey) GetVersion() uint32 {
	if m != nil {
		return m.Version
	}
	return 0
}

func (m *AesSivKey) GetKeyValue() []byte {
	if m != nil {
		return m.KeyValue
	}
	return nil
}

func init() {
	proto.RegisterType((*AesSivKeyFormat)(nil), "google.crypto.tink.AesSivKeyFormat")
	proto.RegisterType((*AesSivKey)(nil), "google.crypto.tink.AesSivKey")
}

func init() {
	proto.RegisterFile("proto/aes_siv.proto", fileDescriptor_43b3629c0a9079e5)
}

var fileDescriptor_43b3629c0a9079e5 = []byte{
	// 218 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xe2, 0x52, 0x29, 0xc9, 0xc8, 0x2c,
	0x4a, 0x89, 0x2f, 0x48, 0x2c, 0x2a, 0xa9, 0xd4, 0x2f, 0xc9, 0xcc, 0xcb, 0xd6, 0x2f, 0x28, 0xca,
	0x2f, 0xc9, 0xd7, 0x4f, 0x4c, 0x2d, 0x8e, 0x2f, 0xce, 0x2c, 0xd3, 0x03, 0xf3, 0x84, 0x84, 0xd2,
	0xf3, 0xf3, 0xd3, 0x73, 0x52, 0xf5, 0x92, 0x8b, 0x2a, 0x0b, 0x4a, 0xf2, 0xf5, 0x40, 0xea, 0x94,
	0x74, 0xb8, 0xf8, 0x1d, 0x53, 0x8b, 0x83, 0x33, 0xcb, 0xbc, 0x53, 0x2b, 0xdd, 0xf2, 0x8b, 0x72,
	0x13, 0x4b, 0x84, 0x24, 0xb9, 0x38, 0xb2, 0x53, 0x2b, 0xe3, 0x8b, 0x33, 0xab, 0x52, 0x25, 0x18,
	0x15, 0x18, 0x35, 0x78, 0x83, 0xd8, 0xb3, 0x53, 0x2b, 0x83, 0x33, 0xab, 0x52, 0x95, 0x9c, 0xb8,
	0x38, 0xe1, 0xaa, 0x85, 0x24, 0xb8, 0xd8, 0xcb, 0x52, 0x8b, 0x8a, 0x33, 0xf3, 0xf3, 0x60, 0xca,
	0xa0, 0x5c, 0x21, 0x69, 0x2e, 0x4e, 0x90, 0x09, 0x65, 0x89, 0x39, 0xa5, 0xa9, 0x12, 0x4c, 0x0a,
	0x8c, 0x1a, 0x3c, 0x41, 0x20, 0x23, 0xc3, 0x40, 0x7c, 0xa7, 0x08, 0x2e, 0x99, 0xe4, 0xfc, 0x5c,
	0x3d, 0x4c, 0xb7, 0x40, 0x5c, 0x19, 0xc0, 0x18, 0xa5, 0x9b, 0x9e, 0x59, 0x92, 0x51, 0x9a, 0xa4,
	0x97, 0x9c, 0x9f, 0xab, 0x0f, 0x51, 0x86, 0xc5, 0x4f, 0xf1, 0xe9, 0xf9, 0xf1, 0x60, 0x81, 0x45,
	0x4c, 0x6c, 0x21, 0x9e, 0x7e, 0xde, 0x01, 0x4e, 0x49, 0x6c, 0x60, 0xbe, 0x31, 0x20, 0x00, 0x00,
	0xff, 0xff, 0x81, 0x45, 0x27, 0x3b, 0x0e, 0x01, 0x00, 0x00,
}

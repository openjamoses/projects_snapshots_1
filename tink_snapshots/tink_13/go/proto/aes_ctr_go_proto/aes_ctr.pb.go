// Copyright 2020 Google Inc.
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
// source: third_party/tink/proto/aes_ctr.proto

package aes_ctr_go_proto

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

type AesCtrParams struct {
	IvSize               uint32   `protobuf:"varint,1,opt,name=iv_size,json=ivSize,proto3" json:"iv_size,omitempty"`
	XXX_NoUnkeyedLiteral struct{} `json:"-"`
	XXX_unrecognized     []byte   `json:"-"`
	XXX_sizecache        int32    `json:"-"`
}

func (m *AesCtrParams) Reset()         { *m = AesCtrParams{} }
func (m *AesCtrParams) String() string { return proto.CompactTextString(m) }
func (*AesCtrParams) ProtoMessage()    {}
func (*AesCtrParams) Descriptor() ([]byte, []int) {
	return fileDescriptor_c4338643e2d19ef2, []int{0}
}

func (m *AesCtrParams) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_AesCtrParams.Unmarshal(m, b)
}
func (m *AesCtrParams) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_AesCtrParams.Marshal(b, m, deterministic)
}
func (m *AesCtrParams) XXX_Merge(src proto.Message) {
	xxx_messageInfo_AesCtrParams.Merge(m, src)
}
func (m *AesCtrParams) XXX_Size() int {
	return xxx_messageInfo_AesCtrParams.Size(m)
}
func (m *AesCtrParams) XXX_DiscardUnknown() {
	xxx_messageInfo_AesCtrParams.DiscardUnknown(m)
}

var xxx_messageInfo_AesCtrParams proto.InternalMessageInfo

func (m *AesCtrParams) GetIvSize() uint32 {
	if m != nil {
		return m.IvSize
	}
	return 0
}

type AesCtrKeyFormat struct {
	Params               *AesCtrParams `protobuf:"bytes,1,opt,name=params,proto3" json:"params,omitempty"`
	KeySize              uint32        `protobuf:"varint,2,opt,name=key_size,json=keySize,proto3" json:"key_size,omitempty"`
	XXX_NoUnkeyedLiteral struct{}      `json:"-"`
	XXX_unrecognized     []byte        `json:"-"`
	XXX_sizecache        int32         `json:"-"`
}

func (m *AesCtrKeyFormat) Reset()         { *m = AesCtrKeyFormat{} }
func (m *AesCtrKeyFormat) String() string { return proto.CompactTextString(m) }
func (*AesCtrKeyFormat) ProtoMessage()    {}
func (*AesCtrKeyFormat) Descriptor() ([]byte, []int) {
	return fileDescriptor_c4338643e2d19ef2, []int{1}
}

func (m *AesCtrKeyFormat) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_AesCtrKeyFormat.Unmarshal(m, b)
}
func (m *AesCtrKeyFormat) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_AesCtrKeyFormat.Marshal(b, m, deterministic)
}
func (m *AesCtrKeyFormat) XXX_Merge(src proto.Message) {
	xxx_messageInfo_AesCtrKeyFormat.Merge(m, src)
}
func (m *AesCtrKeyFormat) XXX_Size() int {
	return xxx_messageInfo_AesCtrKeyFormat.Size(m)
}
func (m *AesCtrKeyFormat) XXX_DiscardUnknown() {
	xxx_messageInfo_AesCtrKeyFormat.DiscardUnknown(m)
}

var xxx_messageInfo_AesCtrKeyFormat proto.InternalMessageInfo

func (m *AesCtrKeyFormat) GetParams() *AesCtrParams {
	if m != nil {
		return m.Params
	}
	return nil
}

func (m *AesCtrKeyFormat) GetKeySize() uint32 {
	if m != nil {
		return m.KeySize
	}
	return 0
}

// key_type: type.googleapis.com/google.crypto.tink.AesCtrKey
type AesCtrKey struct {
	Version              uint32        `protobuf:"varint,1,opt,name=version,proto3" json:"version,omitempty"`
	Params               *AesCtrParams `protobuf:"bytes,2,opt,name=params,proto3" json:"params,omitempty"`
	KeyValue             []byte        `protobuf:"bytes,3,opt,name=key_value,json=keyValue,proto3" json:"key_value,omitempty"`
	XXX_NoUnkeyedLiteral struct{}      `json:"-"`
	XXX_unrecognized     []byte        `json:"-"`
	XXX_sizecache        int32         `json:"-"`
}

func (m *AesCtrKey) Reset()         { *m = AesCtrKey{} }
func (m *AesCtrKey) String() string { return proto.CompactTextString(m) }
func (*AesCtrKey) ProtoMessage()    {}
func (*AesCtrKey) Descriptor() ([]byte, []int) {
	return fileDescriptor_c4338643e2d19ef2, []int{2}
}

func (m *AesCtrKey) XXX_Unmarshal(b []byte) error {
	return xxx_messageInfo_AesCtrKey.Unmarshal(m, b)
}
func (m *AesCtrKey) XXX_Marshal(b []byte, deterministic bool) ([]byte, error) {
	return xxx_messageInfo_AesCtrKey.Marshal(b, m, deterministic)
}
func (m *AesCtrKey) XXX_Merge(src proto.Message) {
	xxx_messageInfo_AesCtrKey.Merge(m, src)
}
func (m *AesCtrKey) XXX_Size() int {
	return xxx_messageInfo_AesCtrKey.Size(m)
}
func (m *AesCtrKey) XXX_DiscardUnknown() {
	xxx_messageInfo_AesCtrKey.DiscardUnknown(m)
}

var xxx_messageInfo_AesCtrKey proto.InternalMessageInfo

func (m *AesCtrKey) GetVersion() uint32 {
	if m != nil {
		return m.Version
	}
	return 0
}

func (m *AesCtrKey) GetParams() *AesCtrParams {
	if m != nil {
		return m.Params
	}
	return nil
}

func (m *AesCtrKey) GetKeyValue() []byte {
	if m != nil {
		return m.KeyValue
	}
	return nil
}

func init() {
	proto.RegisterType((*AesCtrParams)(nil), "google.crypto.tink.AesCtrParams")
	proto.RegisterType((*AesCtrKeyFormat)(nil), "google.crypto.tink.AesCtrKeyFormat")
	proto.RegisterType((*AesCtrKey)(nil), "google.crypto.tink.AesCtrKey")
}

func init() {
	proto.RegisterFile("proto/aes_ctr.proto", fileDescriptor_c4338643e2d19ef2)
}

var fileDescriptor_c4338643e2d19ef2 = []byte{
	// 268 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x94, 0x90, 0x41, 0x4b, 0xc3, 0x30,
	0x14, 0x80, 0x69, 0x85, 0xd6, 0x3d, 0x27, 0x42, 0x2e, 0x56, 0xf4, 0x50, 0x86, 0xe0, 0x2e, 0xa6,
	0xa0, 0x17, 0xaf, 0x4e, 0x10, 0x64, 0x20, 0xa5, 0x8a, 0x88, 0x97, 0x92, 0xd5, 0x67, 0x17, 0xba,
	0xee, 0x95, 0x24, 0x2b, 0x64, 0xf8, 0x6b, 0xfc, 0xa5, 0xd2, 0x74, 0xca, 0xc0, 0x5d, 0x3c, 0x7e,
	0xe1, 0xcb, 0xfb, 0x1e, 0x0f, 0xce, 0xcd, 0x5c, 0xaa, 0xf7, 0xbc, 0x11, 0xca, 0xd8, 0xc4, 0xc8,
	0x65, 0x95, 0x34, 0x8a, 0x0c, 0x25, 0x02, 0x75, 0x5e, 0x18, 0xc5, 0x1d, 0x31, 0x56, 0x12, 0x95,
	0x0b, 0xe4, 0x85, 0xb2, 0x8d, 0x21, 0xde, 0x79, 0xa3, 0x0b, 0x18, 0xde, 0xa2, 0xbe, 0x33, 0x2a,
	0x15, 0x4a, 0xd4, 0x9a, 0x1d, 0x43, 0x28, 0xdb, 0x5c, 0xcb, 0x35, 0x46, 0x5e, 0xec, 0x8d, 0x0f,
	0xb3, 0x40, 0xb6, 0x4f, 0x72, 0x8d, 0xa3, 0x0f, 0x38, 0xea, 0xc5, 0x29, 0xda, 0x7b, 0x52, 0xb5,
	0x30, 0xec, 0x06, 0x82, 0xc6, 0xfd, 0x72, 0xea, 0xc1, 0x55, 0xcc, 0xff, 0x06, 0xf8, 0xf6, 0xf4,
	0x6c, 0xe3, 0xb3, 0x13, 0xd8, 0xaf, 0xd0, 0xf6, 0x19, 0xdf, 0x65, 0xc2, 0x0a, 0xad, 0xeb, 0x7c,
	0xc2, 0xe0, 0xb7, 0xc3, 0x22, 0x08, 0x5b, 0x54, 0x5a, 0xd2, 0x72, 0xb3, 0xcd, 0x0f, 0x6e, 0xb5,
	0xfd, 0x7f, 0xb6, 0x4f, 0x61, 0xd0, 0xb5, 0x5b, 0xb1, 0x58, 0x61, 0xb4, 0x17, 0x7b, 0xe3, 0x61,
	0xd6, 0x2d, 0xf3, 0xd2, 0xf1, 0xe4, 0x15, 0xce, 0x0a, 0xaa, 0x77, 0xcd, 0x72, 0x27, 0x4c, 0xbd,
	0xb7, 0xcb, 0x52, 0x9a, 0xf9, 0x6a, 0xc6, 0x0b, 0xaa, 0x93, 0x5e, 0xdb, 0x71, 0xf0, 0xbc, 0xa4,
	0xdc, 0x3d, 0x7c, 0xf9, 0xc1, 0xf3, 0xc3, 0xe3, 0x34, 0x9d, 0xcc, 0x02, 0xc7, 0xd7, 0xdf, 0x01,
	0x00, 0x00, 0xff, 0xff, 0x94, 0xe7, 0xc9, 0x0e, 0xab, 0x01, 0x00, 0x00,
}

// Copyright (c) CangKui <cangyingzhijia@126.com>
// All rights reserved.
//
// This library is dual-licensed: you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation. For the terms of this
// license, see <http://www.gnu.org/licenses/>.
//
// You are free to use this library under the terms of the GNU General
// Public License, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// Alternatively, you can license this library under a commercial
// license, as set out in <http://cesanta.com/>.
//
// Author: CangKui
// Date: 2013-03-28
//
#ifndef XPROTO_PROTOBUF_MESSAGE_FACTORY_H_
#define XPROTO_PROTOBUF_MESSAGE_FACTORY_H_

#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

namespace xproto {

//
// 动态编译.proto文件,获取proto文件中定义的google::protobuf::Message对象
//
class ProtobufMessageFactory {
 public:
  void AddIncludePath(const std::string& include_dir);

  void Import(const std::string& protoFileName);

  // 返回的message对象，需要使用delete销毁掉
  google::protobuf::Message* NewMessage( const std::string& message_type_name);

  const google::protobuf::ServiceDescriptor* FindServiceByName(const std::string& service_name);

  const google::protobuf::MethodDescriptor* FindMethodByName(const std::string& method_name);

 public:
  static ProtobufMessageFactory& GetInstance();

 private:
  ProtobufMessageFactory();
  virtual ~ProtobufMessageFactory();

 private:
  google::protobuf::compiler::MultiFileErrorCollector* error_collector_;
  google::protobuf::compiler::DiskSourceTree* source_tree_;
  google::protobuf::compiler::Importer* importer_;
  google::protobuf::DynamicMessageFactory* dynamic_message_factory_;
};

}  // namespace xproto

#endif  // XPROTO_PROTOBUF_MESSAGE_FACTORY_H_

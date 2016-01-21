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
#include "xproto/protobuf-message-factory.h"

#include <boost/assert.hpp>
#include "xproto/define.h"

using namespace google::protobuf;  // NOLINT
using namespace google::protobuf::compiler;   // NOLINT

namespace xproto {

class MockErrorCollector : public MultiFileErrorCollector {
 public:
  MockErrorCollector() {}
  ~MockErrorCollector() {}

  void AddError(const std::string& filename, int line, int column,
                const std::string& message) {
    FCOUT("import proto file error# filename:%s, line:%d, column:%d, message:%s", 
          filename.c_str(), line, column, message.c_str());
  }
};

ProtobufMessageFactory::ProtobufMessageFactory() {
  error_collector_ = new MockErrorCollector();
  BOOST_ASSERT(error_collector_ != NULL);
  source_tree_ = new DiskSourceTree();
  BOOST_ASSERT(source_tree_ != NULL);
  importer_ = new Importer(source_tree_, error_collector_);
  BOOST_ASSERT(importer_ != NULL);
  AddIncludePath("/usr/include");
  AddIncludePath("/usr/local/include");
  dynamic_message_factory_ = new DynamicMessageFactory();
  BOOST_ASSERT(dynamic_message_factory_ != NULL);
}

ProtobufMessageFactory::~ProtobufMessageFactory() {
  delete error_collector_;
  delete source_tree_;
  delete dynamic_message_factory_;
  delete importer_;
}

ProtobufMessageFactory& ProtobufMessageFactory::GetInstance() {
  static ProtobufMessageFactory factory;
  return factory;
}

void ProtobufMessageFactory::AddIncludePath(const std::string& include_dir) {
  source_tree_->MapPath("", include_dir);
}

void ProtobufMessageFactory::Import(const std::string& proto_file_name) {
  std::string path = "./";
  std::string fname = proto_file_name;
  if (proto_file_name.find_last_of("/") != string::npos) {
    string::size_type idx = proto_file_name.find_last_of("/");
    path = proto_file_name.substr(0, idx);
    fname = proto_file_name.substr(idx + 1);
  }
  AddIncludePath(path);
  importer_->Import(fname);
}

Message* ProtobufMessageFactory::NewMessage(const std::string& type_name) {
  Message* message = NULL;
  const Descriptor* descriptor = DescriptorPool::generated_pool()
      ->FindMessageTypeByName(type_name);
  if (descriptor) {
    const Message* prototype = MessageFactory::generated_factory()
        ->GetPrototype(descriptor);
    if (prototype) {
      message = prototype->New();
    }
  } else {
    descriptor = importer_->pool()->FindMessageTypeByName(type_name);
    if (descriptor) {
      message = dynamic_message_factory_->GetPrototype(descriptor)->New();
    } else {
      FCOUT("要创建的消息类型不存在:%s，"
            "请检查是否在导入的文件中或在检查拼写是否正确!", 
            type_name.c_str());
    }
  }
  return message;
}

const ServiceDescriptor* ProtobufMessageFactory::FindServiceByName(const std::string& service_name) {
  const ServiceDescriptor* service_descriptor = DescriptorPool::generated_pool()->FindServiceByName(service_name);
  if (!service_descriptor) {
    service_descriptor = importer_->pool()->FindServiceByName(service_name);
  }
  return service_descriptor;
}

const MethodDescriptor* ProtobufMessageFactory::FindMethodByName(const std::string& method_name) {
  const MethodDescriptor* method_descriptor = DescriptorPool::generated_pool()->FindMethodByName(method_name);
  if (!method_descriptor) {
    method_descriptor = importer_->pool()->FindMethodByName(method_name);
  }
  return method_descriptor;
}

}  //  namespace xproto

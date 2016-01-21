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
#include <iostream>
#include "xproto/protobuf-message-factory.h"

using namespace std;  // NOLINT
using namespace xproto;  // NOLINT

int main(){
  ProtobufMessageFactory::GetInstance().AddIncludePath("./");
  ProtobufMessageFactory::GetInstance().Import("mock-server.proto");
  google::protobuf::Message* message = ProtobufMessageFactory::GetInstance().NewMessage("tools.Request");
  const google::protobuf::ServiceDescriptor* sd = ProtobufMessageFactory::GetInstance().FindServiceByName("tools.MockServer");
  cout << "ServiceDescriptor: "<< sd << endl;
  const google::protobuf::MethodDescriptor* method = sd->method(0);
  const google::protobuf::Descriptor* input_descriptor = method->input_type();
  const google::protobuf::Descriptor* output_descriptor = method->output_type();
  cout << method->full_name() <<endl;
  cout << input_descriptor->full_name() <<endl;
  cout << output_descriptor->full_name() <<endl;
  const google::protobuf::MethodDescriptor* method_descriptor = ProtobufMessageFactory::GetInstance().FindMethodByName("tools.MockServer.Query");
  cout << "MathodDescriptor:" << method_descriptor << endl;
  delete message;
  return 0;
}

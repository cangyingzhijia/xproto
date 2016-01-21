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
#include "xproto/protobuf-message-codec.h"

#include <boost/assert.hpp>
#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

#include "define.h"

using namespace google::protobuf;  // NOLINT
using namespace google::protobuf::compiler;  // NOLINT

namespace xproto {

#define SET(message, fieldDesc, reflection, type, value) \
do { \
  if ((fieldDesc)->label() != FieldDescriptor::LABEL_REPEATED) { \
    (reflection)->Set##type((message), (fieldDesc), (value)); \
  } else { \
    (reflection)->Add##type((message), (fieldDesc), (value)); \
  } \
} while (0)


bool ProtobufMessageCodec::FillFieldValue(Message *message, 
                                          const FieldDescriptor* fieldDesc, 
                                          const Reflection* reflection) {
  switch (fieldDesc->type()) {
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_INT64:
      SET(message, fieldDesc, reflection, 
          Int64, fieldDesc->default_value_int64());
      break;
    case FieldDescriptor::TYPE_UINT64:
    case FieldDescriptor::TYPE_FIXED64:
      SET(message, fieldDesc, reflection, 
          UInt64, fieldDesc->default_value_uint64());
      break;
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_INT32:
      SET(message, fieldDesc, reflection, 
          Int32, fieldDesc->default_value_int32());
      break;
    case FieldDescriptor::TYPE_UINT32:
    case FieldDescriptor::TYPE_FIXED32:
      SET(message, fieldDesc, reflection, 
          UInt32, fieldDesc->default_value_uint32());
      break;
    case FieldDescriptor::TYPE_BYTES:
    case FieldDescriptor::TYPE_STRING:
      SET(message, fieldDesc, reflection, 
          String, fieldDesc->default_value_string());
      break;
    case FieldDescriptor::TYPE_DOUBLE:
      SET(message, fieldDesc, reflection, 
          Double, fieldDesc->default_value_double());
      break;
    case FieldDescriptor::TYPE_FLOAT:
      SET(message, fieldDesc, reflection, 
          Float, fieldDesc->default_value_float());
      break;
    case FieldDescriptor::TYPE_BOOL:
      SET(message, fieldDesc, reflection, 
          Bool, fieldDesc->default_value_bool());
      break;
    case FieldDescriptor::TYPE_ENUM:
      SET(message, fieldDesc, reflection, 
          Enum, fieldDesc->default_value_enum());
      break;
    case FieldDescriptor::TYPE_MESSAGE: {
      if (fieldDesc->label() != FieldDescriptor::LABEL_REPEATED) {
        Message* fieldMessage = reflection->MutableMessage(
            message, fieldDesc, NULL);
        FillDefaultValue(fieldMessage);
      } else {
        Message* fieldMessage = reflection->AddMessage(
            message, fieldDesc, NULL);
        FillDefaultValue(fieldMessage);
      }
      break;
    }
    default:
      FCOUT("字段[%s]是不支持的数据类型:%d", 
            fieldDesc->name().c_str(), fieldDesc->type());
  }
  return true;
}

void ProtobufMessageCodec::FillDefaultValue(Message* message) {
  BOOST_ASSERT(message != NULL);
  const Reflection* reflection = message->GetReflection();
  const Descriptor* descriptor = message->GetDescriptor();
  int field_count = descriptor->field_count();
  for (int i = 0; i < field_count; ++ i) {
    FillFieldValue(message, descriptor->field(i), reflection);
  }
}

}  // namespace xproto

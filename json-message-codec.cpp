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
#include "xproto/json-message-codec.h"

#include <sstream>
#include <cstring>
#include <google/protobuf/descriptor.h>
#include <boost/algorithm/string.hpp>

#include "common/utility/lexical-cast.h"
#include "xproto/define.h"

namespace xproto {

using namespace std;  // NOLINT
using namespace google::protobuf;  // NOLINT

#define JSON_GET(reflection, message, descriptor, TYPE, out) \
do {\
  if (descriptor->label() == FieldDescriptor::LABEL_REPEATED) {\
    out += "[" ;\
    int size = reflection->FieldSize(message, descriptor);\
    int i = 0; \
    for (; i< size - 1; ++i) {\
      out += lexical_cast<string>(reflection->GetRepeated##TYPE (message, descriptor, i));\
      out += ",";\
    }\
    if ( i == size - 1){\
      out += lexical_cast<string>(reflection->GetRepeated##TYPE (message, descriptor, i)); \
    }\
    out += "]" ;\
  } else {\
    out += lexical_cast<string>(reflection->Get##TYPE (message, descriptor));\
  }\
} while (0)

#define JSON_SET_INTERGER(reflection, message, descriptor, TYPE, CASTTYPE, CVTF, data, i) \
do {\
  if (descriptor->label() == FieldDescriptor::LABEL_REPEATED) {\
    if (data[i] != '[') { /*不是json数组*/ \
      FCOUT("json字符串不是一个有效的数组:field_name=%s, json=%s",\
            descriptor->name().c_str(), data + i);\
      return false;\
    }\
    ++i; /*跳过'['*/\
    while (data[i]) { \
      char *nptr = data + i;\
      char *endptr = NULL;\
      CASTTYPE to_value = CVTF(nptr, &endptr, 10);\
      i += (endptr - nptr);\
      reflection->Add##TYPE(message, descriptor, to_value);\
      if (*endptr == ',') {\
        ++i;\
      } else if(*endptr == ']') {\
        ++i;\
        break;\
      } else {\
        FCOUT("不是一个有效的整数:%s", nptr);\
        return false;\
      }\
    }\
  } else {\
    char *nptr = data + i;\
    char *endptr = NULL;\
    CASTTYPE to_value = CVTF(nptr, &endptr, 10);\
    i += (endptr - nptr);\
    reflection->Set##TYPE(message, descriptor, to_value);\
  }\
} while (0)

#define JSON_SET_FLOAT(reflection, message, descriptor, TYPE, CASTTYPE, CVTF, data, i) \
do {\
  if (descriptor->label() == FieldDescriptor::LABEL_REPEATED) {\
    if (data[i] != '[') { /*不是json数组*/ \
      FCOUT("json字符串不是一个有效的数组:field_name=%s, json=%s",\
            descriptor->name().c_str(), data + i);\
      return false;\
    }\
    ++i; /*跳过'['*/\
    while (data[i]) { \
      char *nptr = data + i;\
      char *endptr = NULL;\
      CASTTYPE to_value = CVTF(nptr, &endptr);\
      i += (endptr - nptr);\
      reflection->Add##TYPE(message, descriptor, to_value);\
      if (*endptr == ',') {\
        ++i;\
      } else if(*endptr == ']') {\
        ++i;\
        break;\
      } else {\
        FCOUT("不是一个有效的浮点数:%s", nptr);\
        return false;\
      }\
    }\
  } else {\
    char *nptr = data + i;\
    char *endptr = NULL;\
    CASTTYPE to_value = CVTF(nptr, &endptr);\
    i += (endptr - nptr);\
    reflection->Set##TYPE(message, descriptor, to_value);\
  }\
} while (0)

int JsonMessageCodec::FieldToJsonString(const Message &message, 
                                        const FieldDescriptor *field_descriptor, 
                                        const Reflection* reflection, 
                                        std::string &out) {
  if ((field_descriptor->label() == FieldDescriptor::LABEL_REPEATED)) {
    if (reflection->FieldSize(message, field_descriptor) == 0) {
      return 1;
    }
  } else if(! reflection->HasField(message, field_descriptor)) {
    return 1;
  }
  out += "\"" + field_descriptor->name() + "\":";  // output name

  switch (field_descriptor->type()) {
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_INT64:
      JSON_GET(reflection, message, field_descriptor, Int64, out);
      break;
    case FieldDescriptor::TYPE_UINT64:
    case FieldDescriptor::TYPE_FIXED64:
      JSON_GET(reflection, message, field_descriptor, UInt64, out);
      break;
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_INT32:
      JSON_GET(reflection, message, field_descriptor, Int32, out);
      break;
    case FieldDescriptor::TYPE_UINT32:
    case FieldDescriptor::TYPE_FIXED32:
      JSON_GET(reflection, message, field_descriptor, UInt32, out);
      break;
    case FieldDescriptor::TYPE_BYTES:
    case FieldDescriptor::TYPE_STRING: {
      if (field_descriptor->label() == FieldDescriptor::LABEL_REPEATED) {
        out += "[";
        int size = reflection->FieldSize(message, field_descriptor);
        int i = 0;
        for (; i < size - 1; ++i) {
          string value = reflection->GetRepeatedString(
              message, field_descriptor, i);
          boost::replace_all(value, "\"", "\\\"");
          out += "\"" + value + "\"";
          out += ",";
        }
        if (i == size - 1) {
          string value = reflection->GetRepeatedString(
              message, field_descriptor, i);
          boost::replace_all(value, "\"", "\\\"");
          out += "\"" +  value +  "\"";
        }
        out += "]";
      } else {
        string value = reflection->GetString(message, field_descriptor);
        boost::replace_all(value, "\"", "\\\"");
        out += "\"" + value + "\"";
      }
      break;
    }
    case FieldDescriptor::TYPE_DOUBLE:
      JSON_GET(reflection, message, field_descriptor, Double, out);
      break;
    case FieldDescriptor::TYPE_FLOAT:
      JSON_GET(reflection, message, field_descriptor, Float, out);
      break;
    case FieldDescriptor::TYPE_BOOL: {
      if (field_descriptor->label() == FieldDescriptor::LABEL_REPEATED) {
        out += "[";
        int size = reflection->FieldSize(message, field_descriptor);
        int i = 0;
        for (; i < size - 1; ++i) {
          bool value = reflection->GetRepeatedBool(
              message, field_descriptor, i);
          out += (value ? "true" : "false");
          out += ",";
        }
        if (i == size - 1) {
          bool value = reflection->GetRepeatedBool(
              message, field_descriptor, i);
          out += (value ? "true" : "false");
        }
        out += "]";
      } else {
        bool value = reflection->GetBool(message, field_descriptor);
        out += (value ? "true" : "false");
      }
      break;
    }
    case FieldDescriptor::TYPE_ENUM: {
      if (field_descriptor->label() == FieldDescriptor::LABEL_REPEATED) {
        out += "[";
        int size = reflection->FieldSize(message, field_descriptor);
        int i = 0;
        for (; i < size - 1; ++i) {
          const EnumValueDescriptor* value = reflection->GetRepeatedEnum(
              message, field_descriptor, i);
          out += lexical_cast<string>(value->index());
          out += ",";
        }
        if (i == size - 1) {
          const EnumValueDescriptor* value = reflection->GetRepeatedEnum(
              message, field_descriptor, i);
          out += lexical_cast<string>(value->index());
        }
        out += "]";
      } else {
        const EnumValueDescriptor* value = reflection->GetEnum(
            message, field_descriptor);
        out += "\"" + lexical_cast<string>(value->index()) + "\"";
      }
      break;
    }
    case FieldDescriptor::TYPE_MESSAGE: {
      if (field_descriptor->label() != FieldDescriptor::LABEL_REPEATED) {
        const Message &field_message = reflection->GetMessage(message, field_descriptor, NULL);
        ToJsonString(field_message, out);
      } else {
        out += "[";
        int size = reflection->FieldSize(message, field_descriptor);
        for (int i=0; i < size; ++i) {
          const Message &field_message = reflection->GetRepeatedMessage(
              message, field_descriptor, i);
          ToJsonString(field_message, out);
          out += ",";
        }
        if (boost::ends_with(out, ",")) {
          out.erase(out.size() - 1);
        }
        out += "]";
      }
      break;
    }
    default:
      FCOUT("字段[%s]是不支持的数据类型:%d", 
            field_descriptor->name().c_str(), field_descriptor->type());
      return 2;
  }
  return 0;

}

bool JsonMessageCodec::ToString(const google::protobuf::Message &msg,
                                std::string &result) {
  result = "";
  ToJsonString(msg, result);
  return true;
}

void JsonMessageCodec::TemplateString(const google::protobuf::Message &msg, 
                                      std::string &result) {
  google::protobuf::Message * tmp = msg.New();
  if (tmp == NULL) {
    return;
  }
  FillDefaultValue(tmp);
  ToString(*tmp, result);
  if (tmp) {
    delete tmp;
  }
}

void JsonMessageCodec::ToJsonString(const google::protobuf::Message &message,
                                    std::string &out) {
  out += "{";
  const Reflection* reflection = message.GetReflection();
  const Descriptor* descriptor = message.GetDescriptor();
  int field_count = descriptor->field_count();
  int i = 0;
  for (; i < field_count; ++i) {
    int ret = FieldToJsonString(message, descriptor->field(i), reflection, out);
    if(ret == 0){
      out += ",";
    }
  }
  if (boost::ends_with(out, ",")) {
    out.erase(out.size() - 1);
  }
  out += "}";
}

bool JsonMessageCodec::FillFieldValue(char *data, int &i, 
                                      const FieldDescriptor* field_descriptor,
                                      const Reflection* reflection,
                                      Message *message) {
  if (data[i] != ':') {
    return false;
  }
  ++i; // 跳过冒号
  switch (field_descriptor->type()) {
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_INT64:
      JSON_SET_INTERGER(reflection, message, field_descriptor, 
                        Int64, int64, strtoll, data, i);
      break;
    case FieldDescriptor::TYPE_UINT64:
    case FieldDescriptor::TYPE_FIXED64:
      JSON_SET_INTERGER(reflection, message, field_descriptor, 
                        UInt64, uint64, strtoull, data, i);
      break;
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_INT32:
      JSON_SET_INTERGER(reflection, message, field_descriptor, 
                        Int32, uint32, strtol, data, i);
      break;
    case FieldDescriptor::TYPE_UINT32:
    case FieldDescriptor::TYPE_FIXED32:
      JSON_SET_INTERGER(reflection, message, field_descriptor, 
                        UInt32, uint32, strtoul, data, i);
      break;
    case FieldDescriptor::TYPE_BYTES:
    case FieldDescriptor::TYPE_STRING: {
      if (field_descriptor->label() == FieldDescriptor::LABEL_REPEATED) {
        char *cur = data + i;
        if (data[i] != '[') {  // 不是json数组
          FCOUT("json字符串不是一个有效的数组:field_name=%s, json=%s",
                field_descriptor->name().c_str(), data + i);
          return false;
        }
        ++i;  // 跳过'['
        while (data[i]) {
          cur = data + i;
          char expect = data[i++];
          if (expect == ']') {
            break;
          }
          if (expect != '\'' && expect != '"') {
            FCOUT("字符串不是开始于引号: %s", cur);
            return false;
          }
          char *vptr = data + i;  // begin str
          while (data[i]) {
            if (data[i] == expect && data[i - 1] != '\\') {
              ++i;
              break;
            }
            ++i;
          }
          if (!data[i]) {
            FCOUT("字符串不是结束于引号: %s", cur);
            return false;
          }
          string value(vptr, data + (i - 1) - vptr);
          boost::replace_all(value, "\\\"", "\"");
          reflection->AddString(message, field_descriptor, value);
          char *endptr = data + i;
          if (*endptr == ',') {
            ++i;
          } else if (*endptr == ']') {
            ++i;
            break;
          } else {
            return false;
          }
        }
      } else {
        char *cur = data + i;
        char expect = data[i++];
        if (expect != '\'' && expect != '"') {
          FCOUT("字符串不是开始于引号: %s", cur);
          return false;
        }
        char *vptr = data + i;  // begin str
        while (data[i]) {
          if (data[i] == expect && data[i - 1] != '\\') {
            break;
          }
          ++i;
        }
        if (!data[i]) {
          FCOUT("字符串不是结束于引号: %s", cur);
          return false;
        }
        string value(vptr, data + i - vptr);
        boost::replace_all(value, "\\\"", "\"");
        reflection->SetString(message, field_descriptor, value);
        ++i;  // 跳过后引号
      }
      break;
    }
    case FieldDescriptor::TYPE_DOUBLE:
      JSON_SET_FLOAT(reflection, message, field_descriptor,
                     Double, double, strtod, data, i);
      break;
    case FieldDescriptor::TYPE_FLOAT:
      JSON_SET_FLOAT(reflection, message, field_descriptor,
                     Float, float, strtof, data, i);
      break;
    case FieldDescriptor::TYPE_BOOL:{
      if (field_descriptor->label() == FieldDescriptor::LABEL_REPEATED) {
        char *cur = data + i;
        if (data[i] != '[') {  // 不是json数组
          FCOUT("json字符串不是一个有效的数组:field_name=%s, json=%s",
                field_descriptor->name().c_str(), data + i);
          return false;
        }
        ++i;  // 跳过'['
        while (data[i]) {
          cur = data + i;
          bool value = false;
          if (strncasecmp(cur, "true", 4) == 0) {
            value = true;
            i += 4;
          } else if (strncasecmp(cur, "false", 5) == 0) {
            value = false;
            i += 5;
          } else {
            FCOUT("不是一个布尔数: %s", cur);
            return false;
          }
          reflection->AddBool(message, field_descriptor, value);
          if (data[i] == ',') {
            ++i;
          } else if (data[i] == ']') {
            ++i;
            break;
          } else {
            FCOUT("不是有效的布尔数组: %s", cur);
            return false;
          }
        }
      } else {
        char *cur = data + i;
        bool value = false;
        if (strncasecmp(cur, "true", 4) == 0) {
          value = true;
          i += 4;
        } else if (strncasecmp(cur, "false", 5) == 0) {
          value = false;
          i += 5;
        } else {
          FCOUT("不是一个布尔数: %s", cur);
          return false;
        }
        reflection->SetBool(message, field_descriptor, value);
      }
      break;
    }
    case FieldDescriptor::TYPE_ENUM: {
      const EnumDescriptor* enum_desc = field_descriptor->enum_type();
      if (field_descriptor->label() != FieldDescriptor::LABEL_REPEATED) {
        char *kptr = data + i;
        while (LABEL_CHARS[static_cast<uint8_t>(data[i])] || 
               data[i] == '\"' ||
               data[i] == '\'') {
          ++i;
        }
        string key(kptr, data + i - kptr);
        boost::trim_if(key, boost::is_any_of("\"' "));
        const EnumValueDescriptor* enum_value = enum_desc->FindValueByName(key);
        if (enum_value == NULL) {
          enum_value = enum_desc->FindValueByNumber(lexical_cast<int>(key));
          if(enum_value == NULL){
            FCOUT("字段[%s]不存在枚举值[%s]", field_descriptor->name().c_str(), key.c_str());
            return false;
          }
        }
        reflection->SetEnum(message, field_descriptor, enum_value);
      } else {
        if (data[i] != '[') {  // 不是json数组
          FCOUT("json字符串不是一个有效的数组:field_name=%s, json=%s", field_descriptor->name().c_str(), data + i);
          return false;
        }
        ++i; // 跳过'['
        while (data[i]) {
          char *kptr = data + i;
          while (LABEL_CHARS[static_cast<uint8_t>(data[i])]) {
            ++i;
          }
          string key(kptr, data + i - kptr);
          boost::trim_if(key, boost::is_any_of("\"' "));
          const EnumValueDescriptor* enum_value = enum_desc->FindValueByName(key);
          if (enum_value == NULL) { 
            enum_value = enum_desc->FindValueByNumber(lexical_cast<int>(key));
            if(enum_value == NULL){
              FCOUT("字段[%s]不存在枚举值[%s]", field_descriptor->name().c_str(), key.c_str());
              return false;
            }

          }
          reflection->AddEnum(message, field_descriptor, enum_value);
          if (data[i] == ',') {
            ++i;
          } else if (data[i] == ']') {
            ++i;
            break;
          } else {
            return false;
          }
        }
      }
      break;
    }
    case FieldDescriptor::TYPE_MESSAGE: {
      if (field_descriptor->label() != FieldDescriptor::LABEL_REPEATED) {
        Message* field_message = reflection->MutableMessage(message,
                                                            field_descriptor, NULL);
        FromJsonObject(data, i, field_message);
      } else {
        if (data[i] != '[') {  // 不是json数组
          FCOUT("json字符串不是一个有效的数组:field_name=%s, json=%s", field_descriptor->name().c_str(), data + i);
          return false;
        }
        ++i; // 跳过'['
        while (data[i]) {
          Message* field_message = reflection->AddMessage(
              message, field_descriptor, NULL);
          if (!FromJsonObject(data, i, field_message)) {
            return false;
          }
          if (data[i] == ',') {
            ++i;
          } else if (data[i] == ']') {
            ++i;
            break;
          } else {
            return false;
          }
        }
      }
      break;
    }
    default:
      FCOUT("字段[%s]是不支持的数据类型:%d", field_descriptor->name().c_str(), field_descriptor->type());
  }
  return true;
}

bool JsonMessageCodec::FromJsonObject(char *data, int &i, Message *message) {
  if (data[i++] != '{') {
    return false;
  }
  const Reflection* reflection = message->GetReflection();
  const Descriptor* descriptor = message->GetDescriptor();
  while (data[i]) {
    // 循环各个字段
    if (data[i] == '\'' || data[i] == '"') {
      ++i;
    }
    char *nptr = data + i;
    while (LABEL_CHARS[static_cast<uint8_t>(data[i])]) {
      ++i;
    }
    string field_name(nptr, data + i - nptr );
    if(field_name == ""){
      return true;
    }
    const FieldDescriptor* field_descriptor 
        = descriptor->FindFieldByName(field_name);
    if (field_descriptor == NULL) {
      FCOUT("在消息类型[%s]中不存在字段[%s] at:%s",
            descriptor->name().c_str(), field_name.c_str(), nptr);
      return false;
    }

    if (data[i] == '\'' || data[i] == '"') {
      ++i;
    }

    FillFieldValue(data, i, field_descriptor, reflection, message);
    if (data[i++] == '}') {  // 跳过括号及逗号
      break;
    }

  }
  return true;

}

bool JsonMessageCodec::CompactAndCheckString(const string &json, 
                                             string &result) {
  int i = 0;
  int size = json.size();
  const char * from = json.data();
  char *to = &result[0];
  char stack[32] = "";
  int top = 0;
outer:
  while (i < size) {
    char ch = from[i];
    if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r') {
      ++i;
      continue;
    }
    if (ch == '"' || ch == '\'') {
      // string
      *to++ = ch;
      char expect = ch;
      ++i;
      while (i < size) {
        ch = from[i];
        if (ch == expect && from[i - 1] != '\\') {
          *to++ = ch;
          ++i;
          goto outer;
        }
        *to++ = ch;
        ++i;
      }
      if (i == size) {
        // 引号不配对
        return false;
      }
    } else if (ch == '[' || ch == '{') {
      stack[top++] = ch;
    } else if (ch == ']') {
      if (top <= 0 || stack[top - 1] != '[') {
        return false;
      }
      top--;
    } else if (ch == '}') {
      if (top <= 0 || stack[top - 1] != '{') {
        return false;
      }
      top--;
    }
    *to++ = ch;
    ++i;
  }
  if (top != 0) {
    // 括号不配对
    return false;
  }
  return true;
}

bool JsonMessageCodec::FromString(const std::string &json,
                                  google::protobuf::Message &msg) {
  int size = json.size();
  if (size == 0) {
    return false;
  }
  string tmp;
  tmp.reserve(json.size() + 1);
  if (!CompactAndCheckString(json, tmp)) {
    return false;
  }
  int i = 0;
  tmp[size] = '\0';
  return FromJsonObject(&tmp[0], i, &msg);
}

bool JsonMessageCodec::PrettyString(const std::string &json, 
                                    std::string &result) {
  static const char *indent[] = {
    "",
    "  ",
    "    ",
    "      ",
    "        ",
    "          ",
    "            ",
    "              ",
    "                ",
    "                  ",
    "                    ",
    "                      ",
    "                        "
  };
  const int max_indent_length = sizeof(indent) / sizeof(indent[0]);

  int i = 0;
  int size = json.size();
  const char * from = json.data();
  int level = 0;
  bool inBracket = false;
outer:
  while (i < size) {
    char ch = from[i];

    switch (ch) {
      case ' ':
      case '\n':
      case '\r':
      case '\t': {
        ++i;
        break;
      }
      case '"':
      case '\'': {
        //string
        result += ch;
        char expect = ch;
        ++i;
        while (i < size) {
          ch = from[i];
          if (ch == expect && from[i - 1] != '\\') {
            result += ch;
            ++i;
            goto outer;
          }
          result += ch;
          ++i;
        }
        FCOUT("字符串未结尾，或没有引号结尾.");
        break;
      }
      case ',': {
        if (!inBracket) {
          result += ch;
          result += '\n';
          result += indent[level];
        } else {
          result += ch;
          result += " ";
        }
        break;
      }
      case '[':
      case '{': {
        result += ch;
        result += '\n';
        if (++level >= max_indent_length) {
          FCOUT("对象嵌套太深,最大嵌套深度为:%d", max_indent_length);
        }
        result += indent[level];
        inBracket = ch == '[' ? true : false;
        break;
      }
      case ']':
      case '}': {
        result += '\n';
        level--;
        if (level < 0) {
          FCOUT("括号不匹配.");
          return false;
        }
        result += indent[level];
        result += ch;
        inBracket = false;
        break;
      }
      default: {
        result += ch;
      }
    } // end switch
    ++i;

  } // end while

  return true;
}

}  // namespace xproto


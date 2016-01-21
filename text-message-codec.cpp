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
#include "xproto/text-message-codec.h"

#include <google/protobuf/descriptor.h>

#include "common/base/likely.h"
#include "common/utility/lexical-cast.h"
#include "xproto/define.h"

#define SET(reflection, message, descriptor, TYPE, CASTTYPE, value, level) \
do {\
  if (descriptor->label() == FieldDescriptor::LABEL_REPEATED) {\
    char separator = *field_seperator_array_[level];\
    char *token = NULL;\
    foreach_token(token, value, separator){\
      reflection->Add##TYPE (message, descriptor, boost::lexical_cast< CASTTYPE >(token));\
    }\
  } else {\
    reflection->Set##TYPE (message, descriptor, boost::lexical_cast< CASTTYPE >(value));\
  }\
} while (0)

#define SET_NUMBER(reflection, message, descriptor, TYPE, CASTTYPE, FORMAT, value, level) \
do {\
  if (descriptor->label() == FieldDescriptor::LABEL_REPEATED) {\
    char separator = *field_seperator_array_[level];\
    char *token = NULL;\
    foreach_token(token, value, separator) {\
      CASTTYPE to_value;\
      if (sscanf(token, FORMAT, &to_value) == -1) {\
        to_value = 0;\
      }\
      reflection->Add##TYPE (message, descriptor, to_value);\
    }\
  } else {\
    CASTTYPE to_value;\
    if (sscanf(value, FORMAT, &to_value) == -1) {\
      to_value = 0;\
    }\
    reflection->Set##TYPE (message, descriptor, to_value);\
  }\
} while (0)

#define GET(reflection, message, descriptor, TYPE, result, level) \
do {\
  if (descriptor->label() == FieldDescriptor::LABEL_REPEATED) {\
    int size = reflection->FieldSize(message, descriptor);\
    for (int i= 0; i< size; ++i) {\
      result.append(boost::lexical_cast< std::string >(reflection->GetRepeated##TYPE (message, descriptor, i)));\
      result.append(field_seperator_array_[level]);\
    }\
    if (size > 0) {\
      result.erase(result.size() - 1);\
    }\
  } else {\
    result.append(boost::lexical_cast< std::string >(reflection->Get##TYPE (message, descriptor)));\
  }\
} while (0)

#define GET_STRING(reflection, message, descriptor, TYPE, result, level) \
do {\
  if (descriptor->label() == FieldDescriptor::LABEL_REPEATED) {\
    int size = reflection->FieldSize(message, descriptor);\
    for (int i= 0; i< size; ++i) {\
      std::string s = reflection->GetRepeated##TYPE (message, descriptor, i);\
      if (!s.empty()) { \
        std::string&& es_str = EscapeBytes(&(s[0])); \
        result.append(es_str);\
      } \
      result.append(field_seperator_array_[level]);\
    }\
    if (size > 0) {\
      result.erase(result.size() - 1);\
    }\
  } else {\
    std::string s = reflection->Get##TYPE (message, descriptor);\
    if (!s.empty()) { \
      std::string&& es_str = EscapeBytes(&(s[0])); \
      result.append(es_str);\
    } \
  }\
} while (0)

namespace xproto {

using namespace google::protobuf;  // NOLINT

static const char* FIELD_SEPERATOR_ARRAY[] = {
  "\001", "\002", "\003", "\004",
  "\005", "\006", "\007", "\008",
  "\021", "\022", "\023", "\024",
  "\025", "\026", "\027", "\030"
};

static const int MAX_FIELD_LEVEL = sizeof (FIELD_SEPERATOR_ARRAY) / sizeof (FIELD_SEPERATOR_ARRAY[0]);

class ProtobufCodec {
 public:
  // 构造一个protobuf格式与CTRL_A、CTRL_B、CTRL_C...等分隔的文本数据的编码解码器
  ProtobufCodec(){
    for(int i=0; i<MAX_FIELD_LEVEL; i++){
      field_seperator_array_[i] = strdup(FIELD_SEPERATOR_ARRAY[i]);
    }
  }
  virtual ~ProtobufCodec(){
    for(int i=0; i<MAX_FIELD_LEVEL; i++){
      free(field_seperator_array_[i]);
    }
  }
  
  // 将CTRL_A、CTRL_B、CTRL_C...等分隔的文本数据编码成protobuf格式的数据
  // @param data 等待编码的数据
  // @param result 编码后的protobuf格式输出结果
  // @return true成功，false失败
  bool Encode(const std::string &data, Message &result);

  //  将protobuf格式的数据解码为CTRL_A、CTRL_B、CTRL_C...等分隔的文本数据
  //  @param pbMsg 要解析的protobuf格式数据
  //  @param result 解析的结果数据
  //  @return true解析成功，false解析失败
  bool Decode(const Message &pbMsg, std::string &result);

  void SetSeperator(const char *sep_list){
    if(sep_list != NULL){
      int slen = strlen(sep_list);
      int level = 0;
      for(int i=0;level<MAX_FIELD_LEVEL && i<slen; ++i){
        char sep = sep_list[i];
        if(sep == ','){
          continue;
        }
        field_seperator_array_[level] = static_cast<char *>( 
            realloc(field_seperator_array_[level], 2 *sizeof(char)));
        field_seperator_array_[level][0] = sep;
        field_seperator_array_[level][1] = '\0';
        level++;
      }
    }
  }

 private:
  inline bool ReflectionDumpField(const Message &message,
                                  const FieldDescriptor* descriptor,
                                  const Reflection* reflection,
                                  std::string &field_value,
                                  int level);

  inline bool ReflectionFillField(Message *message,
                                  const FieldDescriptor* descriptor,
                                  const Reflection* reflection,
                                  char *field_value,
                                  int level);

  bool ReflectionDumplMessage(const Message &message,
                              std::string &data,
                              int level);

  bool ReflectionFillMessage(Message* message, char *data, int level);

 private:
  char *field_seperator_array_[MAX_FIELD_LEVEL];
  char buff[2*1024*1024];  // 2M
};

static inline std::string EscapeBytes(char* text) {
  std::string result;
  char* ptr = text;
  while (*ptr) {
    char ch = *ptr++;
    switch (ch) {
      case '\001':
        result += "\\^A";
        break;
      case '\002':
        result += "\\^B";
        break;
      case '\003':
        result += "\\^C";
        break;
      case '\004':
        result += "\\^D";
        break;
      case '\005':
        result += "\\^E";
        break;
      case '\006':
        result += "\\^F";
        break;
      case '\007':
        result += "\\^G";
        break;
      case '\010':
        result += "\\^H";
        break;
      case '\021':
        result += "\\^Q";
        break;
      case '\022':
        result += "\\^R";
        break;
      case '\023':
        result += "\\^S";
        break;
      case '\024':
        result += "\\^T";
        break;
      case '\025':
        result += "\\^U";
        break;
      case '\026':
        result != "\\^V";
        break;
      case '\027':
        result += "\\^W";
        break;
      case '\030':
        result += "\\^X";
        break;
      case '\r':
        // result += "\\r";
        break;
      case '\n':
        // result += "\\n";
        break;
      case '\\':
        result += "\\\\";
        break;
      default:
        result += ch;
        break;
    }
  }
  return result;
}

static std::string UnEscapeBytes(char* text) {
  int len = strlen(text);
  char* ptr = text;
  char* endptr = text + len;
  std::string result;
  for (; ptr != endptr; ++ptr) {
    char first = *ptr;
    if (first != '\\') {
      result += first;
      continue;
    }
    // match ^ 
    if (++ptr != endptr) {
      result += first;
      break;
    }
    char second = *ptr;
    if (second == 'n') {
      result += '\n';
      continue;
    } else if (second == 'r') {
      result += '\r';
      continue;
    } else if (second == '\\') {
      result += '\\';
      continue;
    }
    // match ctrl A - H
    if (++ptr != endptr) {
      result += first;
      result += second;
      break;
    }
    char third = *ptr;
    switch (third) {
      case 'A':
        result += '\001';
        break;
      case 'B':
        result += '\002';
        break;
      case 'C':
        result += '\003';
        break;
      case 'D':
        result += '\004';
        break;
      case 'E':
        result += '\005';
        break;
      case 'F':
        result += '\006';
        break;
      case 'G':
        result += '\007';
        break;
      case 'H':
        result += '\010';
        break;
      case 'Q':
        result += '\021';
        break;
      case 'R':
        result += '\022';
        break;
      case 'S':
        result += '\023';
        break;
      case 'T':
        result += '\024';
        break;
      case 'U':
        result += '\025';
        break;
      case 'V':
        result += '\026';
        break;
      case 'W':
        result += '\027';
        break;
      case 'X':
        result += '\030';
        break;
      default:
        result += first;
        result += second;
        result += third;
    }
  }
  return result;
}

inline bool ProtobufCodec::ReflectionFillField(Message *message,
                                               const FieldDescriptor *descriptor, 
                                               const Reflection* reflection,
                                               char *field_value, 
                                               int level) {
  if(descriptor == NULL){
    FCOUT("descriptor is null.");
    return false;
  }
  switch (descriptor->type()) {
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_INT64:
      SET_NUMBER(reflection, message, descriptor,
                 Int64, long long, "%lld",field_value, level);
      break;
    case FieldDescriptor::TYPE_FIXED64:
    case FieldDescriptor::TYPE_UINT64:
      SET_NUMBER(reflection, message, descriptor,
                 UInt64, unsigned long long, "%llu", field_value, level);
      break;
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_INT32:
      SET_NUMBER(reflection, message, descriptor,
                 Int32, int, "%d", field_value, level);
      break;
    case FieldDescriptor::TYPE_FIXED32:
    case FieldDescriptor::TYPE_UINT32:
      SET_NUMBER(reflection, message, descriptor,
                 UInt32, unsigned int, "%u",field_value, level);
      break;
    case FieldDescriptor::TYPE_BYTES:
    case FieldDescriptor::TYPE_STRING: {
      std::string&& result = UnEscapeBytes(field_value);
      SET(reflection, message, descriptor,
          String, string, &result[0], level);
      break;
    }
    case FieldDescriptor::TYPE_DOUBLE:
      SET_NUMBER(reflection, message, descriptor,
                 Double, double, "%lf", field_value, level);
      break;
    case FieldDescriptor::TYPE_FLOAT:
      SET_NUMBER(reflection, message, descriptor,
                 Float, float, "%f", field_value, level);
      break;
    case FieldDescriptor::TYPE_BOOL:
      if (descriptor->label() == FieldDescriptor::LABEL_REPEATED)
      {
        char separator = *field_seperator_array_[level];
        char *token = NULL;
        foreach_token(token, field_value, separator) {
          bool value = strncmp(token, "true", 4) == 0 ? true : false;
          reflection->AddBool(message, descriptor, value);
        }
      }else {
        bool value = strncmp(field_value, "true", 4) == 0 ? true : false;
        reflection->SetBool(message, descriptor, value);
      }
      break;
    case FieldDescriptor::TYPE_MESSAGE:{
      if(descriptor->label() != FieldDescriptor::LABEL_REPEATED){
        Message* fieldMessage = reflection->MutableMessage(
            message, descriptor, NULL);
        ReflectionFillMessage(fieldMessage, field_value, level);
      } else {
        int next_level = level + 1;
        char separator = *field_seperator_array_[level];
        char *token = NULL;
        foreach_token(token, field_value, separator){
          Message* fieldMessage = reflection->AddMessage(
              message, descriptor, NULL);
          ReflectionFillMessage(fieldMessage, token, next_level);
        }
      }
      break;
    }
    case FieldDescriptor::TYPE_ENUM: {
      const EnumDescriptor* enum_desc = descriptor->enum_type();
      if (descriptor->label() == FieldDescriptor::LABEL_REPEATED) {
        char separator = *field_seperator_array_[level];
        char *token = NULL;
        foreach_token(token, field_value, separator) {
          const EnumValueDescriptor* enum_value
              = enum_desc->FindValueByName(token);
          if (enum_value == NULL) {
            enum_value = enum_desc->FindValueByNumber(
                boost::lexical_cast<int>(token));
            if(enum_value == NULL){
              FCOUT("字段[%s]不存在枚举值[%s]", 
                    descriptor->name().c_str(), token);
              return false;
            }
          }
          reflection->AddEnum(message, descriptor, enum_value);
        }
      } else {
        const EnumValueDescriptor* enum_value
            = enum_desc->FindValueByName(field_value);
        if (enum_value == NULL) {
          enum_value = enum_desc->FindValueByNumber(
              boost::lexical_cast<int>(field_value));
          if(enum_value == NULL){
            FCOUT("字段[%s]不存在枚举值[%s]",
                  descriptor->name().c_str(), field_value);
            return false;
          }
        }
        reflection->SetEnum(message, descriptor, enum_value);
      }
      break;
    }
    default:
      FCOUT("字段[%s]是不支持的数据类型:%d",
            descriptor->name().c_str(), descriptor->type());
      return false;
  }
  return true;
}

bool ProtobufCodec::ReflectionFillMessage(Message *message,
                                          char *data, int level) {
  const Reflection* reflection = message->GetReflection();
  const Descriptor* descriptor = message->GetDescriptor();

  int next_level = level + 1;
  int field_count = descriptor->field_count();
  int i=0;
  char separator = *field_seperator_array_[level];
  char *token = NULL;
  foreach_token(token, data, separator){
    if(i>=field_count){
      break;
    }
    if(! ReflectionFillField(message, descriptor->field(i),
                             reflection, token, next_level)){
      return false;
    }
    ++i;
  }
  return true;

}

inline bool ProtobufCodec::ReflectionDumpField(const Message &message,
                                               const FieldDescriptor *descriptor,
                                               const Reflection* reflection,
                                               std::string & field_value,
                                               int level) {
  if(descriptor == NULL){
    FCOUT("descriptor is null.");
    return false;
  }
  switch (descriptor->type()) {
    case FieldDescriptor::TYPE_SFIXED64:
    case FieldDescriptor::TYPE_INT64:
      GET(reflection, message, descriptor, Int64, field_value, level);
      break;
    case FieldDescriptor::TYPE_UINT64:
    case FieldDescriptor::TYPE_FIXED64:
      GET(reflection, message, descriptor, UInt64, field_value, level);
      break;
    case FieldDescriptor::TYPE_SFIXED32:
    case FieldDescriptor::TYPE_INT32:
      GET(reflection, message, descriptor, Int32, field_value, level);
      break;
    case FieldDescriptor::TYPE_UINT32:
    case FieldDescriptor::TYPE_FIXED32:
      GET(reflection, message, descriptor, UInt32, field_value, level);
      break;
    case FieldDescriptor::TYPE_BYTES:
    case FieldDescriptor::TYPE_STRING:
      GET_STRING(reflection, message, descriptor, String, field_value, level);
      break;
    case FieldDescriptor::TYPE_DOUBLE:
      GET(reflection, message, descriptor, Double, field_value, level);
      break;
    case FieldDescriptor::TYPE_FLOAT:
      GET(reflection, message, descriptor, Float, field_value, level);
      break;
    case FieldDescriptor::TYPE_BOOL:
      if (descriptor->label() == FieldDescriptor::LABEL_REPEATED) {
        int size = reflection->FieldSize(message, descriptor);
        for (int i = 0; i < size; ++i) {
          const char *value = reflection->GetRepeatedBool(
              message, descriptor, i) ? "true" : "false";
          field_value.append(value);
          field_value.append(field_seperator_array_[level]);
        }
        if (size > 0) {
          field_value.erase(field_value.size() - 1);
        }
      } else {
        const char *value = reflection->GetBool(
            message, descriptor) ? "true" : "false";
        field_value.append(value);
      }
      break;
    case FieldDescriptor::TYPE_MESSAGE:{
      if (descriptor->label() != FieldDescriptor::LABEL_REPEATED) {
        const Message &fieldMessage = reflection->GetMessage(
            message, descriptor, NULL);
        ReflectionDumplMessage(fieldMessage, field_value, level);
      } else {
        int size = reflection->FieldSize(message, descriptor);
        int next_level = level + 1;
        for (int i = 0; i < size; ++i) {
          const Message &fieldMessage = reflection->GetRepeatedMessage(
              message, descriptor, i);
          ReflectionDumplMessage(fieldMessage, field_value, next_level);
          field_value.append(field_seperator_array_[level]);
        }
        if (size > 0) {
          field_value.erase(field_value.size() - 1);
        }
      }
      break;
    }
    case FieldDescriptor::TYPE_ENUM: {
      if (descriptor->label() == FieldDescriptor::LABEL_REPEATED) {
        int size = reflection->FieldSize(message, descriptor);
        for (int i = 0; i < size; ++i) {
          const EnumValueDescriptor* value = reflection->GetRepeatedEnum(
              message, descriptor, i);
          field_value.append(value->name());
          field_value.append(field_seperator_array_[level]);
        }
        if (size > 0) {
          field_value.erase(field_value.size() - 1);
        }
      } else {
        const EnumValueDescriptor* value = reflection->GetEnum(
            message, descriptor);
        field_value.append(value->name());
      }
      break;
    }
    default:
      FCOUT("字段[%s]是不支持的数据类型:%d",
            descriptor->name().c_str(), descriptor->type());
  }
  return true;
}

bool ProtobufCodec::ReflectionDumplMessage(const Message &message,
                                           std::string & result,
                                           int level) {
  const Reflection* reflection = message.GetReflection();
  const Descriptor* descriptor = message.GetDescriptor();
  int next_level = level + 1;
  int field_count = descriptor->field_count();
  for (int i = 0; i < field_count; ++i) {
    if(! ReflectionDumpField(message, descriptor->field(i), 
                             reflection, result, next_level)) {
      return false;
    }
    result.append(field_seperator_array_[level]);
  }
  if(field_count > 0){
    result.erase(result.size() - 1);
  }
  return true;
}

bool ProtobufCodec::Encode(const std::string &data, Message &result) {
  result.Clear();
  uint32_t data_size = data.size();
  if( data_size >= sizeof(buff)){
    FCOUT("数据大于buffer最大值:data_size=%lu, buffer_size=%lu", 
          data.size(), sizeof(buff));
    return false;
  }
  memcpy(buff, data.c_str(), data_size);
  buff[data_size] = '\0';
  return ReflectionFillMessage(&result, buff, 0);
}

bool ProtobufCodec::Decode(const Message &pbMsg, std::string &result) {
  result.clear();
  return ReflectionDumplMessage(pbMsg, result, 0);
}

TextMessageCodec::TextMessageCodec() {
  SetDefaultSeperatorList();
}

void TextMessageCodec::SetDefaultSeperatorList() {
  sep_list_ = "\001,\002,\003,\004,\005,\006,\007,\008,\009";
}

void TextMessageCodec::SetSeperatorList(const char* sep_list) {
  sep_list_ = sep_list;
}

bool TextMessageCodec::ToString(const google::protobuf::Message &msg, 
                                std::string &result) {
  ProtobufCodec pbc;
  pbc.SetSeperator(sep_list_);
  return pbc.Decode(msg, result);
}

bool TextMessageCodec::FromString(const std::string &text,
                                  google::protobuf::Message &msg) {
  ProtobufCodec pbc;
  pbc.SetSeperator(sep_list_);
  return pbc.Encode(text, msg);
}

bool TextMessageCodec::CompactAndCheckString(const std::string &text,
                                             string &result) {
  result = text;
  return true;
}

bool TextMessageCodec::PrettyString(const std::string &text,
                                    string &result) {
  result = text;
  return true;
}


void TextMessageCodec::TemplateString(const google::protobuf::Message &msg,
                                      std::string &result){
  google::protobuf::Message * tmp = msg.New();
  if (tmp) {
    FillDefaultValue(tmp);
    ToString(*tmp, result);
    delete tmp;
  }
}

}  // namespace xproto

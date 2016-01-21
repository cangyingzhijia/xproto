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
#ifndef XPROTO_PROTOBUF_MESSAGE_CODEC_H_
#define XPROTO_PROTOBUF_MESSAGE_CODEC_H_

#include <string>

#include <google/protobuf/message.h>

namespace xproto {

class ProtobufMessageCodec {
 public:
  virtual ~ProtobufMessageCodec() {};

  virtual bool ToString(const google::protobuf::Message &msg, 
                        std::string &output) = 0;

  virtual bool FromString(const std::string &input, 
                          google::protobuf::Message &msg) = 0;

  virtual bool CompactAndCheckString(const std::string &json, 
                                     std::string &result) = 0;

  virtual bool PrettyString(const std::string &json, 
                            std::string &result) = 0;

  virtual void TemplateString(const google::protobuf::Message &msg,
                              std::string &result) = 0;

 protected: 
  // set default value for message
  void FillDefaultValue(google::protobuf::Message* message);

  bool FillFieldValue(google::protobuf::Message *message, 
                      const google::protobuf::FieldDescriptor* fieldDesc, 
                      const google::protobuf::Reflection* reflection);

};

}  // namespace xproto

#endif  // XPROTO_PROTOBUF_MESSAGE_CODEC_H_

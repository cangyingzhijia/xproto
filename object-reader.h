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
// Date: 2013-03-29
//
#ifndef XPROTO_OBJECT_READER_H_
#define XPROTO_OBJECT_READER_H_

#include <string>

namespace xproto {

//
// 从文件中读取各种格式的数据object,目前实现的有xml、json、text
//
class ObjectReader {
 public:
  ObjectReader(const char *file);
  virtual ~ObjectReader();
  bool Read(std::string &obj);
 protected:
  virtual bool DoRead(std::string &obj) = 0;
 protected:
  FILE * file_;
};

class JsonObjectReader : public ObjectReader {
 public:
  JsonObjectReader(const char *file);
  ~JsonObjectReader();
 protected:
  virtual bool DoRead(std::string &obj);
};

class XmlObjectReader : public ObjectReader {
 public:
  XmlObjectReader(const char *file);
  ~XmlObjectReader();
 protected:
  virtual bool DoRead(std::string &obj);
};

class TextObjectReader : public ObjectReader {
 public:
  TextObjectReader(const char *file);
  ~TextObjectReader();
 protected:
  virtual bool DoRead(std::string &obj);
};

}  // namespace xproto

#endif // XPROTO_OBJECT_READER_H_

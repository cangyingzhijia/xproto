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
#include "xproto/object-reader.h"
#include <boost/assert.hpp>
#include "common/utility/lexical-cast.h"
#include "xproto/define.h"  // FOUT

using namespace std;  // NOLINT

namespace xproto {

ObjectReader::ObjectReader(const char *file) : file_(NULL) {
  BOOST_ASSERT(file != NULL);
  file_ = fopen(file, "r");
  BOOST_ASSERT(file_ != NULL);
}

ObjectReader::~ObjectReader() {
  if (file_) {
    fclose(file_);
    file_ = NULL;
  }
}

bool ObjectReader::Read(std::string &obj) {
  return DoRead(obj);
}

// JsonObjectReader implement
JsonObjectReader::JsonObjectReader(const char *file) :
    ObjectReader(file) {
}

JsonObjectReader::~JsonObjectReader(){
}

bool JsonObjectReader::DoRead(std::string &json_obj) {
  json_obj = "";
  int ch = getc(file_);
  while (ch != EOF && ch != '{') {
    ch = getc(file_);
  }
  int level = 0;
  while (ch != EOF) {
    json_obj += ch;
    if (ch == '{') {
      level++;
    } else if (ch == '}') {
      level--;
    }
    if (level == 0) {
      return true;
    }
    ch = getc(file_);
  }
  return false;
}

// XmlObjectReader implement
XmlObjectReader::XmlObjectReader(const char *file) :
    ObjectReader(file) {
}

XmlObjectReader::~XmlObjectReader() {
}

bool XmlObjectReader::DoRead(std::string &xml_obj) {
  xml_obj.clear();
  int level = 0;
  char ch = getc(file_);
  if (ch == EOF) {
    return false;
  }
  do {
    switch (ch) {
      case '<': {
        int sbegin = xml_obj.size();
        xml_obj += ch;
        ch = getc(file_);
        if (ch == '/') {
          xml_obj += ch;
          ch = getc(file_);
          level--;
        } else {
          level++;
        }
        while (LABEL_CHARS[static_cast<uint8_t>(ch)]) {
          xml_obj += ch;
          ch = getc(file_);
        }
        if (ch != '>') {
          FCOUT("数据错误，不是一个有效的element:%s", &xml_obj[sbegin]);
          return false;
        }
        xml_obj += ch;
        break;
      }
      case '\'':
      case '\"': {
        int sbegin = xml_obj.size();
        xml_obj += ch;
        char expect = ch;
        ch = getc(file_);

        while (ch != EOF) {
          if (ch == expect && xml_obj[xml_obj.size() - 1] != '\\') {
            xml_obj += ch;
            goto next;
          }
          xml_obj += ch;
          ch = getc(file_);

        }
        FCOUT("数据错误，不是一个有效的字符串:%s, ----------- %s", &xml_obj[sbegin], &xml_obj[0]);
        return false;
      }
      default: {
        xml_obj += ch;
      }

    }
    if(level == 0){
      break;
    }

next:
    ch = getc(file_);
  } while (ch != EOF);

  return !xml_obj.empty();
}

// TextObjectReader implement
TextObjectReader::TextObjectReader(const char *file) :
    ObjectReader(file) {
    }

TextObjectReader::~TextObjectReader() {
}

bool TextObjectReader::DoRead(std::string &textObj) {
  textObj.clear();
  char ch = getc(file_);
  while (ch != EOF) {
    if (ch == '\n') {
      break;
    }
    textObj += ch;
    ch = getc(file_);
  }
  return !textObj.empty();
}

}  // namespace xproto

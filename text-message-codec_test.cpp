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

#include "xproto/person.pb.h"
#include "xproto/object-reader.h"
#include "xproto/text-message-codec.h"

using namespace std;  // NOLINT

int main(int argc, char *argv[]) {
  Person p;
  p.set_email("12345@126.com");
  p.set_id(123);
  p.set_name("abc");
  p.set_ismale(true);
  Person_PhoneNumber * phone = p.add_phone();
  phone->set_number("12345678911\"abc");
  phone->set_type(Person_PhoneType_HOME);
  phone->add_sn(1.2);
  phone->add_sn(1.3);
  phone->add_sn(1.4);

  phone = p.add_phone();
  phone->set_number("111111111111");
  phone->set_type(Person_PhoneType_WORK);
  phone->add_sn(2);
  phone->add_sn(3);
  phone->add_sn(4);

  string result;
  xproto::TextMessageCodec codec;
  codec.ToString(p, result);
  cout << result << endl;
  cout << "-------------------------------" << endl;
  Person other;
  codec.FromString(result, other);
  codec.ToString(other, result);
  cout << result << endl;
  cout << "-------TextObjectReader--------" << endl;
  xproto::TextObjectReader Reader("data.txt");
  string textObj;
  codec.SetDefaultSeperatorList();
  while (Reader.Read(textObj)) {
    cout << textObj << endl;
    codec.FromString(textObj, other);
    codec.ToString(other, textObj);
    cout << textObj << endl ;
  }
  return 0;
}

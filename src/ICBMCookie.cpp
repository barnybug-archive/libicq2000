/*
 * ICBM Cookie
 *
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "ICBMCookie.h"

#include <stdlib.h>

namespace ICQ2000 {

  ICBMCookie::ICBMCookie()
    : m_c1(0), m_c2(0) { }

  void ICBMCookie::generate() {
    m_c1 = (unsigned int)(0xffffffff*(rand()/(RAND_MAX+1.0)));
    m_c2 = (unsigned int)(0xffffffff*(rand()/(RAND_MAX+1.0)));
  }

  void ICBMCookie::Parse(Buffer& b) {
    b.setEndianness(Buffer::BIG);
    b >> m_c1
      >> m_c2;
  }

  void ICBMCookie::Output(Buffer& b) const {
    b.setEndianness(Buffer::BIG);
    b << m_c1
      << m_c2;
  }

  bool ICBMCookie::operator==(const ICBMCookie& c) const {
    return (m_c1 == c.m_c1 && m_c2 == c.m_c2);
  }

  ICBMCookie& ICBMCookie::operator=(const ICBMCookie& c) {
    m_c1 = c.m_c1;
    m_c2 = c.m_c2;
    return *this;
  }
 
}

Buffer& operator<<(Buffer& b, const ICQ2000::ICBMCookie& c) { c.Output(b); return b; }

Buffer& operator>>(Buffer& b, ICQ2000::ICBMCookie& c) { c.Parse(b); return b; }

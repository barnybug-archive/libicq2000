/*
 * SNAC - New UIN services
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

#include "SNAC-UIN.h"

namespace ICQ2000 {

  // -------------- New UIN (0x0017) Family -----------------------

  UINRequestSNAC::UINRequestSNAC(const string& p)
    : m_password(p) { }

  void UINRequestSNAC::OutputBody(Buffer& b) const{
    b<<(unsigned int)0x00010039;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x28000300;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x624e0000;
    b<<(unsigned int)0x624e0000;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x0;
    b<<(unsigned int)0x0;
    b.setEndianness(Buffer::LITTLE);
    b.PackUint16StringNull(m_password);
    b.setEndianness(Buffer::BIG);
    b<<(unsigned int)0x624e0000;
    b<<(unsigned int)0x0000d601;
  }

  UINResponseSNAC::UINResponseSNAC() { }

  void UINResponseSNAC::ParseBody(Buffer& b){
    b.advance(46);
    b.setEndianness(Buffer::LITTLE);
    b >> m_uin;
    b.advance(10);
  }
  
  UINRequestErrorSNAC::UINRequestErrorSNAC() { }
  
  void UINRequestErrorSNAC::ParseBody(Buffer& b){
    b.advance(32);
  }

}

/*
 * Capabilities
 *
 * Copyright (C) 2002 Barnaby Gray <barnaby@beedesign.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 */

#include <libicq2000/Capabilities.h>

namespace ICQ2000 {

  Capabilities::Capabilities()
  { }
  
  void Capabilities::default_icq2000_capabilities()
  {
    m_data = string("\x09\x46\x13\x49"
		    "\x4c\x7f\x11\xd1"
		    "\x82\x22\x44\x45"
		    "\x53\x54\x00\x00"
		    "\x09\x46\x13\x44"
		    "\x4c\x7f\x11\xd1"
		    "\x82\x22\x44\x45"
		    "\x53\x54\x00\x00", 32);
  }

  void Capabilities::default_icq2002_capabilities()
  {
    m_data = string("\x09\x46\x13\x49"
		    "\x4c\x7f\x11\xd1"
		    "\x82\x22\x44\x45"
		    "\x53\x54\x00\x00"
		    "\x09\x46\x13\x4e"
		    "\x4c\x7f\x11\xd1"
		    "\x82\x22\x44\x45"
		    "\x53\x54\x00\x00"
		    "\x97\xb1\x27\x51"
		    "\x24\x3c\x43\x34"
		    "\xad\x22\xd6\xab"
		    "\xf7\x3f\x14\x92"
		    "\x09\x46\x13\x44"
		    "\x4c\x7f\x11\xd1"
		    "\x82\x22\x44\x45"
		    "\x53\x54\x00\x00", 64);
  }

  void Capabilities::clear()
  {
    m_data.erase();
  }
  
  void Capabilities::Parse(Buffer& b, unsigned short len)
  {
    b.Unpack( m_data, (unsigned int)len );
  }

  void Capabilities::Output(Buffer& b) const
  {
    b.Pack( m_data );
  }
  
  void Capabilities::OutputFirst16(Buffer& b) const
  {
    string s;
    if (m_data.size() < 16) {
      s = string("\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",16);
    } else {
      s = m_data.substr( 0, 16 );
    }
    
    b.Pack( s );
  }

  unsigned short Capabilities::get_length() const
  {
    return (unsigned short)m_data.size();
  }

  bool Capabilities::get_accept_adv_msgs() const
  {
    // ICQ2001 sends out 64 bytes or so
    // ICQ2000 sends out 32 bytes
    // ICQLite sends out 16, so this is what we'll use for now until someone
    // figures out what those capabilities mean
    return (m_data.size() > 16);
  }

}


/*
 * UserInfoBlock
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

#ifndef USERINFOBLOCK_H
#define USERINFOBLOCK_H

#include <string>

#include "buffer.h"

namespace ICQ2000 {
 
  /* the user information block of screenname and then TLVs
   * of user info appears in several different SNACs so
   * encapsulate it here
   */
  class UserInfoBlock {
   protected:
    string m_screenname;
    unsigned short m_warninglevel, m_userClass;
    unsigned char m_allowDirect, m_webAware;
    unsigned short m_status;
    unsigned int m_timeOnline;
    unsigned int m_signupDate, m_signonDate;
    unsigned int m_lan_ip, m_ext_ip;
    unsigned short m_lan_port, m_ext_port, m_firewall;
    unsigned char m_tcp_version;
    
   public:
    UserInfoBlock() { }

    string getScreenName() const;
    unsigned int getUIN() const;
    unsigned int getTimeOnline() const;
    unsigned int getLanIP() const;
    unsigned int getExtIP() const;
    unsigned short getLanPort() const;
    unsigned short getExtPort() const;
    unsigned short getFirewall() const;
    unsigned char getTCPVersion() const;
    unsigned short getStatus() const;

    void Parse(Buffer& b);
  };

}

#endif

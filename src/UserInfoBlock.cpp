/*
 * UserInfoBlock
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

#include "UserInfoBlock.h"

#include "Contact.h"
#include "TLV.h"

namespace ICQ2000 {

  string UserInfoBlock::getScreenName() const { return m_screenname; }

  unsigned int UserInfoBlock::getUIN() const {
    return Contact::StringtoUIN(m_screenname);
  }
  
  unsigned int UserInfoBlock::getTimeOnline() const { return m_timeOnline; }
  unsigned int UserInfoBlock::getLanIP() const { return m_lan_ip; }
  unsigned int UserInfoBlock::getExtIP() const { return m_ext_ip; }
  unsigned short UserInfoBlock::getLanPort() const { return m_lan_port; }
  unsigned short UserInfoBlock::getExtPort() const { return m_ext_port; }
  unsigned short UserInfoBlock::getFirewall() const { return m_firewall; }
  unsigned char UserInfoBlock::getTCPVersion() const { return m_tcp_version; }
  unsigned short UserInfoBlock::getStatus() const { return m_status; }

  void UserInfoBlock::Parse(Buffer& b) {
    // (byte)length, string screenname
    b.UnpackByteString(m_screenname);

    b >> m_warninglevel;
    unsigned short no_tlvs;
    b >> no_tlvs;
    
    TLVList tlvlist;
    tlvlist.Parse(b, TLV_ParseMode_Channel02, no_tlvs);

    m_userClass = 0;
    if (tlvlist.exists(TLV_UserClass)) {
      UserClassTLV *t = (UserClassTLV*)tlvlist[TLV_UserClass];
      m_userClass = t->Value();
    }

    m_status = 0;
    m_allowDirect = 0;
    m_webAware = 0;
    if (tlvlist.exists(TLV_Status)) {
      StatusTLV *t = (StatusTLV*)tlvlist[TLV_Status];
      m_allowDirect = t->getAllowDirect();
      m_webAware = t->getWebAware();
      m_status = t->getStatus();
    }

    m_timeOnline = 0;
    if (tlvlist.exists(TLV_TimeOnline)) {
      TimeOnlineTLV *t = (TimeOnlineTLV*)tlvlist[TLV_TimeOnline];
      m_timeOnline = t->Value();
    }

    m_signupDate = 0;
    if (tlvlist.exists(TLV_SignupDate)) {
      SignupDateTLV *t = (SignupDateTLV*)tlvlist[TLV_SignupDate];
      m_signupDate = t->Value();
    }

    m_signonDate = 0;
    if (tlvlist.exists(TLV_SignonDate)) {
      SignonDateTLV *t = (SignonDateTLV*)tlvlist[TLV_SignonDate];
      m_signonDate = t->Value();
    }

    m_lan_ip = 0;
    m_lan_port = 0;
    m_firewall = 0;
    m_tcp_version = 0;
    if (tlvlist.exists(TLV_LANDetails)) {
      LANDetailsTLV *t = (LANDetailsTLV*)tlvlist[TLV_LANDetails];
      m_lan_ip = t->getLanIP();
      m_lan_port = t->getLanPort();
      m_firewall = t->getFirewall();
      m_tcp_version = t->getTCPVersion();
    }

    m_ext_ip = 0;
    if (tlvlist.exists(TLV_IPAddress)) {
      IPAddressTLV *t = (IPAddressTLV*)tlvlist[TLV_IPAddress];
      m_ext_ip = t->Value();
    }

    m_ext_port = 0;
    if (tlvlist.exists(TLV_Port)) {
      PortTLV *t = (PortTLV*)tlvlist[TLV_Port];
      m_ext_port = t->Value();
    }

  }

}

/*
 * TLVs
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

#include "TLV.h"

namespace ICQ2000 {

  static const unsigned char XORtable[] = { 0xf3, 0x26, 0x81, 0xc4,
					    0x39, 0x86, 0xdb, 0x92,
					    0x71, 0xa3, 0xb9, 0xe6,
					    0x53, 0x7a, 0x95, 0x7c };
  
  // ------------------ Generic TLV ---------------


  InTLV* InTLV::ParseTLV(Buffer& b, TLV_ParseMode parsemode) {
    unsigned short type;
    b >> type;

    InTLV *tlv = NULL;

    switch(parsemode) {

    // ----- CHANNEL 1 -----
    case TLV_ParseMode_Channel01:
      switch(type) {
      case TLV_Screenname:
	tlv = new ScreenNameTLV();
	break;
      case TLV_Cookie:
	tlv = new CookieTLV();
	break;
      }
      break;


    // ----- CHANNEL 2 -----
    case TLV_ParseMode_Channel02:
      switch(type) {
      case TLV_UserClass:
	tlv = new UserClassTLV();
	break;
      case TLV_SignupDate:
	tlv = new SignupDateTLV();
	break;
      case TLV_SignonDate:
	tlv = new SignonDateTLV();
	break;
      case TLV_Status:
	tlv = new StatusTLV();
	break;
      case TLV_WebAddress:
	tlv = new WebAddressTLV();
	break;
      case TLV_TimeOnline:
	tlv = new TimeOnlineTLV();
	break;
      case TLV_LANDetails:
	tlv = new LANDetailsTLV();
	break;
      case TLV_IPAddress:
	tlv = new IPAddressTLV();
	break;
      case TLV_Port:
	tlv = new PortTLV();
	break;
      }
      break;

      // ----- CHANNEL 3 -----
      // todo
      
      // ----- CHANNEL 4 -----
    case TLV_ParseMode_Channel04:
      switch(type) {
      case TLV_Screenname:
	tlv = new ScreenNameTLV();
	break;
      case TLV_Redirect:
	tlv = new RedirectTLV();
	break;
      case TLV_Cookie:
	tlv = new CookieTLV();
	break;
      case TLV_ErrorURL:
	tlv = new ErrorURLTLV();
	break;
      case TLV_ErrorCode:
	tlv = new ErrorCodeTLV();
	break;
      case TLV_DisconnectReason:
	tlv = new DisconnectReasonTLV();
	break;
      case TLV_DisconnectMessage:
	tlv = new DisconnectMessageTLV();
	break;
      }
      break;

      // ----- MESSAGEBLOCK -----
    case TLV_ParseMode_MessageBlock:
      switch(type) {
      case TLV_MessageData:
	tlv = new MessageDataTLV();
	break;
      case TLV_ICQData:
	tlv = new ICQDataTLV();
	break;
      }
      break;

      // ----- ADVMSGBLOCK -------
    case TLV_ParseMode_AdvMsgBlock:
      switch(type) {
      case TLV_AdvMsgData:
	tlv = new AdvMsgDataTLV();
	break;
      }
      break;

      // ----- INMESSAGEDATA -----
    case TLV_ParseMode_InMessageData:
      switch(type) {
      case TLV_MessageText:
	tlv = new MessageTextTLV();
	break;
      }
      break;

    case TLV_ParseMode_InAdvMsgData:
      switch(type) {
      case TLV_AdvMsgBody:
	tlv = new AdvMsgBodyTLV();
	break;
      }
      break;
    }

    if (tlv == NULL) {
      // unrecognised tlv
      // parse as a RawTLV
      tlv = new RawTLV(type);
    }

    tlv->ParseValue(b);

    return tlv;

  }

  void OutTLV::Output(Buffer& b) const {
    OutputHeader(b);
    OutputValue(b);
  }

  void OutTLV::OutputHeader(Buffer& b) const {
    b << Type();
  }

  // ----------------- Base Classes ---------------

  ShortTLV::ShortTLV() { }
  ShortTLV::ShortTLV(unsigned short n) : m_value(n) { }
  void ShortTLV::OutputValue(Buffer& b) const {
    b << Length();
    b << m_value;
  }
  void ShortTLV::ParseValue(Buffer& b) {
    unsigned short l;
    b >> l; // should be 2
    b >> m_value;
  }

  LongTLV::LongTLV() { }
  LongTLV::LongTLV(unsigned int n) : m_value(n) { }
  void LongTLV::OutputValue(Buffer& b) const {
    b << Length();
    b << m_value;
  }
  void LongTLV::ParseValue(Buffer& b) {
    unsigned short l;
    b >> l; // should be 4
    b >> m_value;
  }

  StringTLV::StringTLV() { }
  StringTLV::StringTLV(const string& val) : m_value(val) { }
  void StringTLV::OutputValue(Buffer& b) const {
    b << m_value;
  }
  void StringTLV::ParseValue(Buffer& b) {
    b >> m_value;
  }

  // ----------------- Actual Classes -------------

  // ----------------- ScreenName TLV -------------

  ScreenNameTLV::ScreenNameTLV() { }
  ScreenNameTLV::ScreenNameTLV(const string& val) : StringTLV(val) { }

  // ----------------- Password TLV ---------------
  
  PasswordTLV::PasswordTLV(const string& pw) : m_password(pw) { }
  void PasswordTLV::OutputValue(Buffer& b) const {
    b << (unsigned short)m_password.size();
    for(int i = 0; i < m_password.size(); i++)
      b << (unsigned char)(m_password[i] ^ XORtable[i%16]);

  }

  // ----------------- Capabilities TLV -----------

  void CapabilitiesTLV::OutputValue(Buffer& b) const {
    b << Length();
    b << (unsigned int)0x09461349
      << (unsigned int)0x4c7f11d1
      << (unsigned int)0x82224445
      << (unsigned int)0x53540000
      << (unsigned int)0x09461344
      << (unsigned int)0x4c7f11d1
      << (unsigned int)0x82224445
      << (unsigned int)0x53540000;
  }

  // ----------------- Status TLV -----------------

  void StatusTLV::OutputValue(Buffer& b) const {
    b << Length();
    b << m_allowDirect
      << m_webAware
      << m_status;
  }

  void StatusTLV::ParseValue(Buffer& b) {
    unsigned short l;
    b >> l; // should be 4
    b >> m_allowDirect
      >> m_webAware
      >> m_status;
  }

  // ----------------- Redirect TLV ---------------

  void RedirectTLV::ParseValue(Buffer& b) {
    string hp;
    b >> hp;

    int d = hp.find(':');
    if (d != -1) {
      m_server = hp.substr(0,d);
      m_port = atoi(hp.substr(d+1).c_str());
    } else {
      m_server = hp;
      m_port = 0;
    }
  }

  // ----------------- Cookie TLV -----------------

  CookieTLV::CookieTLV(const unsigned char *ck, unsigned short len)
    : m_length(len)
  {
    m_value = new unsigned char[m_length];
    memcpy(m_value, ck, m_length);
  }

  CookieTLV::~CookieTLV() {
    if (m_value)
      delete [] m_value;
  }

  void CookieTLV::ParseValue(Buffer& b) {
    b >> m_length;

    m_value = new unsigned char[m_length];

    unsigned char c;
    for (unsigned short a = 0; a < m_length; a++) {
      b >> c;
      m_value[a] = c;
    }
  }

  void CookieTLV::OutputValue(Buffer& b) const {
    b  << m_length;
    for (unsigned short a = 0; a < m_length; a++)
      b << m_value[a];
  }

  // ----------------- LAN Details TLV ------------

  LANDetailsTLV::LANDetailsTLV()
    : m_firewall(0x0400), m_tcp_version(7) { }

  LANDetailsTLV::LANDetailsTLV(unsigned int ip, unsigned short port)
    : m_firewall(0x0400), m_tcp_version(7), m_lan_ip(ip), m_lan_port(port) { }

  void LANDetailsTLV::ParseValue(Buffer& b) {
    unsigned short length;
    b >> length;

    if (length == 0x0025) {
      // user accepts direct connections
      b >> m_lan_ip;
      b.advance(2);
      b >> m_lan_port;
    }

    unsigned int waste_int;
    unsigned short waste_short;
    b >> m_firewall
      >> m_tcp_version
      >> waste_int // unknown
      >> waste_int // always 0x00000050
      >> waste_int // always 0x00000003
      >> waste_int // unknown
      >> waste_int // unknown
      >> waste_int // unknown
      >> waste_short; // unknown
  }

  void LANDetailsTLV::OutputValue(Buffer& b) const {
    b << (unsigned short)0x0025;
    b << (unsigned int)m_lan_ip;
    b << (unsigned int)m_lan_port;
    b << m_firewall
      << m_tcp_version
      << (unsigned int)0x279c6996
      << (unsigned int)0x00000050
      << (unsigned int)0x00000003

      //      << (unsigned int)0x3bb9d506
      //      << (unsigned int)0x3bb9d4f9
      //      << (unsigned int)0x3bb9d4fa
      << (unsigned int)0x3AA773EE
      << (unsigned int)0x3AA66380
      << (unsigned int)0x3A877A42


      //      << (unsigned int)0x00000000
      //      << (unsigned int)0x00000000
      //      << (unsigned int)0x00000000
      << (unsigned short)0x0000;
  }

  // ----------------- Raw TLV --------------------

  RawTLV::RawTLV(unsigned short type) : m_type(type) { }


  void RawTLV::ParseValue(Buffer& b) {
    unsigned short length;
    b >> length;
    m_length = length;    
    unsigned char c;
    b.advance(length);
  }

  MessageDataTLV::MessageDataTLV() { }

  void MessageDataTLV::ParseValue(Buffer& b) {
    unsigned short length;
    b >> length;

    /*
     * A list of TLVs inside a TLV, these AOL
     * guys are craaazy..
     */
    TLVList tlvlist;
    tlvlist.Parse(b, TLV_ParseMode_InMessageData, (short unsigned int)-1);

    if (tlvlist.exists(TLV_MessageText))
      mttlv = *(static_cast<MessageTextTLV*>(tlvlist[TLV_MessageText]));

  }

  void MessageTextTLV::ParseValue(Buffer& b) {
    unsigned short length;
    b >> length;
    b >> m_flag1;
    b >> m_flag2;

    b.Unpack(m_message, length-4);
    b.ServerToClient(m_message);
  }

  AdvMsgDataTLV::AdvMsgDataTLV() : m_icqsubtype(NULL) { }

  AdvMsgDataTLV::~AdvMsgDataTLV() {
    if (m_icqsubtype != NULL) delete m_icqsubtype;
  }

  ICQSubType *AdvMsgDataTLV::grabICQSubType() {
    ICQSubType *ret = m_icqsubtype;
    m_icqsubtype = NULL;
    return ret;
  }

  void AdvMsgDataTLV::ParseValue(Buffer& b) {
    unsigned short length;
    b >> length;

    unsigned short type;
    b >> type;
    b.advance(8); // ICBM Cookie again

    b.advance(16); // a capability

    TLVList tlvlist;
    tlvlist.Parse(b, TLV_ParseMode_InAdvMsgData, (short unsigned int)-1);

    if (!tlvlist.exists(TLV_AdvMsgBody))
      throw ParseException("No Advanced Message Body TLV in SNAC 0x0004 0x0007 on channel 2");
    
    AdvMsgBodyTLV *t = static_cast<AdvMsgBodyTLV*>(tlvlist[TLV_AdvMsgBody]);
    m_icqsubtype = t->grabICQSubType();
  }

  AdvMsgBodyTLV::AdvMsgBodyTLV() : m_icqsubtype(NULL) { }

  AdvMsgBodyTLV::~AdvMsgBodyTLV() {
    if (m_icqsubtype != NULL) delete m_icqsubtype;
  }

  ICQSubType *AdvMsgBodyTLV::grabICQSubType() {
    ICQSubType *ret = m_icqsubtype;
    m_icqsubtype = NULL;
    return ret;
  }

  void AdvMsgBodyTLV::ParseValue(Buffer& b) {
    unsigned short length, unknown;
    unsigned char flags;
    b >> length;

    b.advance(27); // unknown

    b.setEndianness(Buffer::LITTLE);
    unsigned short seqnum;
    b >> seqnum
      >> unknown
      >> seqnum; // again

    /* unknown
     * = 0x000e for normal messages
     * = 0x0012 for the weird query ones sent by icq2000 clients through server
     */
    if (unknown != 0x000e && unknown != 0x0012) throw ParseException("Received unknown Server-Message type");

    b.advance(12); // unknown - all zeroes

    m_icqsubtype = ICQSubType::ParseICQSubType(b, true);
    if (m_icqsubtype != NULL) m_icqsubtype->setSeqNum(seqnum);
    
    if (unknown == 0x0012) {
      /* this is a botch, we let the weird messages
       * get parsed as normal messages, then throw it away
       */
      delete m_icqsubtype;
      m_icqsubtype = NULL;
      
    }
  }

  ICQDataTLV::ICQDataTLV() : m_icqsubtype(NULL) { }

  ICQDataTLV::~ICQDataTLV() {
    if (m_icqsubtype != NULL) delete m_icqsubtype;
  }

  ICQSubType* ICQDataTLV::getICQSubType() const { return m_icqsubtype; }

  ICQSubType* ICQDataTLV::grabICQSubType() {
    ICQSubType *ret = m_icqsubtype;
    m_icqsubtype = NULL;
    return ret;
  }

  void ICQDataTLV::ParseValue(Buffer& b) {
    unsigned short length;
    b >> length;

    /* Now this part you can see is where
     * the ICQ folks take over from the AOL folks
     * Intel byte ordering from now on..
     */
    b.setEndianness(Buffer::LITTLE);
    
    /*
     * UIN
     * For SMS - Magic UIN 1002
     */
    unsigned int uin;
    b >> uin;

    m_icqsubtype = ICQSubType::ParseICQSubType(b, false);
    
  }

  // ----------------- TLV List -------------------

  TLVList::TLVList() { }
  TLVList::~TLVList() {
    // delete all elements from hash_map
    hash_map<unsigned short,InTLV*>::iterator i = tlvmap.begin();
    while (i != tlvmap.end()) {
      InTLV *t = (*i).second;
      delete t;
      i++;
    }
    tlvmap.clear();
  }

  void TLVList::Parse(Buffer& b, TLV_ParseMode pm, unsigned short no_tlvs) {
    InTLV *t;
    unsigned short ntlv = 0;
    while (b.beforeEnd() && ntlv < no_tlvs) {
      t = InTLV::ParseTLV(b,pm);
      // duplicate TLVs of one type - this shouldn't happen!
      if (tlvmap.count(t->Type())) {
	delete tlvmap[t->Type()];
      }

      tlvmap[t->Type()] = t;
      ntlv++;
    }
  }

  bool TLVList::exists(unsigned short type) {
    return (tlvmap.count(type) != 0);
  }
  
  InTLV* & TLVList::operator[](unsigned short type) {
    return tlvmap[type];
  }

}

Buffer& operator<<(Buffer& b, const ICQ2000::OutTLV& tlv) { tlv.Output(b); return b; }

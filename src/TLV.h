/*
 * TLVs (Type, Length, Value)
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

#ifndef TLV_H
#define TLV_H

#include <string>

#include <config.h>

#ifdef HAVE_EXT_HASH_MAP
# include <ext/hash_map>
#elif HAVE_HASH_MAP
# include <hash_map>
#else
# error "hash_map not defined"
#endif

#include <string.h>
#include <stdlib.h>

#include "Xml.h"
#include "exceptions.h"
#include "buffer.h"
#include "constants.h"
#include "ICQ.h"

using std::string;
using std::hash_map;

namespace ICQ2000 {
 
  // ------------- TLV numerical constants ------------

  /*
   * TLV types
   * Originally I thought TLV types were distinct within
   * each channel, but in Messages it turns out they are only
   * distinct within each block. To complicate matters you
   * then get TLVs inside TLVs..
   * Solution: the TLV parser must be told what it is expecting
   * to parse so that the correct TLV types are associated
   */

  enum TLV_ParseMode { TLV_ParseMode_Channel01,
		       TLV_ParseMode_Channel02,
		       TLV_ParseMode_Channel04,
		       TLV_ParseMode_MessageBlock,
		       TLV_ParseMode_AdvMsgBlock,
		       TLV_ParseMode_InMessageData,
		       TLV_ParseMode_InAdvMsgData
  };

  // Channel 0x0001
  const unsigned short TLV_Screenname = 0x0001;
  const unsigned short TLV_Password = 0x0002;
  const unsigned short TLV_ClientProfile = 0x0003;
  const unsigned short TLV_UserInfo = 0x0005;
  const unsigned short TLV_Cookie = 0x0006;
  const unsigned short TLV_CountryCode = 0x000e;
  const unsigned short TLV_Language = 0x000f;
  const unsigned short TLV_ClientBuildMinor = 0x0014;
  const unsigned short TLV_ClientType = 0x0016;
  const unsigned short TLV_ClientVersionMajor = 0x0017;
  const unsigned short TLV_ClientVersionMinor = 0x0018;
  const unsigned short TLV_ClientICQNumber = 0x0019;
  const unsigned short TLV_ClientBuildMajor = 0x001a;

  // Channel 0x0002
  const unsigned short TLV_UserClass = 0x0001;
  const unsigned short TLV_SignupDate = 0x0002;
  const unsigned short TLV_SignonDate = 0x0003;
  const unsigned short TLV_Port = 0x0004; // ??
  const unsigned short TLV_Capabilities = 0x0005;
  const unsigned short TLV_Status = 0x0006;
  const unsigned short TLV_Unknown = 0x0008; // ??
  const unsigned short TLV_IPAddress = 0x000a;
  const unsigned short TLV_WebAddress = 0x000b;
  const unsigned short TLV_LANDetails = 0x000c;
  const unsigned short TLV_Unknown000d = 0x000d;
  const unsigned short TLV_TimeOnline = 0x000f;

  // Channel 0x0004
  // const unsigned short TLV_Screenname = 0x0001;
  const unsigned short TLV_ErrorURL = 0x0004;
  const unsigned short TLV_Redirect = 0x0005;
  // const unsigned short TLV_Cookie = 0x0006;
  const unsigned short TLV_ErrorCode = 0x0008;
  const unsigned short TLV_DisconnectReason = 0x0009;
  const unsigned short TLV_DisconnectMessage = 0x000b;
  const unsigned short TLV_Unknown3 = 0x000c;
  const unsigned short TLV_EmailAddress = 0x0011;
  const unsigned short TLV_RegStatus = 0x0013;

  // Message Block
  const unsigned short TLV_MessageData = 0x0002;
  const unsigned short TLV_ServerAckRequested = 0x0003;
  const unsigned short TLV_MessageIsAutoResponse = 0x0004;
  const unsigned short TLV_ICQData = 0x0005;

  // Advanced Message Block
  const unsigned short TLV_AdvMsgData = 0x0005;

  // In Message Data
  const unsigned short TLV_Unknown0501 = 0x0501;
  const unsigned short TLV_MessageText = 0x0101;

  // In Advanced Message Data
  const unsigned short TLV_AdvMsgBody = 0x2711;
  // loads more - but we don't parse them yet

  // ------------- abstract TLV classes ---------------

  class TLV {
   public:
    virtual ~TLV() { }
    
    virtual unsigned short Type() const = 0;
    virtual unsigned short Length() const = 0;
  };

  // -- Inbound TLV --
  class InTLV : public TLV {
   public:
    virtual void ParseValue(Buffer& b) = 0;

    static InTLV* ParseTLV(Buffer& b, TLV_ParseMode pm);
  };

  // -- Outbound TLV --
  class OutTLV : public TLV {
   protected:
    virtual void OutputHeader(Buffer& b) const;
    virtual void OutputValue(Buffer& b) const = 0;

   public:
    virtual void Output(Buffer& b) const;
  };

  // -------------- base classes ----------------------

  class ShortTLV : public OutTLV, public InTLV {
   protected:
    unsigned short m_value;
    
    virtual void OutputValue(Buffer& b) const;

   public:
    ShortTLV();
    ShortTLV(unsigned short n);

    unsigned short Length() const { return 2; }

    virtual void ParseValue(Buffer& b);
    virtual unsigned short Value() const { return m_value; }
  };


  class LongTLV : public OutTLV, public InTLV {
   protected:
    unsigned int m_value;
    
    virtual void OutputValue(Buffer& b) const;

   public:
    LongTLV();
    LongTLV(unsigned int n);
    
    unsigned short Length() const { return 4; }

    virtual void ParseValue(Buffer& b);
    virtual unsigned int Value() const { return m_value; }
  };


  class StringTLV : public OutTLV, public InTLV {
   protected:
    string m_value;

    virtual void OutputValue(Buffer& b) const;

   public:
    StringTLV();
    StringTLV(const string& val);

    unsigned short Length() const { return m_value.size(); }

    virtual void ParseValue(Buffer& b);
    virtual string Value() const { return m_value; }
  };


  // --------------- actual classes -------------------

  class ErrorURLTLV : public StringTLV {
   public:
    ErrorURLTLV() { }
    unsigned short Type() const { return TLV_ErrorURL; }
  };

  class ErrorCodeTLV : public ShortTLV {
   public:
    ErrorCodeTLV() { }
    unsigned short Type() const { return TLV_ErrorCode; }
  };

  class DisconnectReasonTLV : public ShortTLV {
   public:
    DisconnectReasonTLV() { }
    unsigned short Type() const { return TLV_DisconnectReason; }
  };

  class DisconnectMessageTLV : public StringTLV {
   public:
    DisconnectMessageTLV() { }
    unsigned short Type() const { return TLV_DisconnectMessage; }
  };

  class ScreenNameTLV : public StringTLV {
   public:
    ScreenNameTLV();
    ScreenNameTLV(const string& val);

    unsigned short Type() const { return TLV_Screenname; }
  };

  class PasswordTLV : public OutTLV {
   protected:
    string m_password;

    void OutputValue(Buffer& b) const;

   public:
    PasswordTLV(const string& pw);

    unsigned short Type() const { return TLV_Password; }
    unsigned short Length() const { return m_password.size(); }
  };

  const unsigned char ALLOWDIRECT_EVERYONE = 0x00;
  const unsigned char ALLOWDIRECT_AUTHORIZATION = 0x10;
  const unsigned char ALLOWDIRECT_CONTACTLIST = 0x20;

  const unsigned char WEBAWARE_NORMAL = 0x02;
  const unsigned char WEBAWARE_WEBAWARE = 0x03;

  class StatusTLV : public OutTLV, public InTLV {
   private:
    unsigned char m_allowDirect;
    unsigned char m_webAware;
    unsigned short m_status;

   protected:
    void OutputValue(Buffer& b) const;
    void ParseValue(Buffer& b);

   public:
    StatusTLV(unsigned char ad, unsigned char wa, unsigned short st)
      : m_allowDirect(ad), m_webAware(wa), m_status(st)
      { }
    StatusTLV() { }

    unsigned short Type() const { return TLV_Status; }
    unsigned short Length() const { return 4; }

    unsigned char getAllowDirect() { return m_allowDirect; }
    unsigned char getWebAware() { return m_webAware; }
    unsigned short getStatus() { return m_status; }

    void setAllowDirect(unsigned char m) { m_allowDirect = m; }
    void setWebAware(unsigned char m) { m_webAware = m; }
    void setStatus(unsigned short m) { m_status = m; }
  };

  // -- Client*TLVs --

  class ClientProfileTLV : public StringTLV {
   public:
    ClientProfileTLV(const string& val) : StringTLV(val) { }
    unsigned short Type() const { return TLV_ClientProfile; }
  };

  class ClientTypeTLV : public ShortTLV {
   public:
    ClientTypeTLV(unsigned short n) : ShortTLV(n) { }
    unsigned short Type() const { return TLV_ClientType; }
  };

  class ClientVersionMajorTLV : public ShortTLV {
   public:
    ClientVersionMajorTLV(unsigned short n) : ShortTLV(n) { }
    unsigned short Type() const { return TLV_ClientVersionMajor; }
  };

  class ClientVersionMinorTLV : public ShortTLV {
   public:
    ClientVersionMinorTLV(unsigned short n) : ShortTLV(n) { }
    unsigned short Type() const { return TLV_ClientVersionMinor; }
  };

  class ClientICQNumberTLV : public ShortTLV {
   public:
    ClientICQNumberTLV(unsigned short n) : ShortTLV(n) { }
    unsigned short Type() const { return TLV_ClientICQNumber; }
  };

  class ClientBuildMajorTLV : public ShortTLV {
   public:
    ClientBuildMajorTLV(unsigned short n) : ShortTLV(n) { }
    unsigned short Type() const { return TLV_ClientBuildMajor; }
  };

  class ClientBuildMinorTLV : public LongTLV {
   public:
    ClientBuildMinorTLV(unsigned int n) : LongTLV(n) { }
    unsigned short Type() const { return TLV_ClientBuildMinor; }
  };

  class CountryCodeTLV : public StringTLV {
   public:
    CountryCodeTLV(string val) : StringTLV(val) { }
    unsigned short Type() const { return TLV_CountryCode; }
  };

  class LanguageTLV : public StringTLV {
   public:
    LanguageTLV(const string& val) : StringTLV(val) { }
    unsigned short Type() const { return TLV_Language; }
  };

  // --

  class WebAddressTLV : public StringTLV {
   public:
    WebAddressTLV() { }
    unsigned short Type() const { return TLV_WebAddress; }
  };

  class UserClassTLV : public ShortTLV {
   public:
    UserClassTLV() { }
    unsigned short Type() const { return TLV_UserClass; }
  };

  class TimeOnlineTLV : public LongTLV {
   public:
    TimeOnlineTLV() { }
    unsigned short Type() const { return TLV_TimeOnline; }
  };

  class SignupDateTLV : public LongTLV {
   public:
    SignupDateTLV() { }
    unsigned short Type() const { return TLV_SignupDate; }
  };

  class SignonDateTLV : public LongTLV {
   public:
    SignonDateTLV() { }
    unsigned short Type() const { return TLV_SignonDate; }
  };

  class UnknownTLV : public ShortTLV {
   public:
    UnknownTLV() : ShortTLV(0) { }
    unsigned short Type() const { return TLV_Unknown; }
  };

  class IPAddressTLV : public LongTLV {
   public:
    IPAddressTLV() { }
    unsigned short Type() const { return TLV_IPAddress; }
  };

  class PortTLV : public ShortTLV {
   public:
    PortTLV() { }
    unsigned short Type() const { return TLV_Port; }
  };

  class CapabilitiesTLV : public OutTLV {
   public:
    CapabilitiesTLV() { }
    unsigned short Type() const { return TLV_Capabilities; }
    unsigned short Length() const { return 32; }

    void OutputValue(Buffer& b) const;
  };

  class RedirectTLV : public InTLV {
   private:
    string m_server;
    unsigned short m_port;

   public:
    RedirectTLV() { }

    unsigned short Length() const { return m_server.size(); }
    unsigned short Type() const { return TLV_Redirect; }

    void ParseValue(Buffer& b);

    string getHost() { return m_server; }
    unsigned short getPort() { return m_port; }
  };

  class CookieTLV : public InTLV, public OutTLV {
   private:
    unsigned char *m_value;
    unsigned short m_length;

   public:
    CookieTLV() : m_value(NULL), m_length(0) { }
    CookieTLV(const unsigned char *ck, unsigned short len);
    ~CookieTLV();
      
    unsigned short Length() const { return m_length; }
    unsigned short Type() const { return TLV_Cookie; }

    void ParseValue(Buffer& b);
    void OutputValue(Buffer& b) const;

    const unsigned char* Value() { return m_value; }
  };

  // can go out as well
  class LANDetailsTLV : public InTLV, public OutTLV {
   private:
    unsigned int m_lan_ip;
    unsigned short m_lan_port, m_firewall;
    unsigned char m_tcp_version;
    
   public:
    LANDetailsTLV();
    LANDetailsTLV(unsigned int ip, unsigned short port);

    unsigned short Length() const { return 0; } // varies
    unsigned short Type() const { return TLV_LANDetails; }

    unsigned int getLanIP() const { return m_lan_ip; }
    unsigned short getLanPort() const { return m_lan_port; }
    unsigned short getFirewall() const { return m_firewall; }
    unsigned char getTCPVersion() const { return m_tcp_version; }

    void ParseValue(Buffer& b);
    void OutputValue(Buffer& b) const;
  };

  class RawTLV : public InTLV {
   protected:
    unsigned short m_type;
    unsigned short m_length;

   public:
    RawTLV(unsigned short type);

    unsigned short Type() const { return m_type; }
    unsigned short Length() const { return m_length; }
    void ParseValue(Buffer& b);
  };

  class MessageTextTLV : public InTLV {
   protected:
    string m_message;
    unsigned short m_flag1, m_flag2;
    
   public:
    MessageTextTLV()
      : m_message(), m_flag1(0), m_flag2(0) { }

    string getMessage() { return m_message; }
    unsigned short getFlag1() { return m_flag1; }
    unsigned short getFlag2() { return m_flag1; }

    void ParseValue(Buffer& b);
    unsigned short Type() const { return TLV_MessageText; }
    unsigned short Length() const { return 0; }
  };

  class MessageDataTLV : public InTLV {
    MessageTextTLV mttlv;

   public:
    MessageDataTLV();

    string getMessage() { return mttlv.getMessage(); }
    unsigned short getFlag1() { return mttlv.getFlag1(); }
    unsigned short getFlag2() { return mttlv.getFlag2(); }

    void ParseValue(Buffer& b);
    unsigned short Type() const { return TLV_MessageData; }
    unsigned short Length() const { return 0; }
  };

  class AdvMsgBodyTLV : public InTLV {
   protected:
    ICQSubType *m_icqsubtype;
    
   public:
    AdvMsgBodyTLV();
    ~AdvMsgBodyTLV();

    ICQSubType* grabICQSubType();

    void ParseValue(Buffer& b);
    unsigned short Type() const { return TLV_AdvMsgBody; }
    unsigned short Length() const { return 0; }
  };

  class AdvMsgDataTLV : public InTLV {
    ICQSubType *m_icqsubtype;

   public:
    AdvMsgDataTLV();
    ~AdvMsgDataTLV();

    ICQSubType* grabICQSubType();

    void ParseValue(Buffer& b);
    unsigned short Type() const { return TLV_AdvMsgData; }
    unsigned short Length() const { return 0; }
  };


  // --------------- ICQDataTLV ------------------

  class ICQDataTLV : public InTLV {
   private:
    ICQSubType *m_icqsubtype;

   public:
    ICQDataTLV();
    ~ICQDataTLV();

    ICQSubType* getICQSubType() const;
    ICQSubType* grabICQSubType();

    void ParseValue(Buffer& b);
    unsigned short Type() const { return TLV_ICQData; }
    unsigned short Length() const { return 0; }

  };

  // ---------------- TLV List -------------------

  class TLVList {
   private:
    hash_map<unsigned short,InTLV*> tlvmap;
   public:
    TLVList();
    ~TLVList();

    void Parse(Buffer& b, TLV_ParseMode pm, unsigned short no_tlvs);
    bool exists(unsigned short type);
    InTLV* & operator[](unsigned short type);

  };

}

Buffer& operator<<(Buffer& b, const ICQ2000::OutTLV& t);

#endif

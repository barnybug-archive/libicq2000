/*
 * ICQ Subtypes
 *
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>
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

#include <libicq2000/ICQ.h>

#include "sstream_fix.h"
#include <memory>

using std::ostringstream;
using std::auto_ptr;

namespace ICQ2000 {

  // ----------------- ICQSubtypes ----------------

  ICQSubType::ICQSubType()
    : m_flags(0x0000) { }

  ICQSubType* ICQSubType::ParseICQSubType(Buffer& b, bool adv) {
    unsigned char type, flags;
    b >> type
      >> flags;
    
    bool multi = (flags & MSG_Flag_Multi);

    ICQSubType *ist;
    switch(type) {
    case MSG_Type_Normal:
      ist = new NormalICQSubType(multi);
      break;
    case MSG_Type_URL:
      ist = new URLICQSubType();
      break;
    case MSG_Type_SMS:
      ist = new SMSICQSubType();
      break;
    case MSG_Type_AuthReq:
      ist = new AuthReqICQSubType();
      break;
    case MSG_Type_AuthRej:
      ist = new AuthRejICQSubType();
      break;
    case MSG_Type_AuthAcc:
      ist = new AuthAccICQSubType();
      break;
    case MSG_Type_AutoReq_Away:
    case MSG_Type_AutoReq_Occ:
    case MSG_Type_AutoReq_NA:
    case MSG_Type_AutoReq_DND:
    case MSG_Type_AutoReq_FFC:
      ist = new AwayMsgSubType(type);
      break;
    default:
      throw ParseException("Unknown ICQ Subtype");
    }

    if (dynamic_cast<UINICQSubType*>(ist) != NULL) {
      UINICQSubType *ust = dynamic_cast<UINICQSubType*>(ist);
      ust->setAdvanced(adv);
    }
    ist->setFlags(flags);
    ist->ParseBody(b);

    return ist;
  }

  void ICQSubType::Output(Buffer& b) const {
    b << getType()
      << getFlags();

    OutputBody(b);
  }

  UINICQSubType::UINICQSubType()
    : m_source(0), m_destination(0), m_ack(false), m_advanced(false), m_status(0) { }

  UINICQSubType::UINICQSubType(unsigned int s, unsigned int d)
    : m_source(s), m_destination(d), m_ack(false), m_advanced(false), m_status(0) { }

  unsigned int UINICQSubType::getSource() const { return m_source; }

  unsigned int UINICQSubType::getDestination() const { return m_destination; }

  void UINICQSubType::setDestination(unsigned int d) { m_destination = d; }

  void UINICQSubType::setSource(unsigned int s) { m_source = s; }

  bool UINICQSubType::isAdvanced() const { return m_advanced; }

  void UINICQSubType::setAdvanced(bool b) { m_advanced = b; }

  bool UINICQSubType::isACK() const { return m_ack; }

  void UINICQSubType::setACK(bool b) { m_ack = b; }

  unsigned short UINICQSubType::getStatus() const { return m_status; }

  void UINICQSubType::setStatus(unsigned short s) { m_status = s; }

  void UINICQSubType::ParseBody(Buffer& b)
  {
    if (m_advanced) {
      unsigned short unknown;
      b >> m_status
	>> unknown;
    }

    ParseBodyUIN(b);
  }

  void UINICQSubType::OutputBody(Buffer& b) const
  {
    if (m_advanced) {
      b << (unsigned short)m_status
	<< (unsigned short)(m_ack ? 0x0000: 0x0001);
    }

    OutputBodyUIN(b);
  }

  NormalICQSubType::NormalICQSubType(bool multi)
    : m_multi(multi), m_foreground(0x00000000),
      m_background(0x00ffffff) { }

  NormalICQSubType::NormalICQSubType(const string& msg, unsigned int uin)
    : UINICQSubType(0, uin), m_message(msg), m_foreground(0x00000000),
      m_background(0x00ffffff) { }

  string NormalICQSubType::getMessage() const { return m_message; }
  
  bool NormalICQSubType::isMultiParty() const { return m_multi; }

  void NormalICQSubType::setMessage(const string& msg) { m_message = msg; }

  void NormalICQSubType::ParseBodyUIN(Buffer& b) {
    b.UnpackUint16StringNull(m_message);
    b.ServerToClient(m_message);

    if (m_advanced) {
      b >> m_foreground
	>> m_background;
    } else {
      m_foreground = 0x00000000;
      m_background = 0x00ffffff;
    }
  }

  void NormalICQSubType::OutputBodyUIN(Buffer& b) const {
    if (m_ack) {
      b.PackUint16StringNull("");
    } else {
      string m_text = m_message;
      b.ClientToServer(m_text);
      b.PackUint16StringNull(m_text);
    }
    
    if (m_advanced) {
      if (m_ack) {
	b << (unsigned int)0x00000000
	  << (unsigned int)0xffffffff;
      } else {
	b << (unsigned int)m_foreground
	  << (unsigned int)m_background;
      }
    }
  }

  unsigned short NormalICQSubType::Length() const {
    string text = m_message;
    Translator::LFtoCRLF(text);

    return text.size() + (m_advanced ? 13 : 5);
  }

  unsigned char NormalICQSubType::getType() const { return MSG_Type_Normal; }

  void NormalICQSubType::setForeground(unsigned int f) { m_foreground = f; }

  void NormalICQSubType::setBackground(unsigned int b) { m_background = b; }
  
  unsigned int NormalICQSubType::getForeground() const { return m_foreground; }

  unsigned int NormalICQSubType::getBackground() const { return m_background; }

  URLICQSubType::URLICQSubType()
    { }

  URLICQSubType::URLICQSubType(const string& msg, const string& url, unsigned int source, unsigned int destination)
    : m_message(msg), m_url(url), UINICQSubType(source, destination) { }

  string URLICQSubType::getMessage() const { return m_message; }

  string URLICQSubType::getURL() const { return m_url; }

  void URLICQSubType::setMessage(const string& msg) { m_message = msg; }

  void URLICQSubType::setURL(const string& url) { m_url = url; }

  void URLICQSubType::ParseBodyUIN(Buffer& b) {
    string text;
    b.UnpackUint16StringNull(text);
    
    /*
     * Format is [message] 0xfe [url]
     */
    int l = text.find( 0xfe );
    if (l != -1) {
      m_message = text.substr( 0, l );
      m_url = text.substr( l+1 );
    } else {
      m_message = text;
      m_url = "";
    }
    b.ServerToClient(m_message);
    b.ServerToClient(m_url);

  }

  void URLICQSubType::OutputBodyUIN(Buffer& b) const {
    if (m_ack) {
      b.PackUint16StringNull("");
    } else {
      ostringstream ostr;
      string m_text=m_message;
      b.ClientToServer(m_text);
      ostr << m_text << (unsigned char)0xfe << m_url;
      m_text = ostr.str();
      b.PackUint16StringNull(m_text);
    }
  }

  unsigned short URLICQSubType::Length() const {
    string text = m_message + m_url;
    Translator::LFtoCRLF(text);

    return text.size() + 6;
  }

  unsigned char URLICQSubType::getType() const { return MSG_Type_URL; }

  AwayMsgSubType::AwayMsgSubType(unsigned char type)
   : m_type(type) { }

  AwayMsgSubType::AwayMsgSubType(Status s, unsigned int uin)
    : UINICQSubType(0, uin) {

    switch(s) {
    case STATUS_AWAY:
      m_type = MSG_Type_AutoReq_Away;
      break;
    case STATUS_OCCUPIED:
      m_type = MSG_Type_AutoReq_Occ;
      break;
    case STATUS_NA:
      m_type = MSG_Type_AutoReq_NA;
      break;
    case STATUS_DND:
      m_type = MSG_Type_AutoReq_DND;
      break;
    case STATUS_FREEFORCHAT:
      m_type = MSG_Type_AutoReq_FFC;
      break;
    default:
      m_type = MSG_Type_AutoReq_Away;
    }

  }

  void AwayMsgSubType::ParseBodyUIN(Buffer& b) {
    b.UnpackUint16StringNull(m_message);
    b.ServerToClient(m_message);
  }

  void AwayMsgSubType::OutputBodyUIN(Buffer& b) const {
    string tekst = m_message;
    b.ClientToServer(tekst);
    b.PackUint16StringNull(tekst);
  }

  unsigned short AwayMsgSubType::Length() const {
    // This doesn't appear to be called when sending an away message
    // response, so I'm leaving this as 9.
    return 9;
  }

  unsigned char AwayMsgSubType::getType() const { return m_type; }

  unsigned char AwayMsgSubType::getFlags() const { return 0x03; }

  string AwayMsgSubType::getMessage() const { return m_message; }

  void AwayMsgSubType::setMessage(const string& msg) { m_message = msg; }

  SMSICQSubType::SMSICQSubType() { }

  string SMSICQSubType::getMessage() const { return m_message; }

  SMSICQSubType::Type SMSICQSubType::getSMSType() const { return m_type; }

  void SMSICQSubType::ParseBody(Buffer& b) {
    /*
     * Here we go... this is a biggy
     */

    /* Next 21 bytes
     * Unknown 
     * 01 00 00 20 00 0e 28 f6 00 11 e7 d3 11 bc f3 00 04 ac 96 9d c2
     */
    b.advance(21);

    /* Delivery status
     *  0x0000 = SMS
     *  0x0002 = SMS Receipt Success
     *  0x0003 = SMS Receipt Failure
     */
    unsigned short del_stat;
    b >> del_stat;
    switch (del_stat) {
    case 0x0000:
      m_type = SMS;
      break;
    case 0x0002:
    case 0x0003:
      m_type = SMS_Receipt;
      break;
    default:
      throw ParseException("Unknown Type for SMS ICQ Subtype");
    }

    /*
     * A Tag for the type, can be:
     * - "ICQSMS" NULL (?)
     * - "IrCQ-Net Invitation"
     * - ...
     * 07 00 00 00 49 43 51 53 4d 53 00
     * ---length-- ---string-----------
     */
    string tagstr;
    b.UnpackUint32String(tagstr);

    if (tagstr != string("ICQSMS")+'\0') {
      ostringstream ostr;
      ostr << "Unknown SNAC 0x0004 0x0007 ICQ SubType 0x001a tag string: " << tagstr;
      throw ParseException(ostr.str());
    }

    /* Next 3 bytes
     * Unknown
     * 00 00 00
     */
    b.advance(3);


    /* Length till end
     * 4 bytes
     */
    unsigned int msglen;
    b >> msglen;

    string xmlstr;
    b.UnpackUint32String(xmlstr);

    string::iterator s = xmlstr.begin();
    auto_ptr<XmlNode> top(XmlNode::parse(s, xmlstr.end()));

    if (top.get() == NULL) throw ParseException("Couldn't parse xml data in Message SNAC");

    if (m_type == SMS) {

      // -------- Normal SMS Message ---------
      if (top->getTag() != "sms_message") throw ParseException("No <sms_message> tag found in xml data");
      XmlBranch *sms_message = dynamic_cast<XmlBranch*>(top.get());
      if (sms_message == NULL || !sms_message->exists("text")) throw ParseException("No <text> tag found in xml data");
      XmlLeaf *text = sms_message->getLeaf("text");
      if (text == NULL) throw ParseException("<text> tag is not a leaf in xml data");
      m_message = text->getValue();
      
      /**
       * Extra fields
       * senders_network is always blank from my mobile
       */
      XmlLeaf *source = sms_message->getLeaf("source");
      if (source != NULL) m_source = source->getValue();

      XmlLeaf *sender = sms_message->getLeaf("sender");
      if (sender != NULL) m_sender = sender->getValue();

      XmlLeaf *senders_network = sms_message->getLeaf("senders_network");
      if (senders_network != NULL) m_senders_network = senders_network->getValue();

      XmlLeaf *time = sms_message->getLeaf("time");
      if (time != NULL) m_time = time->getValue();

      // ----------------------------------

    } else if (m_type == SMS_Receipt) {

      // -- SMS Delivery Receipt Success --
      if (top->getTag() != "sms_delivery_receipt") throw ParseException("No <sms_delivery_receipt> tag found in xml data");
      XmlBranch *sms_rcpt = dynamic_cast<XmlBranch*>(top.get());
      if (sms_rcpt == NULL) throw ParseException("No tags found in <sms_delivery_receipt>");

      XmlLeaf *message_id = sms_rcpt->getLeaf("message_id");
      if (message_id != NULL) m_message_id = message_id->getValue();

      XmlLeaf *destination = sms_rcpt->getLeaf("destination");
      if (destination != NULL) m_destination = destination->getValue();

      XmlLeaf *delivered = sms_rcpt->getLeaf("delivered");
      m_delivered = false;
      if (delivered != NULL && delivered->getValue() == "Yes") m_delivered = true;

      XmlLeaf *text = sms_rcpt->getLeaf("text");
      if (text != NULL) m_message = text->getValue();

      XmlLeaf *submission_time = sms_rcpt->getLeaf("submition_time"); // can they not spell!
      if (submission_time != NULL) m_submission_time = submission_time->getValue();

      XmlLeaf *delivery_time = sms_rcpt->getLeaf("delivery_time");
      if (delivery_time != NULL) m_delivery_time = delivery_time->getValue();

      // could do with parsing errors for Failure

      // ---------------------------------

    }
      
  }

  void SMSICQSubType::OutputBody(Buffer& b) const {  }

  unsigned short SMSICQSubType::Length() const { return 0; }

  unsigned char SMSICQSubType::getType() const { return MSG_Type_SMS; }

  AuthReqICQSubType::AuthReqICQSubType()
    { }

  AuthReqICQSubType::AuthReqICQSubType(const string& msg, unsigned int source, 
                                       unsigned int destination)
    : UINICQSubType(source, destination), m_message(msg)  { }

  string AuthReqICQSubType::getMessage() const { return m_message; }
  
  void AuthReqICQSubType::ParseBodyUIN(Buffer& b) {
    unsigned char skip;

    /*
    *
    * Dunno what the hell is there in the
    * first 6 bytes (konst).
    *
    */

    b>>skip;
    b>>skip;
    b>>skip;
    b>>skip;
    b>>skip;
    b>>skip;

    /*
    *
    * .. but the authorization request message
    * follows here for sure.
    *
    */

    b.UnpackUint16StringNull(m_message);
    b.ServerToClient(m_message);
  }

  void AuthReqICQSubType::OutputBodyUIN(Buffer& b) const {
    string m_text = m_message;
    b.ClientToServer(m_text);
    b.PackUint16StringNull(m_text);
  }

  unsigned short AuthReqICQSubType::Length() const {
    return 0;
  }

  unsigned char AuthReqICQSubType::getType() const { return MSG_Type_AuthReq; }
  
  AuthRejICQSubType::AuthRejICQSubType()
    { }

  AuthRejICQSubType::AuthRejICQSubType(const string& msg, unsigned int source, 
                                       unsigned int destination)
    : UINICQSubType(source, destination), m_message(msg)  { }

  string AuthRejICQSubType::getMessage() const { return m_message; }
  
  void AuthRejICQSubType::setMessage(const string& msg) { m_message = msg; }

  void AuthRejICQSubType::ParseBodyUIN(Buffer& b) {
    b.UnpackUint16StringNull(m_message);
    b.ServerToClient(m_message);
  }

  void AuthRejICQSubType::OutputBodyUIN(Buffer& b) const {
    string m_text = m_message;
    b.ClientToServer(m_text);
    b.PackUint16StringNull(m_text);
  }

  unsigned short AuthRejICQSubType::Length() const {
    return m_message.size()+3;
  }

  unsigned char AuthRejICQSubType::getType() const { return MSG_Type_AuthRej; }

  AuthAccICQSubType::AuthAccICQSubType()
  { }

  AuthAccICQSubType::AuthAccICQSubType(unsigned int source, 
                                       unsigned int destination)
    : UINICQSubType(source,destination) { }

  void AuthAccICQSubType::ParseBodyUIN(Buffer& b) {
  }

  void AuthAccICQSubType::OutputBodyUIN(Buffer& b) const {
  }

  unsigned short AuthAccICQSubType::Length() const {
    return 0;
  }

  unsigned char AuthAccICQSubType::getType() const { return MSG_Type_AuthAcc; }

}

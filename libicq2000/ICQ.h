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

#ifndef ICQ_H
#define ICQ_H

#include <string>
#include <list>

#include <libicq2000/Xml.h>
#include <libicq2000/buffer.h>
#include <libicq2000/exceptions.h>
#include <libicq2000/constants.h>
#include <libicq2000/Contact.h>

using std::string;

namespace ICQ2000 {

  // ------------- TCP Command Types ------------------

  const unsigned short V6_TCP_START     = 0x07ee;
  const unsigned short V6_TCP_ACK       = 0x07da;
  

  // ------------- Message Types ----------------------

  const unsigned char MSG_Type_Normal  = 0x01;
  const unsigned char MSG_Type_URL     = 0x04;
  const unsigned char MSG_Type_AuthReq = 0x06;
  const unsigned char MSG_Type_AuthRej = 0x07;
  const unsigned char MSG_Type_AuthAcc = 0x08;
  const unsigned char MSG_Type_UserAdd = 0x0c;
  const unsigned char MSG_Type_WebPager= 0x0d;
  const unsigned char MSG_Type_EmailEx = 0x0e;
  const unsigned char MSG_Type_SMS     = 0x1a;

  const unsigned char MSG_Type_AutoReq_Away = 0xe8;
  const unsigned char MSG_Type_AutoReq_Occ  = 0xe9;
  const unsigned char MSG_Type_AutoReq_NA   = 0xea;
  const unsigned char MSG_Type_AutoReq_DND  = 0xeb;
  const unsigned char MSG_Type_AutoReq_FFC  = 0xec;

  const unsigned char MSG_Flag_AutoReq = 0x03;
  const unsigned char MSG_Flag_Multi   = 0x80;

  const unsigned short Priority_Normal        = 0x0001;
  const unsigned short Priority_Urgent        = 0x0002;
  const unsigned short Priority_ToContactList = 0x0004;

  const unsigned short AcceptStatus_Online     = 0x0000; // accepted
  const unsigned short AcceptStatus_Denied     = 0x0001; // not accepted - denied
  const unsigned short AcceptStatus_Away       = 0x0004; // accepted in away
  const unsigned short AcceptStatus_Occupied   = 0x0009; // not accepted in occupied
  const unsigned short AcceptStatus_DND        = 0x000a; // not accepted in DND
  const unsigned short AcceptStatus_Occ_Accept = 0x000c; // accepted from a to contact list in occupied
  const unsigned short AcceptStatus_NA         = 0x000e; // accepted in NA


  /* ICQSubtype classes
   * An attempt at clearing up the complete
   * mess ICQ have made of bundling everything
   * into one TLV
   */

  class ICQSubType {
   protected:
    unsigned short m_seqnum;

    unsigned char m_flags;
    
   public:
    ICQSubType();
    virtual ~ICQSubType() { }

    static ICQSubType* ParseICQSubType(Buffer& b, bool adv, bool ack);
    void Output(Buffer& b) const;

    virtual void ParseBody(Buffer& b) = 0;
    virtual void OutputBody(Buffer& b) const = 0;
    virtual unsigned short Length() const = 0;

    virtual unsigned char getType() const = 0;
    virtual unsigned char getFlags() const { return m_flags; }
    virtual void setFlags(unsigned char f) { m_flags = f; }

    unsigned short getSeqNum() const { return m_seqnum; }
    void setSeqNum(unsigned short s)  { m_seqnum = s; }
  };

  class UINICQSubType : public ICQSubType {
   protected:
    unsigned int m_source, m_destination;
    bool m_advanced, m_ack;
    bool m_urgent, m_tocontactlist;
    unsigned short m_status;
    string m_away_message;

   public:
    UINICQSubType();
    UINICQSubType(unsigned int s, unsigned int d);

    void setDestination(unsigned int uin);
    void setSource(unsigned int uin);
    unsigned int getSource() const;
    unsigned int getDestination() const;

    unsigned short getStatus() const;
    void setStatus(unsigned short s);

    void ParseBody(Buffer& b);
    void OutputBody(Buffer& b) const;

    virtual void ParseBodyUIN(Buffer& b) = 0;
    virtual void ParseBodyUINACK(Buffer& b);

    virtual void OutputBodyUIN(Buffer& b) const = 0;
    virtual void OutputBodyUINACK(Buffer& b) const;

    bool isAdvanced() const;
    void setAdvanced(bool b);
    void setACK(bool b);
    bool isACK() const;
    void setUrgent(bool b);
    bool isUrgent() const;
    void setToContactList(bool b);
    bool isToContactList() const;

    void setAwayMessage(const std::string& m);
    string getAwayMessage() const;
  };

  class NormalICQSubType : public UINICQSubType {
   private:
    string m_message;
    bool m_multi;
    unsigned int m_foreground, m_background;
    
   public:
    NormalICQSubType(bool multi);
    NormalICQSubType(const string& msg);

    string getMessage() const;
    bool isMultiParty() const;
    void setMessage(const string& message);
    
    void setForeground(unsigned int f);
    void setBackground(unsigned int f);
    unsigned int getForeground() const;
    unsigned int getBackground() const;

    void ParseBodyUIN(Buffer& b);
    void OutputBodyUIN(Buffer& b) const;
    void ParseBodyUINACK(Buffer& b);
    void OutputBodyUINACK(Buffer& b) const;

    unsigned short Length() const;
    unsigned char getType() const;
  };

  class URLICQSubType : public UINICQSubType {
   private:
    string m_message;
    string m_url;
    
   public:
    URLICQSubType();
    URLICQSubType(const string& msg, const string& url);

    string getMessage() const;
    void setMessage(const string& msg);
    string getURL() const;
    void setURL(const string& url);
    
    void ParseBodyUIN(Buffer& b);
    void OutputBodyUIN(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;
  };

  class AwayMsgSubType : public UINICQSubType {
   private:
    unsigned char m_type;
    string m_message;

   public:
    AwayMsgSubType(Status s);
    AwayMsgSubType(unsigned char m_type);

    void ParseBodyUIN(Buffer& b);
    void OutputBodyUIN(Buffer& b) const;

    unsigned short Length() const;
    unsigned char getType() const;
    unsigned char getFlags() const;
  };

  class SMSICQSubType : public ICQSubType {
   public:
    enum Type {
      SMS,
      SMS_Receipt
    };

   private:
    // SMS fields
    string m_source, m_sender, m_senders_network, m_time;

    // SMS Receipt fields
    string m_message_id, m_destination, m_submission_time, m_delivery_time;
    bool m_delivered;

    string m_message;
    Type m_type;

   public:
    SMSICQSubType();

    string getMessage() const;
    Type getSMSType() const;

    void ParseBody(Buffer& b);
    void OutputBody(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;

    // -- SMS fields --
    string getSource() const { return m_source; }
    string getSender() const { return m_sender; }
    string getSenders_network() const { return m_senders_network; }
    string getTime() const { return m_time; }

    // -- SMS Receipt fields --
    string getMessageId() const { return m_message_id; }
    string getDestination() const { return m_destination; }
    string getSubmissionTime() const { return m_submission_time; }
    string getDeliveryTime() const { return m_delivery_time; }
    bool delivered() const { return m_delivered; }

  };

  class AuthReqICQSubType : public UINICQSubType {
   private:
    std::string m_alias, m_firstname, m_lastname, m_email, m_message;
    bool m_auth;

   public:
    AuthReqICQSubType();
    AuthReqICQSubType(const string& alias, const string& firstname,
		      const string& lastname, const string& email, bool auth,
		      const string& msg);

    std::string getMessage() const;

    void ParseBodyUIN(Buffer& b);
    void OutputBodyUIN(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;

  };
  
  class AuthAccICQSubType : public UINICQSubType {
   public:
    AuthAccICQSubType();

    void ParseBodyUIN(Buffer& b);
    void OutputBodyUIN(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;

  };
  
  class AuthRejICQSubType : public UINICQSubType {
   private:
    string m_message;

   public:
    AuthRejICQSubType();
    AuthRejICQSubType(const string& msg);

    string getMessage() const;
    void setMessage(const string& msg);

    void ParseBodyUIN(Buffer& b);
    void OutputBodyUIN(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;

  };

  class EmailExICQSubType : public ICQSubType {
   private:
    string m_message, m_email, m_sender;

   public:
    EmailExICQSubType();

    void ParseBody(Buffer& b);
    void OutputBody(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;

    string getMessage() const;
    string getEmail() const;
    string getSender() const;
  };

  /*
   *  (why the hell ICQ invented another subtype for this when it's almost identical to
   *   an email express message is anyones guess..)
   */
  class WebPagerICQSubType : public ICQSubType {
   private:
    string m_message, m_email, m_sender;

   public:
    WebPagerICQSubType();

    void ParseBody(Buffer& b);
    void OutputBody(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;

    string getMessage() const;
    string getEmail() const;
    string getSender() const;
  };

  class UserAddICQSubType : public UINICQSubType {
   private:
    std::string m_alias, m_firstname, m_lastname, m_email;
    bool m_auth;

   public:
    UserAddICQSubType();
    UserAddICQSubType(const std::string& alias, const std::string& firstname,
		      const std::string& lastname, const std::string& email, bool auth);

    void ParseBodyUIN(Buffer& b);
    void OutputBodyUIN(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;
  };

  // helper functions

  void string_split(const std::string& in, const std::string& sep,
		    int count, std::list<std::string>& fields);
}

#endif

/*
 * ICQ Subtypes
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

#ifndef ICQ_H
#define ICQ_H

#include <string>

#include "Xml.h"
#include "buffer.h"
#include "exceptions.h"
#include "constants.h"

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
  const unsigned char MSG_Type_EmailEx = 0x0e;
  const unsigned char MSG_Type_SMS     = 0x1a;

  const unsigned char MSG_Type_AutoReq_Away = 0xe8;
  const unsigned char MSG_Type_AutoReq_Occ  = 0xe9;
  const unsigned char MSG_Type_AutoReq_NA   = 0xea;
  const unsigned char MSG_Type_AutoReq_DND  = 0xeb;
  const unsigned char MSG_Type_AutoReq_FFC  = 0xec;

  const unsigned char MSG_Flag_AutoReq = 0x03;
  const unsigned char MSG_Flag_Multi   = 0x80;

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

    static ICQSubType* ParseICQSubType(Buffer& b, bool adv);
    void Output(Buffer& b) const;

    virtual void Parse(Buffer& b) = 0;
    virtual void OutputBody(Buffer& b) const = 0;
    virtual unsigned short Length() const = 0;

    virtual unsigned char getType() const = 0;
    virtual unsigned char getFlags() const { return m_flags; }
    virtual void setFlags(unsigned char f) { m_flags = f; }

    unsigned short getSeqNum() const { return m_seqnum; }
    void setSeqNum(unsigned short s)  { m_seqnum = s; }
  };

  class UINRelatedSubType : public ICQSubType {
   protected:
    unsigned int m_source, m_destination;
    bool m_advanced, m_ack;

   public:
    UINRelatedSubType(bool adv);
    UINRelatedSubType(unsigned int s, unsigned int d, bool adv);

    void setDestination(unsigned int uin);
    void setSource(unsigned int uin);
    unsigned int getSource() const;
    unsigned int getDestination() const;

    bool isAdvanced() const;
    void setAdvanced(bool b);
    void setACK(bool b);
    bool isACK() const;
  };

  class NormalICQSubType : public UINRelatedSubType {
   private:
    string m_message;
    bool m_multi;
    unsigned int m_foreground, m_background;
    
   public:
    NormalICQSubType(bool multi, bool adv);
    NormalICQSubType(const string& msg, unsigned int destination, bool adv);

    string getMessage() const;
    bool isMultiParty() const;
    void setMessage(const string& message);
    
    void setForeground(unsigned int f);
    void setBackground(unsigned int f);
    unsigned int getForeground() const;
    unsigned int getBackground() const;

    void Parse(Buffer& b);
    void OutputBody(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;
  };

  class URLICQSubType : public UINRelatedSubType {
   private:
    string m_message;
    string m_url;
    bool m_advanced;
    
   public:
    URLICQSubType(bool adv);
    URLICQSubType(const string& msg, const string& url, unsigned int source, unsigned int destination, bool adv);

    string getMessage() const;
    void setMessage(const string& msg);
    string getURL() const;
    void setURL(const string& url);
    
    void Parse(Buffer& b);
    void OutputBody(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;
  };

  class AwayMsgSubType : public UINRelatedSubType {
   private:
    unsigned char m_type;
    string m_message;

   public:
    AwayMsgSubType(Status s, unsigned int destination);
    AwayMsgSubType(unsigned char m_type);

    void Parse(Buffer& b);
    void OutputBody(Buffer& b) const;

    string getMessage() const;
    void setMessage(const string& msg);

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

    void Parse(Buffer& b);
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

  class AuthReqICQSubType : public UINRelatedSubType {
   private:
    string m_nick;
    string m_first_name;
    string m_last_name;
    string m_email;
    string m_message;

   public:
    AuthReqICQSubType(bool adv);
    AuthReqICQSubType(const string& msg, unsigned int source, 
                      unsigned int destination, bool adv);

    string getMessage() const;
    string getNick() const;
    string getFirstName() const;
    string getLastName() const;
    string getEmail() const;

    void Parse(Buffer& b);
    void OutputBody(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;

  };
  
  class AuthAccICQSubType : public UINRelatedSubType {
   public:
    AuthAccICQSubType(bool adv);
    AuthAccICQSubType(unsigned int source, unsigned int destination, bool adv);

    void Parse(Buffer& b);
    void OutputBody(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;

  };
  
  class AuthRejICQSubType : public UINRelatedSubType {
   private:
    string m_message;

   public:
    AuthRejICQSubType(bool adv);
    AuthRejICQSubType(const string& msg, unsigned int source, 
                      unsigned int destination, bool adv);

    string getMessage() const;
    void setMessage(const string& msg);

    void Parse(Buffer& b);
    void OutputBody(Buffer& b) const;
    unsigned short Length() const;
    unsigned char getType() const;

  };
}

#endif

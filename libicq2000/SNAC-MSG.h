/*
 * SNAC - Message Family
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

#ifndef SNAC_MSG_H
#define SNAC_MSG_H

#include <libicq2000/SNAC-base.h>
#include <libicq2000/ICQ.h>
#include <libicq2000/UserInfoBlock.h>
#include <libicq2000/ICBMCookie.h>

namespace ICQ2000 {

  // Messages (Family 0x0004)
  const unsigned short SNAC_MSG_Error = 0x0001;
  const unsigned short SNAC_MSG_AddICBMParameter = 0x0002;
  const unsigned short SNAC_MSG_Send = 0x0006;
  const unsigned short SNAC_MSG_Message = 0x0007;
  const unsigned short SNAC_MSG_MessageACK = 0x000b;
  const unsigned short SNAC_MSG_SentOffline = 0x000c;

  // ----------------- Message (Family 0x0004) SNACs --------------
  
  class MsgFamilySNAC : virtual public SNAC {
   public:
    unsigned short Family() const { return SNAC_FAM_MSG; }
  };

  class MsgAddICBMParameterSNAC : public MsgFamilySNAC, public OutSNAC {
   protected:
    void OutputBody(Buffer& b) const;
    
   public:
    MsgAddICBMParameterSNAC() { }

    unsigned short Subtype() const { return SNAC_MSG_AddICBMParameter; }
  };

  class MsgSendSNAC : public MsgFamilySNAC, public OutSNAC {
   protected:
    ICQSubType *m_icqsubtype;
    bool m_advanced;
    unsigned short m_seqnum;
    ICBMCookie m_cookie;

    void OutputBody(Buffer& b) const;

   public:
    MsgSendSNAC(ICQSubType *icqsubtype, bool ad = false);

    void setSeqNum(unsigned short sn);
    void setAdvanced(bool ad);
    void setICBMCookie(const ICBMCookie& c);

    unsigned short Subtype() const { return SNAC_MSG_Send; }
  };

  class MessageSNAC : public MsgFamilySNAC, public InSNAC {
   protected:

    // General fields
    UserInfoBlock m_userinfo;
    ICQSubType *m_icqsubtype;
    ICBMCookie m_cookie;

    void ParseBody(Buffer& b);

   public:
    MessageSNAC();
    ~MessageSNAC();

    ICQSubType* getICQSubType() const { return m_icqsubtype; }
    ICQSubType* grabICQSubType();
    ICBMCookie getICBMCookie() const { return m_cookie; }

    unsigned short Subtype() const { return SNAC_MSG_Message; }
  };

  class MessageACKSNAC : public MsgFamilySNAC, public InSNAC, public OutSNAC {
   protected:
    ICBMCookie m_cookie;
    UINRelatedSubType *m_icqsubtype;

    void ParseBody(Buffer& b);
    void OutputBody(Buffer& b) const;

   public:
    MessageACKSNAC();
    MessageACKSNAC(ICBMCookie c, UINRelatedSubType *icqsubtype);
    ~MessageACKSNAC();

    UINRelatedSubType* getICQSubType() const { return m_icqsubtype; }  
    ICBMCookie getICBMCookie() const { return m_cookie; }

    unsigned short Subtype() const { return SNAC_MSG_MessageACK; }
  };

  class MessageSentOfflineSNAC : public MsgFamilySNAC, public InSNAC {
   protected:
    unsigned int m_uin;

    void ParseBody(Buffer& b);

   public:

    unsigned short Subtype() const { return SNAC_MSG_SentOffline; }
    unsigned int getUIN() const { return m_uin; }  
  };

}

#endif

/*
 * SNAC - Server
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

#ifndef SNAC_SRV_H
#define SNAC_SRV_H

#include <string>
#include <time.h>
#include <libicq2000/SNAC-base.h>
#include <libicq2000/ICQ.h>
#include <libicq2000/constants.h>
#include <libicq2000/Contact.h>

namespace ICQ2000 {

  // Server Messages (Family 0x0015) - messages through the server
  const unsigned short SNAC_SRV_Error = 0x0001;
  const unsigned short SNAC_SRV_Send = 0x0002;
  const unsigned short SNAC_SRV_Response = 0x0003;

  /*
   * SRV_Response is very generic - ICQ have hacked all the extra ICQ
   * functionality into this one
   */

  // --------------------- Server (Family 0x0015) SNACs ---------

  class SrvFamilySNAC : virtual public SNAC {
   public:
    unsigned short Family() const { return SNAC_FAM_SRV; }
  };

  class SrvSendSNAC : public SrvFamilySNAC, public OutSNAC {
   protected:
    string m_text, m_destination, m_senders_name;
    unsigned int m_senders_UIN;
    bool m_delivery_receipt;
    
    void OutputBody(Buffer& b) const;

   public:
    SrvSendSNAC(const string& text, const string& destination,
		unsigned int senders_UIN, const string& senders_name, bool delrpt);

    unsigned short Subtype() const { return SNAC_SRV_Send; }
  };

  class SrvRequestOfflineSNAC : public SrvFamilySNAC, public OutSNAC {
   private:
    unsigned int m_uin;

   protected:
    void OutputBody(Buffer& b) const;

   public:
    SrvRequestOfflineSNAC(unsigned int uin);

    unsigned short Subtype() const { return SNAC_SRV_Send; }
  };

  class SrvAckOfflineSNAC : public SrvFamilySNAC, public OutSNAC {
   private:
    unsigned int m_uin;

   protected:
    void OutputBody(Buffer& b) const;

   public:
    SrvAckOfflineSNAC(unsigned int uin);

    unsigned short Subtype() const { return SNAC_SRV_Send; }
  };

  class SrvRequestSimpleUserInfo : public SrvFamilySNAC, public OutSNAC {
   private:
    unsigned int m_my_uin, m_user_uin;

   protected:
    void OutputBody(Buffer& b) const;

   public:
    SrvRequestSimpleUserInfo(unsigned int my_uin, unsigned int user_uin);
    unsigned short Subtype() const { return SNAC_SRV_Send; }
  };

  class SrvRequestShortWP : public SrvFamilySNAC, public OutSNAC {
   protected:
    unsigned int m_my_uin;
    string m_nickname, m_firstname, m_lastname;
    void OutputBody(Buffer& b) const;

   public:
    SrvRequestShortWP(unsigned int my_uin, const string& nickname, 
		      const string& firstname, const string& lastname);

    unsigned short Subtype() const { return SNAC_SRV_Send; }
  };

  class SrvRequestFullWP : public SrvFamilySNAC, public OutSNAC {
   private:
    unsigned int m_my_uin;
    string m_nickname, m_firstname, m_lastname, m_email;
    unsigned short m_min_age, m_max_age;
    unsigned char m_sex;
    unsigned char m_language;
    string m_city, m_state, m_company_name, m_department, m_position;
    unsigned short m_country;
    bool m_only_online;

   protected:
    void OutputBody(Buffer& b) const;

   public:
    SrvRequestFullWP(unsigned int my_uin, const string& nickname, const string& firstname,
		     const string& lastname, const string& email, unsigned short min_age, unsigned short max_age,
		     unsigned char sex, unsigned char language, const string& city, const string& state,
		     unsigned short country, const string& company_name, const string& department,
		     const string& position, bool only_online);

    unsigned short Subtype() const { return SNAC_SRV_Send; }
  };

  class SrvRequestDetailUserInfo : public SrvFamilySNAC, public OutSNAC {
   private:
    unsigned int m_my_uin, m_user_uin;

   protected:
    void OutputBody(Buffer& b) const;

   public:
    SrvRequestDetailUserInfo(unsigned int my_uin, unsigned int user_uin);
    unsigned short Subtype() const { return SNAC_SRV_Send; }
  };

  class SrvUpdateMainHomeInfo : public SrvFamilySNAC, public OutSNAC {
    private:
      unsigned int m_my_uin;
      const MainHomeInfo& m_main_home_info;

    protected:
      void OutputBody(Buffer& b) const;

    public:
      SrvUpdateMainHomeInfo(unsigned int my_uin, const MainHomeInfo& main_home_info);
      unsigned short Subtype() const { return SNAC_SRV_Send; }
 };
  
  class SrvUpdateWorkInfo : public SrvFamilySNAC, public OutSNAC {
    private:
      unsigned int m_my_uin;
      const WorkInfo& m_work_info;

    protected:
      void OutputBody(Buffer& b) const;

    public:
      SrvUpdateWorkInfo(unsigned int my_uin, const WorkInfo& work_info);
      unsigned short Subtype() const { return SNAC_SRV_Send; }
 };
  
  class SrvUpdateHomepageInfo : public SrvFamilySNAC, public OutSNAC {
    private:
      unsigned int m_my_uin;
      const HomepageInfo& m_homepage_info;

    protected:
      void OutputBody(Buffer& b) const;

    public:
      SrvUpdateHomepageInfo(unsigned int my_uin, const HomepageInfo& homepage_info);
      unsigned short Subtype() const { return SNAC_SRV_Send; }
 };
  
  class SrvUpdateAboutInfo : public SrvFamilySNAC, public OutSNAC {
    private:
      unsigned int m_my_uin;
      const string& m_about_info;

    protected:
      void OutputBody(Buffer& b) const;

    public:
      SrvUpdateAboutInfo(unsigned int my_uin, const string& about_info);
      unsigned short Subtype() const { return SNAC_SRV_Send; }
 };
  
  const unsigned short SrvResponse_Error          = 0x0001;
  const unsigned short SrvResponse_AckMainHomeInfoChange = 0x0064; // used to be SrvResponse_SMS
  const unsigned short SrvResponse_AckWorkInfoChange	 = 0x006E;
  const unsigned short SrvResponse_AckHomepageInfoChange = 0x0078;
  const unsigned short SrvResponse_AckAboutInfoChange	 = 0x0082;
  const unsigned short SrvResponse_SMS_Done       = 0x0096;
  const unsigned short SrvResponse_SimpleUI       = 0x0190;
  const unsigned short SrvResponse_SimpleUI_Done  = 0x019a;
  const unsigned short SrvResponse_SearchUI       = 0x01a4;
  const unsigned short SrvResponse_SearchUI_Done  = 0x01ae;
  const unsigned short SrvResponse_MainHomeInfo   = 0x00c8;
  const unsigned short SrvResponse_WorkInfo       = 0x00d2;
  const unsigned short SrvResponse_HomePageInfo   = 0x00dc;
  const unsigned short SrvResponse_AboutInfo      = 0x00e6;
  const unsigned short SrvResponse_EmailInfo      = 0x00eb;
  const unsigned short SrvResponse_InterestInfo   = 0x00f0;
  const unsigned short SrvResponse_BackgroundInfo = 0x00fa;
  const unsigned short SrvResponse_Unknown        = 0x010e;

  class SrvResponseSNAC : public SrvFamilySNAC, public InSNAC {
   public:
    enum ResponseType {
      OfflineMessage,
      OfflineMessagesComplete,
      SMS_Error,
      SMS_Response,
      SimpleUserInfo,
      SearchSimpleUserInfo,
      RMainHomeInfo,
      RHomepageInfo,
      REmailInfo,
      RUnknown,
      RWorkInfo,
      RAboutInfo,
      RInterestInfo,
      RBackgroundInfo,
      AckMainHomeInfoChange,
      AckHomepageInfoChange,
      AckWorkInfoChange,
      AckAboutInfoChange
    };

   protected:
    ResponseType m_type;

    // SMS Response fields
    string m_source, m_network, m_message_id, m_messages_left;
    bool m_deliverable, m_smtp_deliverable;
    int m_error_id;
    string m_error_param;
    string m_smtp_from, m_smtp_to, m_smtp_subject;
    
    // Offline Message fields
    time_t m_time;
    unsigned int m_sender_UIN;
    ICQSubType *m_icqsubtype;

    // SimpleUserInfo fields
    bool m_empty_contact;
    unsigned int m_uin;
    string m_alias, m_firstname, m_lastname, m_email;
    bool m_last_in_search;

    // DetailedUserInfo fields
    MainHomeInfo m_main_home_info;
    HomepageInfo m_homepage_info;
    EmailInfo m_email_info;
    WorkInfo m_work_info;
    BackgroundInfo m_background_info;
    PersonalInterestInfo m_personal_interest_info;
    string m_about;

    bool m_authreq;
    Status m_status;
    unsigned int m_more_results;

    void ParseBody(Buffer& b);
    void ParseICQResponse(Buffer& b);
    void ParseOfflineMessage(Buffer& b);
    void ParseInfoChangeAck(Buffer &b, unsigned short subtype);
    void ParseSMSError(Buffer& b);
    void ParseSMSResponse(Buffer& b);
    void ParseSimpleUserInfo(Buffer &b, unsigned short subtype);
    void ParseDetailedUserInfo(Buffer &b, unsigned short subtype);
    
   public:
    SrvResponseSNAC();
    ~SrvResponseSNAC();

    ResponseType getType() const { return m_type; }
    string getSource() const { return m_source; }
    bool deliverable() const { return m_deliverable; }
    bool smtp_deliverable() const { return m_smtp_deliverable; }

    string getSMTPFrom() const { return m_smtp_from; }
    string getSMTPTo() const { return m_smtp_to; }
    string getSMTPSubject() const { return m_smtp_subject; }

    string getNetwork() const { return m_network; }
    string getMessageId() const { return m_message_id; }
    string getMessagesLeft() const { return m_messages_left; }
    int getErrorId() const { return m_error_id; }
    string getErrorParam() const { return m_error_param; }

    ICQSubType *getICQSubType() const { return m_icqsubtype; }
    unsigned int getSenderUIN() const { return m_sender_UIN; }
    time_t getTime() const { return m_time; }

    bool isEmptyContact() const { return m_empty_contact; }
    unsigned int getUIN() const { return m_uin; }
    string getAlias() const { return m_alias; }
    string getFirstName() const { return m_firstname; }
    string getLastName() const { return m_lastname; }
    string getEmail() const { return m_email; }
    bool getAuthReq() const { return m_authreq; }
    Status getStatus() const { return m_status; }
    
    bool isLastInSearch() const { return m_last_in_search; }
    unsigned int getNumberMoreResults() const { return m_more_results; }
    

    unsigned short Subtype() const { return SNAC_SRV_Response; }

    // detailed user info structures
    MainHomeInfo& getMainHomeInfo() { return m_main_home_info; }
    HomepageInfo& getHomepageInfo() { return m_homepage_info; }
    EmailInfo &getEmailInfo() { return m_email_info; }
    WorkInfo &getWorkInfo() { return m_work_info; }
    BackgroundInfo &getBackgroundInfo() { return m_background_info; }
    PersonalInterestInfo &getPersonalInterestInfo() { return m_personal_interest_info; }
    string getAboutInfo() const { return m_about; }
  };

}

#endif

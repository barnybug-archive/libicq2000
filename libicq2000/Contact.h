/*
 * Contact (model)
 * A contact on the contact list
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

#ifndef CONTACT_H
#define CONTACT_H

#include <list>
#include <string>

#include <libicq2000/constants.h>

using std::list;
using std::string;

namespace ICQ2000 {

  class MessageEvent;

  // DetailedUserInfo classes

  class MainHomeInfo {
    string cellular, normalised_cellular;
    // cellular private - access must be through
    // get/setMobileNo for consistency
    
    void normaliseMobileNo();

   public:
    MainHomeInfo();

    string alias, firstname, lastname, email, city, state, phone, fax, street, zip;
    unsigned short country;
    signed char timezone;

    string getCountry() const;
    string getMobileNo() const;
    void setMobileNo(const string& s);

    string getNormalisedMobileNo() const;
  };

  class HomepageInfo {
   public:
    HomepageInfo();

    unsigned char age, sex;
    string homepage;
    unsigned short birth_year;
    unsigned char birth_month, birth_day, lang1, lang2, lang3;

    string getBirthDate() const;
    string getLanguage(int l) const;
  };

  class EmailInfo {
   private:
    list<string> email_list;

   public:
    EmailInfo();

    void addEmailAddress(const string&);
  };
  
  class WorkInfo {
   public:
    WorkInfo();
    
    string city, state, street, zip;
    unsigned short country;
    string company_name, company_dept, company_position, company_web;
  };

  class BackgroundInfo {
   public:
    typedef std::pair<unsigned short, string> School;
    list<School> schools;   // school names

    BackgroundInfo();

    void addSchool(unsigned short cat, const string& s);
  };

  class PersonalInterestInfo {
   public:
    typedef std::pair<unsigned short, string> Interest;
    list<Interest> interests;

    PersonalInterestInfo();
    
    void addInterest(unsigned short cat, const string& s);
  };

  class Contact {
   private:
    void Init();
    bool m_icqcontact;
    bool m_mobilecontact;

    list<MessageEvent*> m_pending_msgs;

    // static fields
    unsigned int m_uin;

    // dynamic fields - updated when they come online
    unsigned char m_tcp_version;
    Status m_status;
    bool m_invisible;
    bool m_authreq;
    bool m_direct;
    unsigned int m_ext_ip, m_lan_ip;
    unsigned short m_ext_port, m_lan_port;
    bool m_accept_adv_msgs;

    static unsigned int imag_uin;
    
    // other fields
    unsigned short m_seqnum;

    // detailed fields
    MainHomeInfo m_main_home_info;
    HomepageInfo m_homepage_info;
    EmailInfo m_email_info;
    WorkInfo m_work_info;
    PersonalInterestInfo m_personal_interest_info;
    BackgroundInfo m_background_info;
    string m_about;

  public:
    Contact();
    Contact(unsigned int uin);
    Contact(const string& a, const string& m);

    ~Contact();

    unsigned int getUIN() const;
    void setUIN(unsigned int uin);
    string getStringUIN() const;
    string getMobileNo() const;
    string getNormalisedMobileNo() const;
    string getAlias() const;
    string getFirstName() const;
    string getLastName() const;
    string getEmail() const;

    Status getStatus() const;
    string getStatusStr() const;
    bool isInvisible() const;
    bool getAuthReq() const;

    unsigned int getExtIP() const;
    unsigned int getLanIP() const;
    unsigned short getExtPort() const;
    unsigned short getLanPort() const;
    unsigned char getTCPVersion() const;
    bool acceptAdvancedMsgs() const;

    void setMobileNo(const string& mn);
    void setAlias(const string& al);
    void setFirstName(const string& fn);
    void setLastName(const string& ln);
    void setEmail(const string& em);
    void setAuthReq(bool b);

    bool getDirect() const;
    void setDirect(bool b);

    void setStatus(Status st);
    void setInvisible(bool i);
    void setExtIP(unsigned int ip);
    void setLanIP(unsigned int ip);
    void setExtPort(unsigned short port);
    void setLanPort(unsigned short port);
    void setTCPVersion(unsigned char v);
    void setAcceptAdvMsgs(bool b);
   
    void setMainHomeInfo(const MainHomeInfo& m);
    void setHomepageInfo(const HomepageInfo& s);
    void setEmailInfo(const EmailInfo &e);
    void setWorkInfo(const WorkInfo &w);
    void setInterestInfo(const PersonalInterestInfo& p);
    void setBackgroundInfo(const BackgroundInfo& b);
    void setAboutInfo(const string& about);

    MainHomeInfo& getMainHomeInfo();
    HomepageInfo& getHomepageInfo();
    EmailInfo& getEmailInfo();
    WorkInfo& getWorkInfo();
    BackgroundInfo& getBackgroundInfo();
    PersonalInterestInfo& getPersonalInterestInfo();
    const string& getAboutInfo() const;

    bool isICQContact() const;
    bool isMobileContact() const;

    unsigned short nextSeqNum();

    unsigned int numberPendingMessages() const;

    void addPendingMessage(MessageEvent* e);
    MessageEvent *getPendingMessage() const;
    void erasePendingMessage(MessageEvent* e);

    static string UINtoString(unsigned int uin);
    static unsigned int StringtoUIN(const string& s);

    static unsigned int nextImaginaryUIN();

  };

}

#endif

/*
 * Contact
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

#include <libicq2000/Contact.h>

#include "sstream_fix.h"
#include <time.h>
#include <ctype.h>

#include <libicq2000/userinfoconstants.h>
#include <libicq2000/events.h>

using std::ostringstream;
using std::istringstream;

namespace ICQ2000 {

  Contact::Contact()
    : m_uin(0), m_status(STATUS_OFFLINE), m_invisible(false), m_seqnum(0xffff),
      m_icqcontact(false), m_mobilecontact(false) {
    Init();
  }

  Contact::Contact(unsigned int uin)
    : m_uin(uin), m_status(STATUS_OFFLINE), m_invisible(false), m_seqnum(0xffff),
      m_icqcontact(true), m_mobilecontact(false) {
    m_main_home_info.alias = UINtoString(m_uin);
    Init();
  }

  Contact::Contact(const string& a, const string& m)
    : m_icqcontact(false),
      m_status(STATUS_OFFLINE), m_seqnum(0xffff), m_invisible(false),
      m_mobilecontact(true), m_uin(nextImaginaryUIN()) {

    m_main_home_info.alias = a;
    m_main_home_info.setMobileNo(m);
    Init();
  }

  Contact::~Contact() {
    while (!m_pending_msgs.empty()) {
      delete m_pending_msgs.back();
      m_pending_msgs.pop_back();
    }
  }

  void Contact::Init() {
    m_tcp_version = 0;
    m_ext_ip = 0;
    m_lan_ip = 0;
    m_ext_port = 0;
    m_lan_port = 0;
    m_direct = true;
    m_accept_adv_msgs = false;
  }

  unsigned int Contact::getUIN() const { return m_uin; }

  void Contact::setUIN(unsigned int uin) {
    m_uin = uin;
    m_icqcontact = true;
  }

  string Contact::getStringUIN() const { return UINtoString(m_uin); }

  string Contact::getAlias() const { return m_main_home_info.alias; }

  Status Contact::getStatus() const { return m_status; }

  string Contact::getStatusStr() const { return Status_text[m_status]; }

  string Contact::getMobileNo() const { return m_main_home_info.getMobileNo(); }

  string Contact::getNormalisedMobileNo() const { return m_main_home_info.getNormalisedMobileNo(); }

  string Contact::getFirstName() const { return m_main_home_info.firstname; }

  string Contact::getLastName() const { return m_main_home_info.lastname; }

  string Contact::getEmail() const { return m_main_home_info.email; }

  unsigned int Contact::getExtIP() const { return m_ext_ip; }

  unsigned int Contact::getLanIP() const { return m_lan_ip; }

  unsigned short Contact::getExtPort() const { return m_ext_port; }

  unsigned short Contact::getLanPort() const { return m_lan_port; }

  unsigned char Contact::getTCPVersion() const { return m_tcp_version; }

  bool Contact::getDirect() const { return m_direct; }

  void Contact::setDirect(bool b) { m_direct = b; }

  bool Contact::acceptAdvancedMsgs() const {
    return (m_tcp_version >= 7 && m_status != STATUS_OFFLINE && m_accept_adv_msgs);
  }

  bool Contact::isInvisible() const { return m_invisible; }

  bool Contact::getAuthReq() const { return m_authreq; }

  void Contact::setMobileNo(const string& mn) {
    m_main_home_info.setMobileNo(mn);
    
    if (!mn.empty()) m_mobilecontact = true;
    else m_mobilecontact = false;
  }

  void Contact::setAlias(const string& al) { m_main_home_info.alias = al; }

  void Contact::setFirstName(const string& fn) { m_main_home_info.firstname = fn; }

  void Contact::setLastName(const string& ln) { m_main_home_info.lastname = ln; }

  void Contact::setEmail(const string& em) { m_main_home_info.email = em; }

  void Contact::setStatus(Status st) { m_status = st; }

  void Contact::setInvisible(bool inv) { m_invisible = inv; }

  void Contact::setAuthReq(bool b) { m_authreq = b; }

  bool Contact::isICQContact() const { return m_icqcontact; }

  bool Contact::isMobileContact() const { return m_mobilecontact; }

  void Contact::setExtIP(unsigned int ip) { m_ext_ip = ip; }

  void Contact::setLanIP(unsigned int ip) { m_lan_ip = ip; }

  void Contact::setExtPort(unsigned short port) { m_ext_port = port; }

  void Contact::setLanPort(unsigned short port) { m_lan_port = port; }

  void Contact::setTCPVersion(unsigned char v) { m_tcp_version = v; }

  void Contact::setAcceptAdvMsgs(bool b) { m_accept_adv_msgs = b; }

  void Contact::setMainHomeInfo(const MainHomeInfo& s) { m_main_home_info = s; }

  void Contact::setHomepageInfo(const HomepageInfo& s) { m_homepage_info = s; }

  void Contact::setEmailInfo(const EmailInfo& s) { m_email_info = s; }

  void Contact::setWorkInfo(const WorkInfo& s) { m_work_info = s; }

  void Contact::setInterestInfo(const PersonalInterestInfo& s) { m_personal_interest_info = s; }

  void Contact::setBackgroundInfo(const BackgroundInfo& b) { m_background_info = b; }

  void Contact::setAboutInfo(const string& about) { m_about = about; }

  MainHomeInfo& Contact::getMainHomeInfo() { return m_main_home_info; }

  HomepageInfo& Contact::getHomepageInfo() { return m_homepage_info; }

  WorkInfo& Contact::getWorkInfo() { return m_work_info; }

  PersonalInterestInfo &Contact::getPersonalInterestInfo() { return m_personal_interest_info; }

  BackgroundInfo& Contact::getBackgroundInfo() { return m_background_info; }

  EmailInfo& Contact::getEmailInfo() { return m_email_info; }

  const string& Contact::getAboutInfo() const { return m_about; }

  unsigned int Contact::numberPendingMessages() const { return m_pending_msgs.size(); }

  void Contact::addPendingMessage(MessageEvent* e) { return m_pending_msgs.push_back(e); }

  MessageEvent *Contact::getPendingMessage() const { return m_pending_msgs.front(); }

  void Contact::erasePendingMessage(MessageEvent* e) {
    list<MessageEvent*>::iterator curr = m_pending_msgs.begin();
    while (curr != m_pending_msgs.end()) {
      if (*curr == e) {
	m_pending_msgs.erase(curr);
	delete e;
	break;
      }
      ++curr;
    }
  }

  unsigned short Contact::nextSeqNum() {
    return --m_seqnum;
  }

  string Contact::UINtoString(unsigned int uin) {
    ostringstream ostr;
    ostr << uin;
    return ostr.str();
  }

  unsigned int Contact::StringtoUIN(const string& s) {
    istringstream istr(s);
    unsigned int uin = 0;
    istr >> uin;
    return uin;
  }

  unsigned int Contact::imag_uin = 0;
  
  unsigned int Contact::nextImaginaryUIN() {
    return (--imag_uin);
  }

  // Extra Detailed info class implementations;

  MainHomeInfo::MainHomeInfo()
    : country(0), timezone(Timezone_unknown) { }

  string MainHomeInfo::getCountry() const {
    for(unsigned short a = 0; a < Country_table_size; a++) {
      if (Country_table[a].code == country) return Country_table[a].name;
    }
    return Country_table[0].name;
  }

  string MainHomeInfo::getMobileNo() const
  {
    return cellular;
  }
  
  void MainHomeInfo::setMobileNo(const string& s)
  {
    cellular = s;
    normaliseMobileNo();
  }
  
  string MainHomeInfo::getNormalisedMobileNo() const
  {
    return normalised_cellular;
  }

  void MainHomeInfo::normaliseMobileNo()
  {
    normalised_cellular.erase();
    string::iterator curr = cellular.begin();
    while (curr != cellular.end()) {
      if (isdigit(*curr)) normalised_cellular += *curr;
      ++curr;
    }
  }
  
  HomepageInfo::HomepageInfo()
    : age(0), sex(0), birth_year(0), birth_month(0), birth_day(0),
      lang1(0), lang2(0), lang3(0) { }

  string HomepageInfo::getBirthDate() const {
    if (birth_day == 0 || birth_year == 0) return "Unspecified";

    struct tm birthdate;
    birthdate.tm_sec = 0;
    birthdate.tm_min = 0;
    birthdate.tm_hour = 0;
    birthdate.tm_mday = birth_day;
    birthdate.tm_mon = birth_month-1;
    birthdate.tm_year = birth_year - 1900;
    birthdate.tm_isdst = 0;
    mktime(&birthdate);
    char bday[255];
    strftime(bday, 255, "%B %e, %G", &birthdate);
    return string(bday);
  }

  string HomepageInfo::getLanguage(int l) const {
    if (l < 1 || l > 3) return Language_table[0];
    unsigned char lang;
    if (l == 1) lang = lang1;
    if (l == 2) lang = lang2;
    if (l == 3) lang = lang3;
    if (lang >= Language_table_size) return Language_table[0];
    return Language_table[lang];
  }

  EmailInfo::EmailInfo() { }

  void EmailInfo::addEmailAddress(const string& e) {
    email_list.push_back(e);
  }

  WorkInfo::WorkInfo()
    : country(0) { }
    
  BackgroundInfo::BackgroundInfo() { }

  void BackgroundInfo::addSchool(unsigned short cat, const string& s) {
    schools.push_back(School(cat, s));
  }

  PersonalInterestInfo::PersonalInterestInfo() { }

  void PersonalInterestInfo::addInterest(unsigned short cat, const string& s) {
    interests.push_back(Interest(cat, s));
  }

}

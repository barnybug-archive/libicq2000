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

  /**
   *  Do not use this constructor outside the library.
   *  It constructs a contact that is virtual, but doesn't have
   *  an imaginary UIN allocated to it.
   */
  Contact::Contact()
    : m_virtualcontact(true), m_uin(nextImaginaryUIN()), m_status(STATUS_OFFLINE), 
      m_invisible(false), m_seqnum(0xffff), count(0)
  {
    Init();
  }

  Contact::Contact(unsigned int uin)
    : m_virtualcontact(false), m_uin(uin), m_status(STATUS_OFFLINE), 
      m_invisible(false), m_seqnum(0xffff), count(0)
  {
    m_main_home_info.alias = UINtoString(m_uin);
    Init();
  }

  Contact::Contact(const string& a)
    : m_virtualcontact(true), m_uin(nextImaginaryUIN()),
      m_status(STATUS_OFFLINE), m_invisible(false), m_seqnum(0xffff),
      count(0)
  {
    m_main_home_info.alias = a;
    Init();
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
    m_virtualcontact = false;
    userinfo_change_emit();
  }

  string Contact::getStringUIN() const { return UINtoString(m_uin); }

  string Contact::getAlias() const { return m_main_home_info.alias; }

  string Contact::getNameAlias() const 
  {
    string ret = getAlias();
    if (ret.empty()) {
      ret = getFirstName();
      if (!ret.empty() && !getLastName().empty()) ret += " ";
      ret += getLastName();
    }
    if (ret.empty()) {
      if (isICQContact()) ret = getStringUIN();
      else ret = getMobileNo();
    }

    return ret;
  }

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

  bool Contact::getDirect() const { return (m_direct && m_status != STATUS_OFFLINE); }

  void Contact::setDirect(bool b) {
    m_direct = b;
    userinfo_change_emit();
  }

  bool Contact::acceptAdvancedMsgs() const {
    return (m_tcp_version >= 7 && m_status != STATUS_OFFLINE && m_accept_adv_msgs);
  }

  bool Contact::isInvisible() const { return m_invisible; }

  bool Contact::getAuthReq() const { return m_authreq; }

  void Contact::setMobileNo(const string& mn) {
    m_main_home_info.setMobileNo(mn);
    userinfo_change_emit();
  }

  void Contact::setAlias(const string& al) {
    m_main_home_info.alias = al;
    userinfo_change_emit();
  }

  void Contact::setFirstName(const string& fn) {
    m_main_home_info.firstname = fn;
    userinfo_change_emit();
  }

  void Contact::setLastName(const string& ln) {
    m_main_home_info.lastname = ln;
    userinfo_change_emit();
  }

  void Contact::setEmail(const string& em) {
    m_main_home_info.email = em;
    userinfo_change_emit();
  }

  void Contact::setStatus(Status st) {
    setStatus(st, m_invisible);
  }

  void Contact::setStatus(Status st, bool i) {
    if (m_status == st && m_invisible == i) return;
    
    StatusChangeEvent sev(this, st, m_status);

    m_status = st;
    m_invisible = i;
    
    // clear dynamic fields on going OFFLINE
    if (m_status == STATUS_OFFLINE) {
      m_ext_ip = 0;
      m_lan_ip = 0;
      m_ext_port = 0;
      m_lan_port = 0;
      m_tcp_version = 0;
      m_accept_adv_msgs = false;
    }

    status_change_signal.emit( &sev );
  }

  void Contact::userinfo_change_emit()
  {
    UserInfoChangeEvent ev(this);
    userinfo_change_signal.emit(&ev);
  }

  void Contact::setInvisible(bool inv) {
    setStatus(m_status, inv);
  }

  void Contact::setAuthReq(bool b) {
    m_authreq = b;
    userinfo_change_emit();
  }

  bool Contact::isICQContact() const { return !m_virtualcontact; }

  bool Contact::isVirtualContact() const { return m_virtualcontact; }

  bool Contact::isSMSable() const 
  {
    // ' SMS' suffix might be better way, but isn't reliable really
    return !m_main_home_info.getNormalisedMobileNo().empty();
  }

  void Contact::setExtIP(unsigned int ip) { 
    m_ext_ip = ip;
    userinfo_change_emit();
  }

  void Contact::setLanIP(unsigned int ip) {
    m_lan_ip = ip;
    userinfo_change_emit();
  }

  void Contact::setExtPort(unsigned short port) {
    m_ext_port = port;
    userinfo_change_emit();
  }

  void Contact::setLanPort(unsigned short port) {
    m_lan_port = port;
    userinfo_change_emit();
  }

  void Contact::setTCPVersion(unsigned char v) {
    m_tcp_version = v;
    userinfo_change_emit();
  }

  void Contact::setAcceptAdvMsgs(bool b) { m_accept_adv_msgs = b; }

  void Contact::setMainHomeInfo(const MainHomeInfo& s) {
    m_main_home_info = s;
    userinfo_change_emit();
  }

  void Contact::setHomepageInfo(const HomepageInfo& s) {
    m_homepage_info = s;
    userinfo_change_emit();
  }

  void Contact::setEmailInfo(const EmailInfo& s) {
    m_email_info = s;
    userinfo_change_emit();
  }

  void Contact::setWorkInfo(const WorkInfo& s) {
    m_work_info = s;
    userinfo_change_emit();
  }

  void Contact::setInterestInfo(const PersonalInterestInfo& s) {
    m_personal_interest_info = s;
    userinfo_change_emit();
  }

  void Contact::setBackgroundInfo(const BackgroundInfo& b) {
    m_background_info = b;
    userinfo_change_emit();
  }

  void Contact::setAboutInfo(const string& about) {
    m_about = about;
    userinfo_change_emit();
  }

  Contact::MainHomeInfo& Contact::getMainHomeInfo() { return m_main_home_info; }

  Contact::HomepageInfo& Contact::getHomepageInfo() { return m_homepage_info; }

  Contact::WorkInfo& Contact::getWorkInfo() { return m_work_info; }

  Contact::PersonalInterestInfo& Contact::getPersonalInterestInfo() { return m_personal_interest_info; }

  Contact::BackgroundInfo& Contact::getBackgroundInfo() { return m_background_info; }

  Contact::EmailInfo& Contact::getEmailInfo() { return m_email_info; }

  const string& Contact::getAboutInfo() const { return m_about; }

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



  unsigned short Contact::MapStatusToICQStatus(Status st, bool inv) {
    unsigned short s;

    switch(st) {
    case STATUS_ONLINE:
      s = 0x0000;
      break;
    case STATUS_AWAY:
      s = 0x0001;
      break;
    case STATUS_NA:
      s = 0x0005;
      break;
    case STATUS_OCCUPIED:
      s = 0x0011;
      break;
    case STATUS_DND:
      s = 0x0013;
      break;
    case STATUS_FREEFORCHAT:
      s = 0x0020;
      break;
    default:
      s = 0x0000;
    }

    if (inv) s |= STATUS_FLAG_INVISIBLE;
    return s;
  }

  Status Contact::MapICQStatusToStatus(unsigned short st) {
    if (st & STATUS_FLAG_DND) return STATUS_DND;
    else if (st & STATUS_FLAG_NA) return STATUS_NA;
    else if (st & STATUS_FLAG_OCCUPIED) return STATUS_OCCUPIED;
    else if (st & STATUS_FLAG_FREEFORCHAT) return STATUS_FREEFORCHAT;
    else if (st & STATUS_FLAG_AWAY) return STATUS_AWAY;
    else return STATUS_ONLINE;
  }

  bool Contact::MapICQStatusToInvisible(unsigned short st) {
    return (st & STATUS_FLAG_INVISIBLE);
  }

  // Extra Detailed info class implementations;

  Contact::MainHomeInfo::MainHomeInfo()
    : country(0), timezone(Timezone_unknown) { }

  string Contact::MainHomeInfo::getCountry() const {
    for(unsigned short a = 0; a < Country_table_size; a++) {
      if (Country_table[a].code == country) return Country_table[a].name;
    }
    return Country_table[0].name;
  }

  string Contact::MainHomeInfo::getMobileNo() const
  {
    return cellular;
  }
  
  void Contact::MainHomeInfo::setMobileNo(const string& s)
  {
    cellular = s;
    normaliseMobileNo();
  }
  
  string Contact::MainHomeInfo::getNormalisedMobileNo() const
  {
    return normalised_cellular;
  }

  void Contact::MainHomeInfo::normaliseMobileNo()
  {
    normalised_cellular.erase();
    string::iterator curr = cellular.begin();
    while (curr != cellular.end()) {
      if (isdigit(*curr)) normalised_cellular += *curr;
      ++curr;
    }
  }
  
  Contact::HomepageInfo::HomepageInfo()
    : age(0), sex(0), birth_year(0), birth_month(0), birth_day(0),
      lang1(0), lang2(0), lang3(0) { }

  string Contact::HomepageInfo::getBirthDate() const {
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

  string Contact::HomepageInfo::getLanguage(int l) const {
    if (l < 1 || l > 3) return Language_table[0];
    unsigned char lang = 0;
    if (l == 1) lang = lang1;
    if (l == 2) lang = lang2;
    if (l == 3) lang = lang3;
    if (lang >= Language_table_size) return Language_table[0];
    return Language_table[lang];
  }

  Contact::EmailInfo::EmailInfo() { }

  void Contact::EmailInfo::addEmailAddress(const string& e) {
    email_list.push_back(e);
  }

  Contact::WorkInfo::WorkInfo()
    : country(0) { }
    
  Contact::BackgroundInfo::BackgroundInfo() { }

  void Contact::BackgroundInfo::addSchool(unsigned short cat, const string& s) {
    schools.push_back(School(cat, s));
  }

  Contact::PersonalInterestInfo::PersonalInterestInfo() { }

  void Contact::PersonalInterestInfo::addInterest(unsigned short cat, const string& s) {
    interests.push_back(Interest(cat, s));
  }

}

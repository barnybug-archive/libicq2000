/*
 * Events
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

#include "events.h"

#include "Contact.h"

namespace ICQ2000 {

  // --------------- Event ---------------------------

  Event::Event() {
    m_time = time(NULL);
  }

  Event::Event(time_t t) : m_time(t) { }
  
  time_t Event::getTime() const { return m_time; }

  void Event::setTime(time_t t) { m_time = t; }

  // --------------- Socket Event --------------------

  SocketEvent::SocketEvent(int fd) : m_fd(fd) { }

  int SocketEvent::getSocketHandle() const { return m_fd; }

  AddSocketHandleEvent::AddSocketHandleEvent(int fd, Mode m)
    : SocketEvent(fd), m_mode(m) { }

  bool AddSocketHandleEvent::isRead() const { return m_mode & READ; };
  bool AddSocketHandleEvent::isWrite() const { return m_mode & WRITE; };
  bool AddSocketHandleEvent::isException() const { return m_mode & EXCEPTION; };

  SocketEvent::Mode AddSocketHandleEvent::getMode() const { return m_mode; }

  RemoveSocketHandleEvent::RemoveSocketHandleEvent(int fd)
    : SocketEvent(fd) { }

  // --------------- Connnected Event ----------------

  ConnectedEvent::ConnectedEvent() { }

  // --------------- Disconnected Event --------------

  DisconnectedEvent::DisconnectedEvent(Reason r) : m_reason(r) { }

  DisconnectedEvent::Reason DisconnectedEvent::getReason() const { return m_reason; }
  
  // --------------- Log Event -----------------------

  LogEvent::LogEvent(LogType type, const string& msg)
    : m_type(type), m_msg(msg) { }

  LogEvent::LogType LogEvent::getType() const { return m_type; }

  string LogEvent::getMessage() const { return m_msg; }

  // --------------- ContactList Event ---------------

  ContactListEvent::ContactListEvent(Contact *c) { m_contact = c; }

  Contact *ContactListEvent::getContact() const { return m_contact; }
  unsigned int ContactListEvent::getUIN() const { return m_contact->getUIN(); }
    
  ContactListEvent::~ContactListEvent() { }

  // ----------------- StatusChange Event ----------------

  StatusChangeEvent::StatusChangeEvent(Contact* contact, Status st, Status old_st)
    : ContactListEvent(contact), m_status(st), m_old_status(old_st) { }
  
  ContactListEvent::EventType StatusChangeEvent::getType() const { return StatusChange; }
  Status StatusChangeEvent::getStatus() const { return m_status; }
  Status StatusChangeEvent::getOldStatus() const { return m_old_status; }

  // ----------------- UserAdded Event -------------------

  UserAddedEvent::UserAddedEvent(Contact* contact) : ContactListEvent(contact) { }
  ContactListEvent::EventType UserAddedEvent::getType() const { return UserAdded; }

  // ----------------- UserRemoved Event -------------------

  UserRemovedEvent::UserRemovedEvent(Contact* contact) : ContactListEvent(contact) { }
  ContactListEvent::EventType UserRemovedEvent::getType() const { return UserRemoved; }

  // ----------------- UserInfoChange Event -------------------

  UserInfoChangeEvent::UserInfoChangeEvent(Contact* contact) : ContactListEvent(contact) { }
  ContactListEvent::EventType UserInfoChangeEvent::getType() const { return UserInfoChange; }

  // ----------------- MessageQueueChangedEvent -------------------

  MessageQueueChangedEvent::MessageQueueChangedEvent(Contact* contact) : ContactListEvent(contact) { }
  ContactListEvent::EventType MessageQueueChangedEvent::getType() const { return MessageQueueChanged; }

  // --------------- Message Event -------------------

  MessageEvent::MessageEvent(Contact *c) : m_contact(c) { }
  MessageEvent::~MessageEvent() { }

  Contact* MessageEvent::getContact() { return m_contact; }

  bool MessageEvent::isFinished() const { return m_finished; }
  bool MessageEvent::isDelivered() const { return m_delivered; }
  bool MessageEvent::isDirect() const { return m_direct; }

  void MessageEvent::setFinished(bool f) { m_finished = f; }
  void MessageEvent::setDelivered(bool f) { m_delivered = f; }
  void MessageEvent::setDirect(bool f) { m_direct = f; }

  // ---------------- Normal Message ---------------------

  NormalMessageEvent::NormalMessageEvent(Contact* c, const string& msg)
    : MessageEvent(c), m_message(msg), m_offline(false), m_multi(false),
      m_foreground(0x00000000), m_background(0x00ffffff) {
    setDirect(false);
  }

  NormalMessageEvent::NormalMessageEvent(Contact* c, const string& msg, bool multi)
    : MessageEvent(c), m_message(msg), m_multi(multi), m_offline(false),
      m_foreground(0x00000000), m_background(0x00ffffff) {
    setDirect(false);
  }

  NormalMessageEvent::NormalMessageEvent(Contact *c, const string& msg, time_t t, bool multi)
    : MessageEvent(c), m_message(msg), m_offline(true), m_multi(multi),
      m_foreground(0x00000000), m_background(0x00ffffff) {
    setDirect(false);
    m_time = t;
  }

  NormalMessageEvent::NormalMessageEvent(Contact *c, const string& msg, unsigned int fg, unsigned int bg)
    : MessageEvent(c), m_message(msg), m_offline(false), m_multi(false) /* todo */,
      m_foreground(fg), m_background(bg) {
    setDirect(true);
  }

  MessageEvent::MessageType NormalMessageEvent::getType() const { return MessageEvent::Normal; }
  
  unsigned int NormalMessageEvent::getSenderUIN() const { return m_contact->getUIN(); }

  string NormalMessageEvent::getMessage() const { return m_message; }

  bool NormalMessageEvent::isOfflineMessage() const { return m_offline; }

  bool NormalMessageEvent::isMultiParty() const { return m_multi; }

  unsigned int NormalMessageEvent::getForeground() const { return m_foreground; }

  unsigned int NormalMessageEvent::getBackground() const { return m_background; }

  void NormalMessageEvent::setForeground(unsigned int f) { m_foreground = f; }

  void NormalMessageEvent::setBackground(unsigned int b) { m_background = b; }

  // ---------------- URL Message ---------------------

  URLMessageEvent::URLMessageEvent(Contact* c, const string& msg, const string& url)
    : MessageEvent(c), m_message(msg), m_url(url), m_offline(false) { }

  URLMessageEvent::URLMessageEvent(Contact *c, const string& msg, const string& url, time_t t)
    : MessageEvent(c), m_message(msg), m_url(url), m_offline(true) {
    m_time = t;
  }

  MessageEvent::MessageType URLMessageEvent::getType() const { return MessageEvent::URL; }
  
  unsigned int URLMessageEvent::getSenderUIN() const { return m_contact->getUIN(); }

  string URLMessageEvent::getMessage() const { return m_message; }

  string URLMessageEvent::getURL() const { return m_url; }

  bool URLMessageEvent::isOfflineMessage() const { return m_offline; }

  // ---------------- SMS Message ------------------------

  SMSMessageEvent::SMSMessageEvent(Contact *c, const string& msg, const string& source,
				   const string& senders_network, const string& time)
    : MessageEvent(c), m_message(msg), m_source(source),
      m_senders_network(senders_network) {
    // fix: m_time = time;
  }

  SMSMessageEvent::SMSMessageEvent(Contact *c, const string& msg, bool rcpt)
    : MessageEvent(c), m_message(msg), m_rcpt(rcpt) { }

  MessageEvent::MessageType SMSMessageEvent::getType() const { return MessageEvent::SMS; }
  
  string SMSMessageEvent::getMessage() const { return m_message; }
  string SMSMessageEvent::getSource() const { return m_source; }
  string SMSMessageEvent::getSender() const { return m_contact->getMobileNo(); }
  string SMSMessageEvent::getSenders_network() const { return m_senders_network; }
  bool SMSMessageEvent::getRcpt() const { return m_rcpt; }

  // ---------------- SMS Receipt ------------------------

  SMSReceiptEvent::SMSReceiptEvent(Contact *c, const string& msg, const string& message_id,
				   const string& submission_time, const string& delivery_time, bool del)
    : MessageEvent(c), m_message(msg), m_message_id(message_id),
      m_submission_time(submission_time), m_delivery_time(delivery_time), m_delivered(del) { }
    
  MessageEvent::MessageType SMSReceiptEvent::getType() const { return MessageEvent::SMS_Receipt; }

  string SMSReceiptEvent::getMessage() const { return m_message; }
  string SMSReceiptEvent::getMessageId() const { return m_message_id; }
  string SMSReceiptEvent::getDestination() const { return m_contact->getMobileNo(); }
  string SMSReceiptEvent::getSubmissionTime() const { return m_submission_time; }
  string SMSReceiptEvent::getDeliveryTime() const { return m_delivery_time; }
  bool SMSReceiptEvent::delivered() const { return m_delivered; }


  // ---------------- Away Message -----------------------

  AwayMessageEvent::AwayMessageEvent(Contact *c)
    : MessageEvent(c) { }

  MessageEvent::MessageType AwayMessageEvent::getType() const { return MessageEvent::AwayMessage; }

  string AwayMessageEvent::getMessage() const { return m_message; }

  void AwayMessageEvent::setMessage(const string& msg) { m_message = msg; }

  // ---------------- My Status Change -------------------

  MyStatusChangeEvent::MyStatusChangeEvent(Status s)
    : m_status(s) { }

  Status MyStatusChangeEvent::getStatus() const { return m_status; }

  // ---------------- New UIN ----------------------------

  NewUINEvent::NewUINEvent(unsigned int uin, bool success) 
    : m_uin(uin), m_success(success) { }

  unsigned int NewUINEvent::getUIN() const { return m_uin; }
  bool NewUINEvent::isSuccess() const { return m_success; }


  // ---------------- Rate Info Change -------------------

  RateInfoChangeEvent::RateInfoChangeEvent(unsigned short code, 
                                           unsigned short rateclass,
                                           unsigned int windowsize,
                                           unsigned int clear,
                                           unsigned int alert,
                                           unsigned int limit,
                                           unsigned int disconnect,
                                           unsigned int currentavg,
                                           unsigned int maxavg) 
    : m_code(code), m_rateclass(rateclass), m_windowsize(windowsize),
      m_clear(clear), m_alert(alert), m_limit(limit), m_disconnect(disconnect), 
      m_currentavg(currentavg), m_maxavg(maxavg) { }

  AuthReqEvent::AuthReqEvent(Contact* c, const string& msg)
    : MessageEvent(c), m_message(msg) {}
    
  AuthReqEvent::AuthReqEvent(Contact* c, const string& nick, 
                             const string& first_name, 
                             const string& last_name,
                             const string& email, const string& msg)
    : MessageEvent(c), m_nick(nick), m_first_name(first_name),
      m_last_name(last_name), m_email(email), m_message(msg), 
      m_offline(false) {}
  AuthReqEvent::AuthReqEvent(Contact* c, const string& nick, 
                             const string& first_name, 
                             const string& last_name,
                             const string& email, 
                             const string& msg,time_t t)
    : MessageEvent(c), m_nick(nick), m_first_name(first_name),
      m_last_name(last_name), m_email(email), m_message(msg), 
      m_offline(true) {
    m_time=t;  
  }
  MessageEvent::MessageType AuthReqEvent::getType() const { 
    return MessageEvent::AuthReq; 
  }
  
  unsigned int AuthReqEvent::getSenderUIN() const { 
    return m_contact->getUIN(); 
  }

  string AuthReqEvent::getMessage() const { return m_message; }
  string AuthReqEvent::getNick() const { return m_nick; }
  string AuthReqEvent::getFirstName() const { return m_first_name; }
  string AuthReqEvent::getLastName() const { return m_last_name; }
  string AuthReqEvent::getEmail() const { return m_email; }

  bool AuthReqEvent::isOfflineMessage() const { return m_offline; }
    
  AuthAckEvent::AuthAckEvent(Contact* c, bool granted)
    : MessageEvent(c), m_offline(false), m_granted(granted) {}
      
  AuthAckEvent::AuthAckEvent(Contact* c, const string& msg, bool granted)
    : MessageEvent(c),  m_message(msg), m_offline(false), 
      m_granted(granted) {}
      
  AuthAckEvent::AuthAckEvent(Contact* c, bool granted, time_t t)
    : MessageEvent(c), m_offline(true), m_granted(granted) {
    m_time=t;  
  }

  AuthAckEvent::AuthAckEvent(Contact* c, const string& msg,
                             bool granted, time_t t)
    : MessageEvent(c), m_message(msg), m_offline(true), m_granted(granted) {
    m_time=t;  
  }
  
  MessageEvent::MessageType AuthAckEvent::getType() const { 
    return MessageEvent::AuthAck; 
  }

  bool AuthAckEvent::isGranted() const { 
    return m_granted; 
  }
  
  unsigned int AuthAckEvent::getSenderUIN() const { 
    return m_contact->getUIN(); 
  }

  string AuthAckEvent::getMessage() const { return m_message; }
  bool AuthAckEvent::isOfflineMessage() const { return m_offline; }

}

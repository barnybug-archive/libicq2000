/*
 * Events
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

#include <libicq2000/events.h>

#include <libicq2000/Contact.h>

namespace ICQ2000 {

  // ============================================================================
  //  Event base class
  // ============================================================================

  /**
   *  Base constructor for events, timestamp set to now.
   */
  Event::Event() {
    m_time = time(NULL);
  }

  /**
   *  Base constructor for events, with a set timestamp.
   */
  Event::Event(time_t t) : m_time(t) { }
  
  /**
   *  get the time when the event occurred.
   *
   * @return the time
   */
  time_t Event::getTime() const { return m_time; }

  /**
   *  set the time of the event. This is used by the library only, and
   *  is of no interest to the client.
   *
   * @param t the time
   */
  void Event::setTime(time_t t) { m_time = t; }

  // ============================================================================
  //  Socket Event
  // ============================================================================

  /**
   *  Base constructor for socket events.
   *
   * @param fd socket file descriptor
   */
  SocketEvent::SocketEvent(int fd) : m_fd(fd) { }

  /**
   *  Destructor for SocketEvent
   */
  SocketEvent::~SocketEvent() { }

  /**
   *  get the socket file descriptor
   *
   * @return socket file descriptor
   */
  int SocketEvent::getSocketHandle() const { return m_fd; }

  /**
   *  Constructor for an add socket event.
   *
   * @param fd socket file descriptor
   * @param m mode of selection
   */
  AddSocketHandleEvent::AddSocketHandleEvent(int fd, Mode m)
    : SocketEvent(fd), m_mode(m) { }

  /**
   *  Determine if READ selection is required.
   *
   * @return whether READ is set
   */
  bool AddSocketHandleEvent::isRead() const { return m_mode & READ; };

  /**
   *  Determine if WRITE selection is required.
   *
   * @return whether WRITE is set
   */
  bool AddSocketHandleEvent::isWrite() const { return m_mode & WRITE; };

  /**
   *  Determine if EXCEPTION selection is required.
   *
   * @return whether EXCEPTION is set
   */
  bool AddSocketHandleEvent::isException() const { return m_mode & EXCEPTION; };

  /**
   *  Get the mode of the socket handle.
   *  A client should preferably use the is... methods
   * @see isRead, isWrite, isException
   *
   * @return bitmask of modes
   */
  SocketEvent::Mode AddSocketHandleEvent::getMode() const { return m_mode; }

  /**
   *  Constructor for a remove socket event.
   */
  RemoveSocketHandleEvent::RemoveSocketHandleEvent(int fd)
    : SocketEvent(fd) { }

  // ============================================================================
  //  Connected Event
  // ============================================================================

  /**
   *  Simple constructor for a ConnectedEvent.
   */
  ConnectedEvent::ConnectedEvent() { }

  // ============================================================================
  //  Disconnected Event
  // ============================================================================

  /**
   *  Constructor for a DisconnectedEvent.
   */
  DisconnectedEvent::DisconnectedEvent(Reason r) : m_reason(r) { }

  /**
   *  get the reason for disconnection.
   *
   * @return reason for disconnection
   */
  DisconnectedEvent::Reason DisconnectedEvent::getReason() const { return m_reason; }
  
  // ============================================================================
  //  Log Event
  // ============================================================================

  /**
   *  Constructor for a LogEvent.
   *
   * @param type type of log messages
   * @param msg the log message
   */
  LogEvent::LogEvent(LogType type, const string& msg)
    : m_type(type), m_msg(msg) { }

  /**
   *  get the type of the log message
   *
   * @return type of the log message
   */
  LogEvent::LogType LogEvent::getType() const { return m_type; }

  /**
   *  get the log message
   *
   * @return log message
   */
  string LogEvent::getMessage() const { return m_msg; }

  // ============================================================================
  //  Contact List Event
  // ============================================================================

  /**
   *  Base constructor for contact list events.
   *
   * @param c the contact
   */
  ContactListEvent::ContactListEvent(ContactRef c) { m_contact = c; }

  /**
   *  get the contact
   *
   * @return the contact
   */
  ContactRef ContactListEvent::getContact() const { return m_contact; }

  /**
   *  get the uin of the contact. This could be done just as easily,
   *  with getContact()->getUIN(), provided for convenience.
   *
   * @return
   */
  unsigned int ContactListEvent::getUIN() const { return m_contact->getUIN(); }
    
  /**
   *  Destructor for ContactListEvent
   */
  ContactListEvent::~ContactListEvent() { }

  // ============================================================================
  //  UserAdded Event
  // ============================================================================

  /**
   *  Constructor for UserAddedEvent
   *
   * @param contact the contact that has just been added
   */
  UserAddedEvent::UserAddedEvent(ContactRef contact) : ContactListEvent(contact) { }
  ContactListEvent::EventType UserAddedEvent::getType() const { return UserAdded; }

  // ============================================================================
  //  UserRemoved Event
  // ============================================================================

  /**
   *  Constructor for UserRemovedEvent
   *
   * @param contact the contact that is about to be removed
   */
  UserRemovedEvent::UserRemovedEvent(ContactRef contact) : ContactListEvent(contact) { }
  ContactListEvent::EventType UserRemovedEvent::getType() const { return UserRemoved; }

  // ============================================================================
  //  ServerBasedContactEvent
  // ============================================================================

  ServerBasedContactEvent::ServerBasedContactEvent(const ContactList& l) : m_clist(l) { }
  ContactList& ServerBasedContactEvent::getContactList() { return m_clist; }

  // ============================================================================
  //  Contact Event
  // ============================================================================

  /**
   *  Base constructor for contact list events.
   *
   * @param c the contact
   */
  ContactEvent::ContactEvent(ContactRef c) { m_contact = c; }

  /**
   *  get the contact
   *
   * @return the contact
   */
  ContactRef ContactEvent::getContact() const { return m_contact; }

  /**
   *  get the uin of the contact. This could be done just as easily,
   *  with getContact()->getUIN(), provided for convenience.
   *
   * @return
   */
  unsigned int ContactEvent::getUIN() const { return m_contact->getUIN(); }
    
  /**
   *  Destructor for ContactEvent
   */
  ContactEvent::~ContactEvent() { }

  // ============================================================================
  //  Status Change Event
  // ============================================================================

  /**
   *  Constructor for StatusChangeEvent
   *
   * @param contact the contact whose status has changed
   * @param st the new status
   * @param old_st the old status
   */
  StatusChangeEvent::StatusChangeEvent(ContactRef contact, Status st, Status old_st)
    : ContactEvent(contact), m_status(st), m_old_status(old_st) { }
  
  ContactEvent::EventType StatusChangeEvent::getType() const { return StatusChange; }

  /**
   *  get the new status of the contact
   *
   * @return the new status
   */
  Status StatusChangeEvent::getStatus() const { return m_status; }

  /**
   *  get the old status of the contact
   *
   * @return the old status
   */
  Status StatusChangeEvent::getOldStatus() const { return m_old_status; }

  // ============================================================================
  //  User Info Change Event
  // ============================================================================

  /**
   *  Constructor for UserInfoChangeEvent
   *
   * @param contact the contact whose information has changed
   */
  UserInfoChangeEvent::UserInfoChangeEvent(ContactRef contact) : ContactEvent(contact) { }
  ContactEvent::EventType UserInfoChangeEvent::getType() const { return UserInfoChange; }

  // ============================================================================
  //  Search Result Event
  // ============================================================================

  /**
   *  Constructor for a SearchResultEvent
   */
  SearchResultEvent::SearchResultEvent(SearchResultEvent::SearchType t)
    : m_finished(false), m_expired(false), m_searchtype(t),
      m_last_contact(NULL), m_more_results(0)
  { }

  ContactRef SearchResultEvent::getLastContactAdded() const { return m_last_contact; }
  
  ContactList& SearchResultEvent::getContactList() { return m_clist; }
  
  void SearchResultEvent::setLastContactAdded(ContactRef c) { m_last_contact = c; }
  
  SearchResultEvent::SearchType SearchResultEvent::getSearchType() const { return m_searchtype; }
  
  bool SearchResultEvent::isFinished() const { return m_finished; }

  void SearchResultEvent::setFinished(bool b) { m_finished = b; }

  bool SearchResultEvent::isExpired() const { return m_expired; }
  
  void SearchResultEvent::setExpired(bool b) { m_expired = b; }

  unsigned int SearchResultEvent::getNumberMoreResults() const { return m_more_results; }
  
  void SearchResultEvent::setNumberMoreResults(unsigned int m) { m_more_results = m; }

  // ============================================================================
  //  Message Event
  // ============================================================================

  /**
   *  Constructor for a MessageEvent
   *
   * @param c the contact related to this event
   */
  MessageEvent::MessageEvent(ContactRef c)
    : m_contact(c)
  { }

  /**
   *  Destructor for MessageEvent
   */
  MessageEvent::~MessageEvent() { }

  /**
   *  get the contact related to the event
   *
   * @return the contact related to the event
   */
  ContactRef MessageEvent::getContact() { return m_contact; }

  /**
   *  get if a message event is finished.  This is used in the message
   *  ack'ing system.
   *
   * @return if message is finished
   */
  bool MessageEvent::isFinished() const { return m_finished; }

  /**
   *  get if a message event was delivered.  This is used in the
   *  message ack'ing system.
   *
   * @return if message was delivered
   */
  bool MessageEvent::isDelivered() const { return m_delivered; }

  /**
   *  get if a message event was sent direct.
   *  This is used in the message ack'ing system.
   *
   * @return if message was sent direct
   */
  bool MessageEvent::isDirect() const { return m_direct; }

  /**
   *  set whether the message has been finished.  This is used
   *  internally by the library and is of no interest to the client.
   *
   * @param f if message was finished
   */
  void MessageEvent::setFinished(bool f) { m_finished = f; }

  /**
   *  set whether the message has been delivered.  This is used
   *  internally by the library and is of no interest to the client.
   *
   * @param f if message was delivered
   */
  void MessageEvent::setDelivered(bool f) { m_delivered = f; }

  /**
   *  set whether the message has been sent direct.  This is used
   *  internally by the library and is of no interest to the client.
   *
   * @param f if message was sent direct
   */
  void MessageEvent::setDirect(bool f) { m_direct = f; }

  /**
   *  set the reason for delivery failure.  This is used
   *  internally by the library and is of no interest to the client.
   *
   * @param d delivery failure reason
   */
  void MessageEvent::setDeliveryFailureReason(DeliveryFailureReason d)
  {
    m_failure_reason = d;
  }

  /**
   *  get the reason for delivery failure. Defined when getFinished ==
   *  false and getDelivered == false too.  This is used in the
   *  message ack'ing system.
   *
   * @return reason for delivery failing
   */
  MessageEvent::DeliveryFailureReason MessageEvent::getDeliveryFailureReason() const
  {
    return m_failure_reason;
  }

  // ============================================================================
  //  ICQ Message Event
  // ============================================================================

  /**
   *  Constructor for a ICQMessageEvent
   *
   * @param c the contact related to this event
   */
  ICQMessageEvent::ICQMessageEvent(ContactRef c)
    : MessageEvent(c), m_urgent(false), m_tocontactlist(false) { }

  /**
   *  get whether the message was sent marked as urgent
   *
   * @return the urgency
   */
  bool ICQMessageEvent::isUrgent() const
  {
    return m_urgent;
  }

  /**
   *  set whether the message shall be marked as urgent
   * @param b urgent
   */
  void ICQMessageEvent::setUrgent(bool b)
  {
    m_urgent = b;
  }

  /**
   *  get whether the message was sent 'to contact list'
   *
   * @return whether message was to contact list
   */
  bool ICQMessageEvent::isToContactList() const
  {
    return m_tocontactlist;
  }

  /**
   *  set whether the message shall be marked as urgent
   */
  void ICQMessageEvent::setToContactList(bool b)
  {
    m_tocontactlist = b;
  }

  /**
   *  get the uin of the sender.  This is really miss-named, if you
   *  were sending the message, this would be the UIN of the recipient.
   *
   * @return the uin
   */
  unsigned int ICQMessageEvent::getSenderUIN() const { return m_contact->getUIN(); }

  /**
   *  get the away message
   *
   * @return the away message
   */
  string ICQMessageEvent::getAwayMessage() const { return m_away_message; }

  /**
   *  set the away message
   *
   * @param msg the away message
   */
  void ICQMessageEvent::setAwayMessage(const string& msg) { m_away_message = msg; }

  // ============================================================================
  //  Normal Message
  // ============================================================================

  /**
   *  Construct a NormalMessageEvent.
   *
   * @param c the contact related to the event
   * @param msg the message
   * @param multi tag message as a multireceipt message
   */
  NormalMessageEvent::NormalMessageEvent(ContactRef c, const string& msg, bool multi)
    : ICQMessageEvent(c), m_message(msg), m_offline(false), m_multi(multi),
      m_foreground(0x00000000), m_background(0x00ffffff) {
    setDirect(false);
  }

  /**
   *  Construct a NormalMessageEvent. This constructor is only used by the library.
   *
   * @param c the contact related to the event
   * @param msg the message
   * @param multi tag message as a multireceipt message
   * @param t the time the message was sent
   */
  NormalMessageEvent::NormalMessageEvent(ContactRef c, const string& msg, time_t t, bool multi)
    : ICQMessageEvent(c), m_message(msg), m_offline(true), m_multi(multi),
      m_foreground(0x00000000), m_background(0x00ffffff) {
    setDirect(false);
    m_time = t;
  }

  /**
   *  Construct a NormalMessageEvent.
   *
   * @param c the contact related to the event
   * @param msg the message
   * @param fg foreground colour for the message
   * @param bg background colour for the message
   */
  NormalMessageEvent::NormalMessageEvent(ContactRef c, const string& msg, unsigned int fg, unsigned int bg)
    : ICQMessageEvent(c), m_message(msg), m_offline(false), m_multi(false) /* todo */,
      m_foreground(fg), m_background(bg) {
    setDirect(true);
  }

  MessageEvent::MessageType NormalMessageEvent::getType() const { return MessageEvent::Normal; }
  
  /**
   *  get the message
   *
   * @return the message
   */
  string NormalMessageEvent::getMessage() const { return m_message; }

  /**
   *  get if the message was an offline message
   *
   * @return if the message was an offline message
   */
  bool NormalMessageEvent::isOfflineMessage() const { return m_offline; }

  /**
   *  get if the message is a multiparty message
   *
   * @return if the message is a multiparty message
   */
  bool NormalMessageEvent::isMultiParty() const { return m_multi; }

  /**
   *  get the foreground colour of the message
   *
   * @return foreground colour of the message
   */
  unsigned int NormalMessageEvent::getForeground() const { return m_foreground; }

  /**
   *  get the background colour of the message
   *
   * @return background colour of the message
   */
  unsigned int NormalMessageEvent::getBackground() const { return m_background; }

  /**
   *  set the foreground colour of the message
   *
   * @param f foreground colour of the message
   */
  void NormalMessageEvent::setForeground(unsigned int f) { m_foreground = f; }

  /**
   *  set the background colour of the message
   *
   * @param b background colour of the message
   */
  void NormalMessageEvent::setBackground(unsigned int b) { m_background = b; }

  // ============================================================================
  //  URL Message
  // ============================================================================

  /**
   *  Construct an URLMessageEvent
   *
   * @param c the contact related to the event
   * @param msg the message
   * @param url the url
   */
  URLMessageEvent::URLMessageEvent(ContactRef c, const string& msg, const string& url)
    : ICQMessageEvent(c), m_message(msg), m_url(url), m_offline(false) { }

  /**
   *  Construct an URLMessageEvent. This constructor is only used by the library.
   *
   * @param c the contact related to the event
   * @param msg the message
   * @param url the url
   * @param t time of sending
   */
  URLMessageEvent::URLMessageEvent(ContactRef c, const string& msg, const string& url, time_t t)
    : ICQMessageEvent(c), m_message(msg), m_url(url), m_offline(true) {
    m_time = t;
  }

  MessageEvent::MessageType URLMessageEvent::getType() const { return MessageEvent::URL; }
  
  /**
   *  get the message
   *
   * @return the message
   */
  string URLMessageEvent::getMessage() const { return m_message; }

  /**
   *  get the url
   *
   * @return the url
   */
  string URLMessageEvent::getURL() const { return m_url; }

  /**
   *  get if the message was an offline message
   *
   * @return if the message was an offline message
   */
  bool URLMessageEvent::isOfflineMessage() const { return m_offline; }

  // ============================================================================
  //  SMS Message
  // ============================================================================

  /**
   *  Construct an SMSMessageEvent.
   *
   * @param c the source contact
   * @param msg the message
   * @param source the source (service)
   * @param senders_network the senders network
   * @param time the time of sending
   *
   * @todo fix parsing of time
   */
  SMSMessageEvent::SMSMessageEvent(ContactRef c, const string& msg, const string& source,
				   const string& senders_network, const string& time)
    : MessageEvent(c), m_message(msg), m_source(source),
      m_senders_network(senders_network) {
    // fix: m_time = time;
  }

  /**
   *  Construct an SMSMessageEvent
   *
   * @param c the destination contact
   * @param msg the message
   * @param rcpt whether to request a delivery receipt
   */
  SMSMessageEvent::SMSMessageEvent(ContactRef c, const string& msg, bool rcpt)
    : MessageEvent(c), m_message(msg), m_rcpt(rcpt) { }

  MessageEvent::MessageType SMSMessageEvent::getType() const { return MessageEvent::SMS; }
  

  /**
   *  get the message
   *
   * @return the message
   */
  string SMSMessageEvent::getMessage() const { return m_message; }

  /**
   *  get the source
   *
   * @return the source
   */
  string SMSMessageEvent::getSource() const { return m_source; }

  /**
   *  get the sender (mobile no)
   *
   * @return the sender
   */
  string SMSMessageEvent::getSender() const { return m_contact->getMobileNo(); }

  /**
   *  get the senders network
   *
   * @return the senders network
   */
  string SMSMessageEvent::getSenders_network() const { return m_senders_network; }

  /**
   *  get if a receipt was requested
   *
   * @return if a receipt was requested
   */
  bool SMSMessageEvent::getRcpt() const { return m_rcpt; }

  void SMSMessageEvent::setSMTPFrom(const string& from) { m_smtp_from = from; }

  string SMSMessageEvent::getSMTPFrom() const { return m_smtp_from; }

  void SMSMessageEvent::setSMTPTo(const string& to) { m_smtp_to = to; }

  string SMSMessageEvent::getSMTPTo() const { return m_smtp_to; }

  void SMSMessageEvent::setSMTPSubject(const string& subj) { m_smtp_subject = subj; }

  string SMSMessageEvent::getSMTPSubject() const { return m_smtp_subject; }

  // ============================================================================
  //  SMS Receipt Event
  // ============================================================================

  /**
   *  Construct an SMSReceiptEvent
   *
   * @param c the source contact
   * @param msg the message
   * @param message_id the message id
   * @param submission_time time of submission
   * @param delivery_time time of delivery
   * @param del if the message was delivered
   */
  SMSReceiptEvent::SMSReceiptEvent(ContactRef c, const string& msg, const string& message_id,
				   const string& submission_time, const string& delivery_time, bool del)
    : MessageEvent(c), m_message(msg), m_message_id(message_id),
      m_submission_time(submission_time), m_delivery_time(delivery_time), m_delivered(del) { }
    
  MessageEvent::MessageType SMSReceiptEvent::getType() const { return MessageEvent::SMS_Receipt; }

  /**
   *  get the message
   *
   * @return the message
   */
  string SMSReceiptEvent::getMessage() const { return m_message; }

  /**
   *  get the message id
   *
   * @return the message id
   */
  string SMSReceiptEvent::getMessageId() const { return m_message_id; }

  /**
   *  get the destination mobile no
   *
   * @return the destination mobile no
   */
  string SMSReceiptEvent::getDestination() const { return m_contact->getMobileNo(); }

  /**
   *  get the submission time
   *
   * @return the submission time
   */
  string SMSReceiptEvent::getSubmissionTime() const { return m_submission_time; }

  /**
   *  get the delivery time
   *
   * @return the delivery time
   */
  string SMSReceiptEvent::getDeliveryTime() const { return m_delivery_time; }

  /**
   *  get if the message was delivered
   *
   * @return if the message was delivered
   */
  bool SMSReceiptEvent::delivered() const { return m_delivered; }


  // ============================================================================
  //  Away Message
  // ============================================================================

  /**
   *  Construct an Away message
   *
   * @param c the contact
   */
  AwayMessageEvent::AwayMessageEvent(ContactRef c)
    : ICQMessageEvent(c) { }

  MessageEvent::MessageType AwayMessageEvent::getType() const { return MessageEvent::AwayMessage; }

  // ============================================================================
  //  Authorisation Request
  // ============================================================================

  /**
   *  Constructor for the Authorisation Request
   *
   * @param c the contact
   * @param msg authorisation message
   */
  AuthReqEvent::AuthReqEvent(ContactRef c, const string& msg)
    : ICQMessageEvent(c), m_message(msg) {}
    
  /**
   *  Constructor for the Authorisation Request
   *
   * @param c the contact
   * @param msg authorisation message
   */
  AuthReqEvent::AuthReqEvent(ContactRef c, const string& msg, time_t t)
    : ICQMessageEvent(c), m_message(msg), m_offline(true) {
    m_time=t;  
  }

  MessageEvent::MessageType AuthReqEvent::getType() const { 
    return MessageEvent::AuthReq; 
  }
  
  /**
   *  get the authorisation message
   *
   * @return authorisation message
   */
  string AuthReqEvent::getMessage() const { return m_message; }

  /**
   *  get if this was an offline message
   *
   * @return if this was an offline message
   */
  bool AuthReqEvent::isOfflineMessage() const { return m_offline; }
    
  // ============================================================================
  //  Authorisation Acknowledgement
  // ============================================================================

  /**
   *  Constructor for the Authorisation Acknowledgement
   *
   * @param c the contact
   * @param granted if authorisation was granted
   */
  AuthAckEvent::AuthAckEvent(ContactRef c, bool granted)
    : ICQMessageEvent(c), m_offline(false), m_granted(granted) {}
      
  /**
   *  Constructor for the Authorisation Acknowledgement
   *
   * @param c the contact
   * @param msg the authorisation message
   * @param granted if authorisation was granted
   */
  AuthAckEvent::AuthAckEvent(ContactRef c, const string& msg, bool granted)
    : ICQMessageEvent(c),  m_message(msg), m_offline(false), 
      m_granted(granted) {}
      
  /**
   *  Constructor for the Authorisation Acknowledgement
   *
   * @param c the contact
   * @param granted if authorisation was granted
   * @param t time the message was sent
   */
  AuthAckEvent::AuthAckEvent(ContactRef c, bool granted, time_t t)
    : ICQMessageEvent(c), m_offline(true), m_granted(granted) {
    m_time=t;  
  }

  /**
   *  Constructor for the Authorisation Acknowledgement
   *
   * @param c the contact
   * @param msg the authorisation message
   * @param granted if authorisation was granted
   * @param t time the message was sent
   */
  AuthAckEvent::AuthAckEvent(ContactRef c, const string& msg,
                             bool granted, time_t t)
    : ICQMessageEvent(c), m_message(msg), m_offline(true), m_granted(granted) {
    m_time=t;  
  }
  
  MessageEvent::MessageType AuthAckEvent::getType() const { 
    return MessageEvent::AuthAck; 
  }

  /**
   *  get if the authorisation was granted
   *
   * @return if the authorisation was granted
   */
  bool AuthAckEvent::isGranted() const { 
    return m_granted; 
  }
  
  /**
   *  get the authorisation message
   *
   * @return the authorisation message
   */
  string AuthAckEvent::getMessage() const { return m_message; }

  /**
   *  get if the message was sent offline
   *
   * @return if the message was sent offline
   */
  bool AuthAckEvent::isOfflineMessage() const { return m_offline; }

  // ============================================================================
  //  E-mail Express message
  // ============================================================================

  EmailExEvent::EmailExEvent(ContactRef c, const string &email,
			     const string &sender, const string &msg)
  : MessageEvent(c), m_sender(sender), m_email(email), m_message(msg) {
  }

  string EmailExEvent::getMessage() const { return m_message; }

  string EmailExEvent::getEmail() const { return m_email; }

  string EmailExEvent::getSender() const { return m_sender; }

  MessageEvent::MessageType EmailExEvent::getType() const { return MessageEvent::EmailEx; }

  unsigned int EmailExEvent::getSenderUIN() const { return m_contact->getUIN(); }

  // ============================================================================
  //  "You were added" message
  // ============================================================================

  UserAddEvent::UserAddEvent(ContactRef c) : ICQMessageEvent(c) { }

  MessageEvent::MessageType UserAddEvent::getType() const { return MessageEvent::UserAdd; }

  unsigned int UserAddEvent::getSenderUIN() const { return m_contact->getUIN(); }

  // ============================================================================
  //  Email Message
  // ============================================================================

  EmailMessageEvent::EmailMessageEvent(ContactRef c, const string &msg)
  : MessageEvent(c), m_message(msg) {
  }

  string EmailMessageEvent::getMessage() const { return m_message; }

  EmailMessageEvent::MessageType EmailMessageEvent::getType() const { return MessageEvent::Email; }

  // ============================================================================
  //  New UIN
  // ============================================================================

  /**
   *  Constructor for a NewUINEvent
   *
   * @param uin your new uin
   * @param success if registration was successful
   */
  NewUINEvent::NewUINEvent(unsigned int uin, bool success) 
    : m_uin(uin), m_success(success) { }

  /**
   *  get your new uin
   *
   * @return the new uin
   */
  unsigned int NewUINEvent::getUIN() const { return m_uin; }

  /**
   *  get if registration was a success
   *
   * @return if registration was a success
   */
  bool NewUINEvent::isSuccess() const { return m_success; }


  // ============================================================================
  //  Rate Info Change
  // ============================================================================

  /**
   *  Constructor for a RateInfoChangeEvent
   *
   * @param code the code
   * @param rateclass the rateclass
   * @param windowsize the size of the window
   * @param clear clear (?)
   * @param alert alert (?)
   * @param limit the limit
   * @param disconnect disconnect (?)
   * @param currentavg the current average
   * @param maxavg the maximum average
   */
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



}

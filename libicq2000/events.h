/*
 * Events
 * ------
 *
 * The library signals everything that happens to the program through
 * calling the signal listeners that have been connected to the Signal
 * dispatchers in Client.
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

#ifndef EVENTS_H
#define EVENTS_H

#include <time.h>
#include <string>

#include <libicq2000/constants.h>

#include <libicq2000/ContactList.h>

namespace ICQ2000 {

  class Contact;

  // ============================================================================
  //  Event Base class
  // ============================================================================

  /**
   *  The base class for all events.  Basic functionality of all
   *  events is timestamping, so you can tell when an event occurred.
   */
  class Event {
   protected:
    /**
     *  the time this event occurred
     */
    time_t m_time;

   public:
    Event();
    Event(time_t t);

    time_t getTime() const;
    void setTime(time_t t);
  };

  // ============================================================================
  //  SocketEvents
  // ============================================================================

  /**
   *  Base class for socket events.
   */
  class SocketEvent : public Event {
   private:
    int m_fd;

   public:
    SocketEvent(int fd);
    virtual ~SocketEvent();

    int getSocketHandle() const;

    /**
     *  enum of the different modes of selecting on the socket
     */
    enum Mode {
      READ      = 1 << 0,
      WRITE     = 1 << 1,
      EXCEPTION = 1 << 2
    };
  };

  /**
   *  A socket handle add event.  This is used by the library to
   *  signal to the client that it should be selecting on this file
   *  descriptor, in the mode(s) described.
   */
  class AddSocketHandleEvent : public SocketEvent {
   private:
    Mode m_mode;

   public:
    AddSocketHandleEvent(int fd, Mode m);
    
    Mode getMode() const;
    bool isRead() const;
    bool isWrite() const;
    bool isException() const;
  };

  /**
   *  A socket handle remove event. This is used by the library to
   *  signal to the client that it should stop selection on this file
   *  descriptor, in any mode.
   */
  class RemoveSocketHandleEvent : public SocketEvent {
   public:
    RemoveSocketHandleEvent(int fd);
  };

  // ============================================================================
  //  ConnectingEvent
  // ============================================================================

  /**
   *  This event is signalled when the client is connecting to the ICQ network
   */
  class ConnectingEvent : public Event {
   public:
    ConnectingEvent();
  };

  // ============================================================================
  //  ConnectedEvent
  // ============================================================================

  /**
   *  This event is signalled when the client is connected properly to
   *  the ICQ network.
   */
  class ConnectedEvent : public Event {
   public:
    ConnectedEvent();
  };

  // ============================================================================
  //  DisconnectedEvent
  // ============================================================================

  /**
   *  This event is signalled when the client has been disconnected
   *  from the ICQ network.  This may happen even without having a
   *  ConnectedEvent, if for example you password fails at login and you are disconnected.
   */
  class DisconnectedEvent : public Event {
   public:
    /**
     *  enum for Reasons for disconnection
     */
    enum Reason {
      REQUESTED,
      FAILED_LOWLEVEL,
      FAILED_BADUSERNAME,
      FAILED_TURBOING,
      FAILED_BADPASSWORD,
      FAILED_MISMATCH_PASSWD,
      FAILED_DUALLOGIN,
      FAILED_UNKNOWN
    };

   private:
    Reason m_reason;

   public:
    DisconnectedEvent(Reason r);

    Reason getReason() const;
  };

  // ============================================================================
  //  LogEvents
  // ============================================================================

  /**
   *  This event is for any logging messages generated by the library.
   */
  class LogEvent : public Event {
   public:
    /**
     *  enum of different types of log messages
     */
    enum LogType {
      WARN,
      ERROR,
      INFO,
      PACKET,
      DIRECTPACKET
    };

   private:
    LogType m_type;
    std::string m_msg;

   public:
    LogEvent(LogType type, const std::string& msg);

    LogType getType() const;
    std::string getMessage() const;
  };

  // ============================================================================
  //  ContactListEvents (user added, user removed)
  // ============================================================================

  /**
   *  Base class for Contact List related events.
   */
  class ContactListEvent : public Event {
   public:
    /**
     *  An enum of the different contact list event types.
     */
    enum EventType {
      UserAdded,
      UserRemoved
    };
    
   protected:
    /**
     *  The contact this event refers to.
     */
    ContactRef m_contact;

   public:
    ContactListEvent(ContactRef c);
    virtual ~ContactListEvent();
    
    ContactRef getContact() const;
    unsigned int getUIN() const;

    /**
     *  get the type of ContactListEvent
     *
     * @return type of the ContactListEvent
     */
    virtual EventType getType() const = 0;
  };

  /**
   *  The event signalled when a user is added.
   */
  class UserAddedEvent : public ContactListEvent {
   public:
    UserAddedEvent(ContactRef c);
    EventType getType() const;
  };

  /**
   *  The event signalled when a user is about to be removed.
   */
  class UserRemovedEvent : public ContactListEvent {
   public:
    UserRemovedEvent(ContactRef c);
    EventType getType() const;
  };

  // ============================================================================
  //  ContactEvents (queue changes, status change, user info change)
  // ============================================================================

  /**
   *  Base class for Contact events.
   */
  class ContactEvent : public Event {
   public:
    /**
     *  An enum of the different contact list event types.
     */
    enum EventType {
      StatusChange,
      UserInfoChange,
    };
    
   protected:
    /**
     *  The contact this event refers to.
     */
    ContactRef m_contact;

   public:
    ContactEvent(ContactRef c);
    virtual ~ContactEvent();
    
    ContactRef getContact() const;
    unsigned int getUIN() const;

    /**
     *  get the type of ContactEvent
     *
     * @return type of the ContactEvent
     */
    virtual EventType getType() const = 0;
  };

  /**
   *  The event signalled when user information changes.
   */
  class UserInfoChangeEvent : public ContactEvent {
   private:
    bool m_is_transient_detail;
   public:
    UserInfoChangeEvent(ContactRef c, bool is_transient_detail);
    EventType getType() const;
    bool isTransientDetail() const;
  };

  /**
   *  The event signalled when a user's status changes.
   */
  class StatusChangeEvent : public ContactEvent {
   private:
    Status m_status;
    Status m_old_status;
    
   public:
    StatusChangeEvent(ContactRef contact, Status status, Status old_status);

    EventType getType() const;
    Status getStatus() const;
    Status getOldStatus() const;
  };

  // ============================================================================
  //  MessageEvents
  // ============================================================================

  /**
   *  A message event.  MessageEvents are used for messages, URLs,
   *  SMSs, Authorisation request/responses and away messages.
   */
  class MessageEvent : public Event {
   public:
    /**
     *  enum of the type of the message
     */
    enum MessageType {
      Normal,
      URL,
      SMS,
      SMS_Receipt,
      AuthReq,
      AuthAck,
      AwayMessage,
      EmailEx,
      UserAdd,
      Email,
      WebPager
    };

    enum DeliveryFailureReason {
      Failed,                  // general failure
      Failed_NotConnected,     // you are not connected!
      Failed_ClientNotCapable, // remote client is not capable (away messages)
      Failed_Denied,           // denied outright
      Failed_Ignored,          // ignore completely - send no ACKs back either
      Failed_Occupied,         // resend as to contactlist/urgent
      Failed_DND,              // resend as to contactlist/urgent
      Failed_SMTP
    };

   protected:
    /// the contact related to the MessageEvent
    ContactRef m_contact; 
    /// whether the event is finished    
    bool m_finished;
    /// whether the event was delivered
    bool m_delivered;
    /// whether the event was sent direct
    bool m_direct;

    DeliveryFailureReason m_failure_reason;

   public:
    MessageEvent(ContactRef c);
    virtual ~MessageEvent();

    /**
     *  get the type of the MessageEvent
     *
     * @return the type of the message
     */
    virtual MessageType getType() const = 0;
    ContactRef getContact();
    
    bool isFinished()  const;
    bool isDelivered() const;
    bool isDirect()    const;
    
    void setFinished(bool f);
    void setDelivered(bool f);
    void setDirect(bool f);

    DeliveryFailureReason getDeliveryFailureReason() const;
    void setDeliveryFailureReason(DeliveryFailureReason d);
  };

  /**
   *  Base class for ICQ messages (not SMS)
   */
  class ICQMessageEvent : public MessageEvent {
   private:
    bool m_urgent, m_tocontactlist, m_offline;
    std::string m_away_message;
    
   public:
    ICQMessageEvent(ContactRef c);
    
    bool isUrgent() const;
    void setUrgent(bool b);
    bool isToContactList() const;
    void setToContactList(bool b);
    bool isOfflineMessage() const;
    void setOfflineMessage(bool b);
    unsigned int getSenderUIN() const;
    std::string getAwayMessage() const;
    void setAwayMessage(const std::string& msg);

    virtual ICQMessageEvent* copy() const = 0;
  };

  /**
   *  A normal message
   */
  class NormalMessageEvent : public ICQMessageEvent {
   private:
    std::string m_message;
    bool m_multi;
    unsigned int m_foreground, m_background;
    
   public:
    NormalMessageEvent(ContactRef c, const std::string& msg, bool multi = false);
    NormalMessageEvent(ContactRef c, const std::string& msg, time_t t, bool multi);
    NormalMessageEvent(ContactRef c, const std::string& msg, unsigned int fg, unsigned int bg);

    std::string getMessage() const;
    MessageType getType() const;
    bool isMultiParty() const;
    unsigned int getForeground() const;
    unsigned int getBackground() const;
    void setForeground(unsigned int f);
    void setBackground(unsigned int b);

    ICQMessageEvent* copy() const;
  };
  
  /**
   *  An URL message
   */
  class URLMessageEvent : public ICQMessageEvent {
   private:
    std::string m_message, m_url;
    
   public:
    URLMessageEvent(ContactRef c, const std::string& msg, const std::string& url);
    URLMessageEvent(ContactRef c, const std::string& msg, const std::string& url, time_t t);

    std::string getMessage() const;
    std::string getURL() const;
    MessageType getType() const;

    ICQMessageEvent* copy() const;
  };
  
  /**
   *  An SMS message
   */
  class SMSMessageEvent : public MessageEvent {
   private:
    std::string m_message, m_source, m_sender, m_senders_network;
    std::string m_smtp_from, m_smtp_to, m_smtp_subject;
    bool m_rcpt;

   public:
    SMSMessageEvent(ContactRef c, const std::string& msg, bool rcpt);
    SMSMessageEvent(ContactRef c, const std::string& msg, const std::string& source,
		    const std::string& senders_network, const std::string& time);

    std::string getMessage() const;
    MessageType getType() const;
    std::string getSource() const;
    std::string getSender() const;
    std::string getSenders_network() const;
    bool getRcpt() const;

    void setSMTPFrom(const std::string& from);
    std::string getSMTPFrom() const;

    void setSMTPTo(const std::string& to);
    std::string getSMTPTo() const;

    void setSMTPSubject(const std::string& subj);
    std::string getSMTPSubject() const;
  };

  /**
   *  An SMS (delivery) receipt
   */
  class SMSReceiptEvent : public MessageEvent {
   private:
    std::string m_message, m_message_id, m_destination, m_submission_time, m_delivery_time;
    bool m_delivered;
    
   public:
    SMSReceiptEvent(ContactRef c, const std::string& msg, const std::string& message_id,
		    const std::string& submission_time, const std::string& delivery_time, bool del);
    
    MessageType getType() const;
    std::string getMessage() const;
    std::string getMessageId() const;
    std::string getDestination() const;
    std::string getSubmissionTime() const;
    std::string getDeliveryTime() const;
    bool delivered() const;
  };

  /**
   *  An Away message. The way away messages work in ICQ is they are
   *  just sending a special blank message to the other end, and the
   *  away message comes back in the ACK, as it would for other
   *  messages when sent to someone who is away (N/A, etc..).
   *  
   */
  class AwayMessageEvent : public ICQMessageEvent {
   public:
    AwayMessageEvent(ContactRef c);

    MessageType getType() const;

    ICQMessageEvent* copy() const;
  };

  /**
   *  An Authorisation Request
   */
  class AuthReqEvent : public ICQMessageEvent {
   private:
    std::string m_message;

   public:
    AuthReqEvent(ContactRef c, const std::string& msg);
    AuthReqEvent(ContactRef c, const std::string& msg, time_t time);

    std::string getMessage() const;
    MessageType getType() const;

    ICQMessageEvent* copy() const;
  };
  
  /**
   *  An Authorisation Acknowledge (success/failure)
   */
  class AuthAckEvent : public ICQMessageEvent {
   private:
    std::string m_message;
    bool m_granted;

   public:
    AuthAckEvent(ContactRef c, bool granted);
    AuthAckEvent(ContactRef c, bool granted, time_t time);
    AuthAckEvent(ContactRef c, const std::string& msg, bool granted);
    AuthAckEvent(ContactRef c, const std::string& msg, bool granted, time_t time);

    std::string getMessage() const;
    MessageType getType() const;
    bool isGranted() const;

    ICQMessageEvent* copy() const;
  };

  /**
   *  An E-mail Express message
   */
  class EmailExEvent : public MessageEvent {
   private:
    std::string m_sender, m_email, m_message;

   public:
    EmailExEvent(ContactRef c, const std::string &email, const std::string &sender, const std::string &msg);

    std::string getMessage() const;
    std::string getEmail() const;
    std::string getSender() const;

    MessageType getType() const;
    unsigned int getSenderUIN() const;
  };

  /**
   *  A Web Pager message
   */
  class WebPagerEvent : public MessageEvent {
   private:
    std::string m_sender, m_email, m_message;

   public:
    WebPagerEvent(ContactRef c, const std::string& email, const std::string& sender, const std::string& msg);

    std::string getMessage() const;
    std::string getEmail() const;
    std::string getSender() const;

    MessageType getType() const;
  };

  /**
   *  A "You were added" message
   */
  class UserAddEvent : public ICQMessageEvent {
   public:
    UserAddEvent(ContactRef c);

    MessageType getType() const;
    unsigned int getSenderUIN() const;

    ICQMessageEvent* copy() const;
  };

  /**
   *  An E-mail message, sent with SMTP
   */
  class EmailMessageEvent : public MessageEvent {
   private:
    std::string m_message;

   public:
    EmailMessageEvent(ContactRef c, const std::string &msg);

    std::string getMessage() const;

    MessageType getType() const;
  };

  // ============================================================================
  //  Search Events
  // ============================================================================

  /**
   *  The event signalled when a user-search result is received.
   */
  class SearchResultEvent : public Event {
   public:
    enum SearchType {
      ShortWhitepage,
      FullWhitepage,
      UIN,
      Keyword,
      RandomChat
    };
	
   private:
    bool m_finished, m_expired;
    SearchType m_searchtype;
    ContactList m_clist;
    ContactRef m_last_contact;
    unsigned int m_more_results;
    
   public:
    SearchResultEvent(SearchType t);

    SearchType getSearchType() const;
    ContactList& getContactList();
    ContactRef getLastContactAdded() const;
    void setLastContactAdded(ContactRef c);
    unsigned int getNumberMoreResults() const;

    bool isFinished() const;
    void setFinished(bool b);
    bool isExpired() const;
    void setExpired(bool b);
    void setNumberMoreResults(unsigned int m);
  };

  /**
   *  The event signalled when entries from the server-based contact list is received.
   */
  class ServerBasedContactEvent : public Event {
   public:
    enum SBLType {
     Fetch,
     Upload,
     Remove
    };

    enum UploadResult {
     Success,
     Failed,
     AuthRequired
    };

   private:
    ContactList m_clist;
    SBLType m_type;
    std::vector<UploadResult> m_results;

   public:
    ServerBasedContactEvent(SBLType t, const ContactList& l);

    void setUploadResults(const std::vector<UploadResult> &v);
    std::map<unsigned int, UploadResult> getUploadResults() const;

    ContactList& getContactList();
    SBLType getType() const { return m_type; }
  };

  // ============================================================================
  //  NewUINEvent
  // ============================================================================

  /**
   *  Registration of a new UIN
   */
  class NewUINEvent : public Event {
   private:
    unsigned int m_uin;
    bool m_success;       

   public:
    NewUINEvent(unsigned int uin, bool success=true);
    unsigned int getUIN() const;
    bool isSuccess() const;
  };

  /**
   *  Rate Information Changed
   */
  class RateInfoChangeEvent : public Event {
   public:
    /**
     * enum of the rate classes
     */
    enum RateClass {
      RATE_CHANGE=1,
      RATE_WARNING,
      RATE_LIMIT,
      RATE_LIMIT_CLEARED
    };
    
   private:
    unsigned short m_code;	
    unsigned short m_rateclass;	
    unsigned int m_windowsize;
    unsigned int m_clear;
    unsigned int m_alert;
    unsigned int m_limit;
    unsigned int m_disconnect;
    unsigned int m_currentavg;
    unsigned int m_maxavg;

   public:
    RateInfoChangeEvent(unsigned short code, unsigned short rateclass, 
                        unsigned int windowsize,unsigned int clear,
                        unsigned int alert,unsigned int limit,
                        unsigned int disconnect,unsigned int currentavg,
                        unsigned int maxavg);
    
    /// get the code
    unsigned short getCode() const { return m_code; }	
    /// get the rate class
    unsigned short getRateClass() const { return m_rateclass; }	
    /// get the size of the window
    unsigned int getWindowSize() const { return m_windowsize; }
    /// get clear (?)
    unsigned int getClear() const { return m_clear; }
    /// get alert (?)
    unsigned int getAlert() const { return m_alert; }
    /// get the limit
    unsigned int getLimit() const { return m_limit; }
    /// get disconnect (?)
    unsigned int getDisconnect() const { return m_disconnect; }
    /// get the current average
    unsigned int getCurrentAvg() const { return m_currentavg; }
    /// get the maximum average
    unsigned int getMaxAvg() const { return m_maxavg; }
  };

} 

#endif

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

using std::string;

namespace ICQ2000 {

  class Contact;

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

  // ----------------- SocketEvent ---------------------

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

  // ----------------- ConnectedEvent ------------------

  /**
   *  This event is signalled when the client is connected properly to
   *  the ICQ network.
   */
  class ConnectedEvent : public Event {
   public:
    ConnectedEvent();
  };

  // ---------------- DisconnectedEvent ----------------

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

  // ---------------- LogEvent -------------------------

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
    string m_msg;

   public:
    LogEvent(LogType type, const string& msg);

    LogType getType() const;
    string getMessage() const;
  };

  // ---------------- ContactList Events ----------------

  /**
   *  Base class for Contact List related events.
   */
  class ContactListEvent : public Event {
   public:
    /**
     *  An enum of the different contact list event types.
     */
    enum EventType {
      StatusChange,
      UserInfoChange,
      UserAdded,
      UserRemoved,
      MessageQueueChanged,
      ServerBasedContact
    };
    
   protected:
    /**
     *  The contact this event refers to.
     */
    Contact *m_contact;

   public:
    ContactListEvent(Contact* c);
    virtual ~ContactListEvent();
    
    Contact* getContact() const;
    unsigned int getUIN() const;

    /**
     *  get the type of ContactListEvent
     *
     * @return type of the ContactListEvent
     */
    virtual EventType getType() const = 0;
  };

  /**
   *  The event signalled when user information changes.
   */
  class UserInfoChangeEvent : public ContactListEvent {
   public:
    UserInfoChangeEvent(Contact* c);
    EventType getType() const;
  };

  /**
   *  The event signalled when a contact entry from the server-based contact list is received.
   */
  class ServerBasedContactEvent : public ContactListEvent {
   public:
    ServerBasedContactEvent(Contact *c);
    EventType getType() const;
  };

  /**
   *  The event signalled when a user is added.
   */
  class UserAddedEvent : public ContactListEvent {
   public:
    UserAddedEvent(Contact *c);
    EventType getType() const;
  };

  /**
   *  The event signalled when a user is about to be removed.
   */
  class UserRemovedEvent : public ContactListEvent {
   public:
    UserRemovedEvent(Contact *c);
    EventType getType() const;
  };

  /**
   *  The event signalled when a user's message queue changes.
   */
  class MessageQueueChangedEvent : public ContactListEvent {
   public:
    MessageQueueChangedEvent(Contact *c);
    EventType getType() const;
  };
  
  /**
   *  The event signalled when a user's status changes.
   */
  class StatusChangeEvent : public ContactListEvent {
   private:
    Status m_status;
    Status m_old_status;
    
   public:
    StatusChangeEvent(Contact* contact, Status status, Status old_status);

    EventType getType() const;
    Status getStatus() const;
    Status getOldStatus() const;
  };

  // ---------------- MessageEvent(s) -------------------

  /**
   *  A message event.  MessageEvents are used for messages, URLs,
   *  SMSs, Authorisation request/responses and away messages.
   */
  class MessageEvent : public Event {
   protected:
    /// the contact related to the MessageEvent
    Contact* m_contact; 
    /// whether the event is finished    
    bool m_finished;
    /// whether the event was delivered
    bool m_delivered;
    /// whether the event was sent direct
    bool m_direct;

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
      AwayMessage
    };

    MessageEvent(Contact* c);
    virtual ~MessageEvent();

    /**
     *  get the type of the MessageEvent
     *
     * @return the type of the message
     */
    virtual MessageType getType() const = 0;
    Contact* getContact();
    
    bool isFinished() const;
    bool isDelivered() const;
    bool isDirect() const;
    
    void setFinished(bool f);
    void setDelivered(bool f);
    void setDirect(bool f);

  };

  /**
   *  A normal message
   */
  class NormalMessageEvent : public MessageEvent {
   private:
    string m_message;
    bool m_offline, m_multi;
    unsigned int m_foreground, m_background;
    
   public:
    NormalMessageEvent(Contact* c, const string& msg, bool multi = false);
    NormalMessageEvent(Contact* c, const string& msg, time_t t, bool multi);
    NormalMessageEvent(Contact* c, const string& msg, unsigned int fg, unsigned int bg);

    string getMessage() const;
    MessageType getType() const;
    unsigned int getSenderUIN() const;
    bool isOfflineMessage() const;
    bool isMultiParty() const;
    unsigned int getForeground() const;
    unsigned int getBackground() const;
    void setForeground(unsigned int f);
    void setBackground(unsigned int b);
  };
  
  /**
   *  An URL message
   */
  class URLMessageEvent : public MessageEvent {
   private:
    string m_message, m_url;
    bool m_offline;
    
   public:
    URLMessageEvent(Contact* c, const string& msg, const string& url);
    URLMessageEvent(Contact* c, const string& msg, const string& url, time_t t);

    string getMessage() const;
    string getURL() const;
    MessageType getType() const;
    unsigned int getSenderUIN() const;
    bool isOfflineMessage() const;
  };
  
  /**
   *  An SMS message
   */
  class SMSMessageEvent : public MessageEvent {
   private:
    string m_message, m_source, m_sender, m_senders_network;
    bool m_rcpt;

   public:
    SMSMessageEvent(Contact* c, const string& msg, bool rcpt);
    SMSMessageEvent(Contact* c, const string& msg, const string& source,
		    const string& senders_network, const string& time);

    string getMessage() const;
    MessageType getType() const;
    string getSource() const;
    string getSender() const;
    string getSenders_network() const;
    bool getRcpt() const;
  };

  /**
   *  An SMS (delivery) receipt
   */
  class SMSReceiptEvent : public MessageEvent {
   private:
    string m_message, m_message_id, m_destination, m_submission_time, m_delivery_time;
    bool m_delivered;
    
   public:
    SMSReceiptEvent(Contact* c, const string& msg, const string& message_id,
		    const string& submission_time, const string& delivery_time, bool del);
    
    MessageType getType() const;
    string getMessage() const;
    string getMessageId() const;
    string getDestination() const;
    string getSubmissionTime() const;
    string getDeliveryTime() const;
    bool delivered() const;
  };

  /**
   *  An Away message
   */
  class AwayMessageEvent : public MessageEvent {
   private:
    Contact *m_contact;
    string m_message;

   public:
    AwayMessageEvent(Contact *c);

    MessageType getType() const;
    string getMessage() const;
    void setMessage(const string& msg);
  };

  /**
   *  An Authorisation Request
   */
  class AuthReqEvent : public MessageEvent {
   private:
    string m_message;
    bool m_offline;

   public:
    AuthReqEvent(Contact* c, const string& msg);
    AuthReqEvent(Contact* c, const string& msg, time_t time);

    string getMessage() const;
    MessageType getType() const;
    bool isOfflineMessage() const;
    unsigned int getSenderUIN() const;
  };
  
  /**
   *  An Authorisation Acknowledge (success/failure)
   */
  class AuthAckEvent : public MessageEvent {
   private:
    string m_message;
    bool m_offline;
    bool m_granted;

   public:
    AuthAckEvent(Contact* c, bool granted);
    AuthAckEvent(Contact* c, bool granted, time_t time);
    AuthAckEvent(Contact* c, const string& msg, bool granted);
    AuthAckEvent(Contact* c, const string& msg, bool granted, time_t time);

    string getMessage() const;
    MessageType getType() const;
    bool isOfflineMessage() const;
    bool isGranted() const;
    unsigned int getSenderUIN() const;
  };

  // --------------------- Self Events ----------------------

  /**
   *  Base class for Self events.
   */
  class SelfEvent : public Event {
   protected:
    Contact *m_self_contact;

   public:
    /**
     *  An enum of the different self event types.
     */
    enum EventType {
      MyStatusChange,
      MyUserInfoChange
    };
    
   public:
    SelfEvent(Contact *self);
    virtual ~SelfEvent();
    
    /**
     *  get the type of SelfEvent
     *
     * @return type of the SelfEvent
     */
    virtual EventType getType() const = 0;

    Contact *getSelfContact() const;
  };

  // --------------------- Status Change Event ----------------------

  /**
   *  Your status change
   */
  class MyStatusChangeEvent : public SelfEvent {
   public:
    MyStatusChangeEvent(Contact *self);

    Status getStatus() const;
    bool getInvisible() const;

    EventType getType() const;
  };

  // --------------------- My UserInfo Change Event ----------------------

  /**
   *  Your details fetched from the server
   */
  class MyUserInfoChangeEvent : public SelfEvent {
   public:
    MyUserInfoChangeEvent(Contact *self);

    EventType getType() const;
  };

  // --------------------- Search Events ----------------------------

  /**
   *  The event signalled when a user-search result is received.
   */
  class SearchResultEvent : public Event {
   public:
    enum SearchType {
      ShortWhitepage,
      FullWhitepage,
      UIN
    };
	
   private:
    bool m_finished, m_expired;
    SearchType m_searchtype;
    ContactList m_clist;
    Contact *m_last_contact;
    unsigned int m_more_results;
    
   public:
    SearchResultEvent(SearchType t);

    SearchType getSearchType() const;
    ContactList& getContactList();
    Contact* getLastContactAdded() const;
    void setLastContactAdded(Contact *c);
    unsigned int getNumberMoreResults() const;

    bool isFinished() const;
    void setFinished(bool b);
    bool isExpired() const;
    void setExpired(bool b);
    void setNumberMoreResults(unsigned int m);
  };

  // --------------------- NewUIN Event -----------------------------

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

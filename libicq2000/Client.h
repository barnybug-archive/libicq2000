/**
 * libICQ2000 Client header
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

#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>

#include <map>

#include <sigc++/signal_system.h>

#include <time.h>

#include <libicq2000/buffer.h>
#include <libicq2000/socket.h>
#include <libicq2000/events.h>
#include <libicq2000/constants.h>
#include <libicq2000/Contact.h>
#include <libicq2000/ContactList.h>
#include <libicq2000/custom_marshal.h>
#include <libicq2000/Translator.h>
#include <libicq2000/RequestIDCache.h>
#include <libicq2000/ICBMCookieCache.h>
#include <libicq2000/DirectClient.h>
#include <libicq2000/DCCache.h>
#include <libicq2000/SMTPClient.h>
#include <libicq2000/userinfoconstants.h>
#include <libicq2000/MessageHandler.h>

using std::string;

namespace ICQ2000 {
  
  // declare some SNAC classes - vastly decreases header dependancies
  class MessageSNAC;
  class MessageACKSNAC;
  class MessageOfflineUserSNAC;
  class SrvResponseSNAC;
  class UINResponseSNAC;
  class RateInfoChangeSNAC;
  class BuddyOnlineSNAC;
  class BuddyOfflineSNAC;
  class UserInfoSNAC;
  class OutSNAC;

  /**
   *  The main library object.  This is the object the user interface
   *  instantiates for a connection, hooks up to signal on and has the
   *  methods to connect, disconnect and send events from.
   */
  class Client : public SigC::Object {
   private:
    enum State { NOT_CONNECTED,
		 AUTH_AWAITING_CONN_ACK,
		 AUTH_AWAITING_AUTH_REPLY,
		 BOS_AWAITING_CONN_ACK,
		 BOS_AWAITING_LOGIN_REPLY,
		 BOS_LOGGED_IN,
		 UIN_AWAITING_CONN_ACK,
		 UIN_AWAITING_UIN_REPLY
    } m_state;

    ContactRef m_self;
    string m_password;
    Status m_status_wanted;
    bool m_invisible_wanted;

    string m_authorizerHostname;
    unsigned short m_authorizerPort;

    string m_bosHostname;
    unsigned short m_bosPort;
    bool m_bosOverridePort;

    bool m_in_dc, m_out_dc;

    unsigned short m_client_seq_num;
    unsigned int m_requestid;
    
    Translator m_translator;

    ContactList m_contact_list;
    
    ContactList m_visible_list;
    ContactList m_invisible_list;

    MessageHandler m_message_handler;

    unsigned char *m_cookie_data;
    unsigned short m_cookie_length;

    unsigned int m_ext_ip;
    TCPSocket m_serverSocket;
    TCPServer m_listenServer;

    SMTPClient m_smtp;

    DCCache m_dccache;   // this is for established connections
    map<unsigned int, DirectClient*> m_uinmap;

    time_t m_last_server_ping;

    RequestIDCache m_reqidcache;
    ICBMCookieCache m_cookiecache;

    Buffer m_recv;
   
    void Init();
    unsigned short NextSeqNum();
    unsigned int NextRequestID();

    void ConnectAuthorizer(State state);
    void DisconnectAuthorizer();
    void ConnectBOS();
    void DisconnectBOS();

    // -- Ping server --
    void PingServer();

    DirectClient* ConnectDirect(ContactRef c);
    void DisconnectDirectConns();
    void DisconnectDirectConn(int fd);

    // ------------------ Signal dispatchers -----------------
    void SignalConnect();
    void SignalDisconnect(DisconnectedEvent::Reason r);
    void SignalMessage(MessageSNAC *snac);
    void SignalMessageACK(MessageACKSNAC *snac);
    void SignalMessageOfflineUser(MessageOfflineUserSNAC *snac);
    void SignalSrvResponse(SrvResponseSNAC *snac);
    void SignalUINResponse(UINResponseSNAC *snac);
    void SignalUINRequestError();
    void SignalRateInfoChange(RateInfoChangeSNAC *snac);
    void SignalLog(LogEvent::LogType type, const string& msg);
    void SignalUserOnline(BuddyOnlineSNAC *snac);
    void SignalUserOffline(BuddyOfflineSNAC *snac);
    void SignalServerBasedContactList(const ContactList& l);
    void SignalAddSocket(int fd, SocketEvent::Mode m);
    void SignalRemoveSocket(int fd);
    // ------------------ Outgoing packets -------------------

    // -------------- Callbacks from ContactList -------------
    void contactlist_cb(ContactListEvent *ev);

    // ------- Callbacks from visible, invisible lists -------
    void visiblelist_cb(ContactListEvent *ev);
    void invisiblelist_cb(ContactListEvent *ev);

    // -------------- Callbacks from Contacts ----------------

    void SendAuthReq();
    void SendNewUINReq();
    void SendCookie();
    void SendCapabilities();
    void SendRateInfoRequest();
    void SendRateInfoAck();
    void SendPersonalInfoRequest();
    void SendAddICBMParameter();
    void SendSetUserInfo();
    void SendLogin();
    void SendOfflineMessagesRequest();
    void SendOfflineMessagesACK();

    void SendAdvancedACK(MessageSNAC *snac);

    void Send(Buffer& b);

    void HandleUserInfoSNAC(UserInfoSNAC *snac);

    Buffer::marker FLAPHeader(Buffer& b, unsigned char channel);
    void FLAPFooter(Buffer& b, Buffer::marker& mk);

    void FLAPwrapSNAC(Buffer& b, const OutSNAC& snac);
    void FLAPwrapSNACandSend(const OutSNAC& snac);

    // ------------------ Incoming packets -------------------

    /**
     *  non-blocking receives all waiting packets from server
     *  and parses and handles them
     */
    void RecvFromServer();

    void Parse();
    void ParseCh1(Buffer& b, unsigned short seq_num);
    void ParseCh2(Buffer& b, unsigned short seq_num);
    void ParseCh3(Buffer& b, unsigned short seq_num);
    void ParseCh4(Buffer& b, unsigned short seq_num);

    // -------------------------------------------------------

    ContactRef lookupICQ(unsigned int uin);
    ContactRef lookupMobile(const string& m);
    ContactRef lookupEmail(const string& email);

    ContactRef getUserInfoCacheContact(unsigned int reqid);

    void ICBMCookieCache_expired_cb(MessageEvent *ev);
    void dccache_expired_cb(DirectClient *dc);
    void reqidcache_expired_cb(	RequestIDCacheValue *v );
    void dc_connected_cb(DirectClient *dc);
    void dc_log_cb(LogEvent *ev);
    void dc_socket_cb(SocketEvent *ev);
    void dc_messageack_cb(MessageEvent *ev);

    bool SendDirect(MessageEvent *ev);

    void SendViaServer(MessageEvent *ev);
    void SendViaServerAdvanced(MessageEvent *ev);
    void SendViaServerNormal(MessageEvent *ev);
    
    void Disconnect(DisconnectedEvent::Reason r = DisconnectedEvent::REQUESTED);

   public:
    Client();
    Client(const unsigned int uin, const string& password);
    ~Client();
   
    void setUIN(unsigned int uin);
    unsigned int getUIN() const;
    void setPassword(const string& password);
    string getPassword() const;

    ContactRef getSelfContact();

    bool setTranslationMap(const string& szMapFileName);
    const string& getTranslationMapFileName() const;
    const string& getTranslationMapName() const;
    bool usingDefaultMap() const;

    // -- Signals --
    /**
     *  The signal to connect to for listening to ConnectedEvent's.
     *  A ConnectedEvent is signalled when the client is online proper.
     * @see disconnected, ConnectedEvent
     */
    SigC::Signal1<void,ConnectedEvent*> connected;

    /**
     *  The signal to connect to for listening to DisconnectedEvent's.
     *  A DisconnectedEvent is signalled when you were disconnected
     *  from the server.  This could have been because it was
     *  requested, or the server might have chucked you off. More
     *  information can be in the DisconnectedEvent.
     *
     *  DisconnectedEvent's don't necessarily match a ConnectedEvent,
     *  if you try connecting with an incorrect password, you will
     *  never get a ConnectedEvent before the DisconnectedEvent
     *  signalling incorrect password.
     * @see connected, DisconnectedEvent
     */
    SigC::Signal1<void,DisconnectedEvent*> disconnected;

    /**
     *  The signal to connect to for listening to incoming
     *  MessageEvent. This includes far more than just messages.
     * @see MessageEvent
     */
    SigC::Signal1<void,MessageEvent*> messaged;

    /**
     *  The signal to connect to for listening to the acknowledgements
     *  that the library will generate for when the remote client
     *  sends back a message ack. Additionally it will it is used for
     *  signalling to the Client message delivery failures and when
     *  messages are being reattempted to be send through the server.
     * @see messaged, MessageEvent
     */
    SigC::Signal1<void,MessageEvent*> messageack;

    /**
     *  The signal to connect to for listening to Contact list events.
     * @see ContactListEvent
     */
    SigC::Signal1<void,ContactListEvent*> contactlist;

    /**
     *  The signal to connect to for listening for Contact Userinfo events.
     * @see UserInfoChangeEvent
     */
    SigC::Signal1<void,UserInfoChangeEvent*> contact_userinfo_change_signal;

    /**
     *  The signal to connect to for listening for Contact Status change events.
     * @see StatusChangeEvent
     */
    SigC::Signal1<void,StatusChangeEvent*> contact_status_change_signal;

    /**
     *  The signal for when registering a new UIN has succeeded or
     *  failed after a call to RegisterUIN().
     * @see NewUINEvent, RegisterUIN
     */
    SigC::Signal1<void,NewUINEvent*> newuin;

    /**
     *  The signal for when the server signals the rate at which the client
     *  is sending has been changed.
     * @see RateInfoChangeEvent
     */
    SigC::Signal1<void,RateInfoChangeEvent*> rate;

    /**
     *  The signal for all logging messages that are passed back to
     *  the client.  This leads to a very flexible logging system, as
     *  the user interface may decide where to write the log message
     *  to (stdout, a dialog box, etc..) and also may pick which type
     *  of log messages to display and which to ignore.
     * @see LogEvent
     */
    SigC::Signal1<void,LogEvent*> logger;

    /**
     *  The signal for socket events. All clients must listen to this
     *  and implement their particular scheme of blocking on multiple
     *  sockets for read/write/exception, usually the select system
     *  call in someway. Often toolkits will hide all the details of
     *  select inside them, such as the way gtk or gtkmm do.
     *
     * @see SocketEvent
     */
    SigC::Signal1<void,SocketEvent*> socket;

    /**
     *  The signal to connect to for listening for Self Contact Userinfo events.
     * @see UserInfoChangeEvent
     */
    SigC::Signal1<void,UserInfoChangeEvent*> self_contact_userinfo_change_signal;

    /**
     *  The signal to connect to for listening for Self Contact Status change events.
     * @see StatusChangeEvent
     */
    SigC::Signal1<void,StatusChangeEvent*> self_contact_status_change_signal;

    /**
     *  Signal when someone requests your away message. The client
     *  should setMessage in the AutoMessageEvent to what your away
     *  message is. This allows dynamic away messages for different
     *  people.
     */
    SigC::Signal1<void,ICQMessageEvent*> want_auto_resp;

    /**
     *  Signal when a Search Result has been updated.  The last signal
     *  on a search result will be with
     *  SearchResultEvent::isFinished() set to true. After this the
     *  event is finished and deleted from memory by the library.
     */
    SigC::Signal1<void,SearchResultEvent*> search_result;
    
    SigC::Signal1<void,ServerBasedContactEvent*> server_based_contact_list;
    
    // -------------

    // -- Send calls --
    void SendEvent(MessageEvent *ev);

    // -- Set Status --
    void setStatus(const Status st);
    void setStatus(const Status st, bool inv);
    void setInvisible(bool inv);
    
    Status getStatus() const;
    bool getInvisible() const;

    void uploadSelfDetails();
    
    // -- Contact List --
    void addContact(ContactRef c);
    void removeContact(const unsigned int uin);
    void addVisible(ContactRef c);
    void removeVisible(const unsigned int uin);
    void addInvisible(ContactRef c);
    void removeInvisible(const unsigned int uin);
    ContactRef getContact(const unsigned int uin);

    ContactList& getContactList();

    void fetchSimpleContactInfo(ContactRef c);
    void fetchDetailContactInfo(ContactRef c);
    void fetchServerBasedContactList();
    void fetchSelfSimpleContactInfo();
    void fetchSelfDetailContactInfo();

    // -- Whitepage searches --
    SearchResultEvent* searchForContacts(const string& nickname, const string& firstname,
					 const string& lastname);

    SearchResultEvent* searchForContacts(const string& nickname, const string& firstname,
					 const string& lastname, const string& email,
					 AgeRange age, Sex sex, unsigned char language, const string& city,
					 const string& state, unsigned short country,
					 const string& company_name, const string& department,
					 const string& position, bool only_online);

    SearchResultEvent* searchForContacts(unsigned int uin);

    /*
     *  Poll must be called regularly (at least every 60 seconds)
     *  but I recommended 5 seconds, so timeouts work with good
     *  granularity.
     *  It is not related to the socket callback - the client using
     *  this library must select() on the sockets it gets signalled
     *  and call socket_cb when select returns a status flag on one
     *  of the sockets. ickle simply uses the gtk-- built in signal handlers
     *  to do all this.
     */

    // -- Network settings --
    void setLoginServerHost(const string& host);
    string getLoginServerHost() const;

    void setLoginServerPort(const unsigned short& port);
    unsigned short getLoginServerPort() const;

    void setBOSServerOverridePort(const bool& b);
    bool getBOSServerOverridePort() const;

    void setBOSServerPort(const unsigned short& port);
    unsigned short getBOSServerPort() const;

    void setSMTPServerHost(const string& host);
    string getSMTPServerHost() const;

    void setSMTPServerPort(unsigned short port);
    unsigned short getSMTPServerPort() const;

    void setAcceptInDC(bool d);
    bool getAcceptInDC() const;

    void setUseOutDC(bool d);
    bool getUseOutDC() const;

    void Poll();
    void socket_cb(int fd, SocketEvent::Mode m);

    void RegisterUIN();

    /* isConnected() is a convenience for the
     * client, it should correspond exactly to ConnectedEvents
     * & DisconnectedEvents the client gets
     */
    bool isConnected() const;
    
  };
}

#endif

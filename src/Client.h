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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_EXT_HASH_MAP
# include <ext/hash_map>
#elif HAVE_HASH_MAP
# include <hash_map>
#else
# error "hash_map not defined"
#endif

#include <sigc++/signal_system.h>

#include <time.h>

#include "buffer.h"
#include "socket.h"
#include "SNAC.h"
#include "events.h"
#include "constants.h"
#include "Contact.h"
#include "ContactList.h"
#include "custom_marshal.h"
#include "Translator.h"
#include "RequestIDCache.h"
#include "ICBMCookieCache.h"
#include "DirectClient.h"
#include "DCCache.h"

using std::string;
using SigC::Signal1;

namespace ICQ2000 {

  // -- Status Codes Flags --
  const unsigned short STATUS_FLAG_ONLINE = 0x0000;
  const unsigned short STATUS_FLAG_AWAY = 0x0001;
  const unsigned short STATUS_FLAG_DND = 0x0002;
  const unsigned short STATUS_FLAG_NA = 0x0004;
  const unsigned short STATUS_FLAG_OCCUPIED = 0x0010;
  const unsigned short STATUS_FLAG_FREEFORCHAT = 0x0020;
  const unsigned short STATUS_FLAG_INVISIBLE = 0x0100;

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

    unsigned int m_uin;
    string m_password;

    string m_authorizerHostname;
    unsigned short m_authorizerPort;

    string m_bosHostname;
    unsigned short m_bosPort;
    bool m_bosOverridePort;

    unsigned short m_client_seq_num;
    unsigned int m_requestid;
    Status m_status;
    bool m_invisible;
    
    Translator m_translator;

    ContactList m_contact_list;

    unsigned char *m_cookie_data;
    unsigned short m_cookie_length;

    unsigned int m_ext_ip;
    TCPSocket m_serverSocket;
    TCPServer m_listenServer;

    DCCache m_dccache;   // this is for established connections
    hash_map<unsigned int, DirectClient*> m_uinmap;

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
    void DisconnectInt();

    // -- Ping server --
    void PingServer();

    DirectClient* ConnectDirect(Contact *c);
    void DisconnectDirectConns();
    void DisconnectDirectConn(int fd);

    // ------------------ Signal dispatchers -----------------
    void SignalConnect();
    void SignalDisconnect(DisconnectedEvent::Reason r);
    void SignalMessage(MessageSNAC *snac);
    void SignalMessageACK(MessageACKSNAC *snac);
    void SignalSrvResponse(SrvResponseSNAC *snac);
    void SignalUINResponse(UINResponseSNAC *snac);
    void SignalUINRequestError();
    void SignalRateInfoChange(RateInfoChangeSNAC *snac);
    void SignalLog(LogEvent::LogType type, const string& msg);
    void SignalUserOnline(BuddyOnlineSNAC *snac);
    void SignalUserOffline(BuddyOfflineSNAC *snac);
    void SignalUserAdded(Contact *c);
    void SignalUserRemoved(Contact *c);
    void SignalAddSocket(int fd, SocketEvent::Mode m);
    void SignalRemoveSocket(int fd);
    // ------------------ Outgoing packets -------------------

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

    unsigned int FLAPHeader(Buffer& b, unsigned char channel);
    void FLAPFooter(Buffer& b, unsigned int d);

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

    Contact* lookupICQ(unsigned int uin);
    Contact* lookupMobile(const string& m);

    Contact* getUserInfoCacheContact(unsigned int reqid);

    /* Maps the Status enum code to the real uint16
     * value ICQ sends and vice versa
     */
    unsigned short MapStatusToICQStatus(Status st, bool inv);
    Status MapICQStatusToStatus(unsigned short st);
    bool MapICQStatusToInvisible(unsigned short st);

    void ICBMCookieCache_expired_cb(MessageEvent *ev);
    void dccache_expired_cb(DirectClient *dc);
    void dc_connected_cb(DirectClient *dc);
    void dc_log_cb(LogEvent *ev);
    void dc_socket_cb(SocketEvent *ev);
    void dc_messaged_cb(MessageEvent *ev);
    void dc_messageack_cb(MessageEvent *ev);

    bool SendDirect(MessageEvent *ev);
    void SendViaServer(MessageEvent *ev);
    
    void Connect();
    void Disconnect(DisconnectedEvent::Reason r = DisconnectedEvent::REQUESTED);

   public:
    Client();
    Client(const unsigned int uin, const string& password);
    ~Client();
   
    /**
     *  Get the invisible status.
     * @return whether you are invisible or not
     */
    bool getInvisible() const { return m_invisible; }

    /**
     *  Set the invisible status
     * @param i Invisible boolean
     *
     * @todo Need to send new status to server, and invisible/visible lists
     */
    void setInvisible(bool i) { m_invisible = i; }

    /**
     *  Set your uin.
     *  Use to set what the uin you would like to log in as, before connecting.
     * @param uin your UIN
     */
    void setUIN(unsigned int uin) { m_uin = uin; }

    /**
     *  Get your uin.
     * @return your UIN
     */
    unsigned int getUIN() const { return m_uin; }

    /** 
     *  Set the password to use at login.
     * @param password your password
     */
    void setPassword(const string& password) { m_password = password; }

    /**
     *  Get the password you set for login
     * @return your password
     */
    string getPassword() const { return m_password; }

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
    Signal1<void,ConnectedEvent*> connected;

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
    Signal1<void,DisconnectedEvent*> disconnected;

    /**
     *  The signal to connect to for listening to incoming
     *  MessageEvent. This includes far more than just messages.
     * @see MessageEvent
     */
    Signal1<bool,MessageEvent*,StopOnTrueMarshal> messaged;

    /**
     *  The signal to connect to for listening to the acknowledgements
     *  that the library will generate for when the remote client
     *  sends back a message ack. Additionally it will it is used for
     *  signalling to the Client message delivery failures and when
     *  messages are being reattempted to be send through the server.
     * @see messaged, MessageEvent
     */
    Signal1<void,MessageEvent*> messageack;

    /**
     *  The signal to connect to for listening to Contact list events.
     * @see ContactListEvent
     */
    Signal1<void,ContactListEvent*> contactlist;

    /**
     *  The signal for when registering a new UIN has succeeded or
     *  failed after a call to RegisterUIN().
     * @see NewUINEvent, RegisterUIN
     */
    Signal1<void,NewUINEvent*> newuin;

    /**
     *  The signal for when the server signals the rate at which the client
     *  is sending has been changed.
     * @see RateInfoChangeEvent
     */
    Signal1<void,RateInfoChangeEvent*> rate;

    /**
     *  The signal for all logging messages that are passed back to
     *  the client.  This leads to a very flexible logging system, as
     *  the user interface may decide where to write the log message
     *  to (stdout, a dialog box, etc..) and also may pick which type
     *  of log messages to display and which to ignore.
     * @see LogEvent
     */
    Signal1<void,LogEvent*> logger;

    /**
     *  The signal for socket events. All clients must listen to this
     *  and implement their particular scheme of blocking on multiple
     *  sockets for read/write/exception, usually the select system
     *  call in someway. Often toolkits will hide all the details of
     *  select inside them, such as the way gtk or gtkmm do.
     *
     * @see SocketEvent
     */
    Signal1<void,SocketEvent*> socket;

    /**
     *  Signal to listen to for when the server has accepted a status
     *  change request. Your change in status has been acknowledged by
     *  the server then, and other clients will see the new status. In
     *  a User Interface the setting of the status should be done
     *  separately from the updating the display of it. The updating
     *  the display of it should only be done once you have received
     *  this signal.
     * @see MyStatusChangeEvent
     */
    Signal1<void,MyStatusChangeEvent*> statuschanged;
    // -------------

    // -- Signal Dispatchers --
    void SignalUserInfoChange(Contact *c);
    void SignalMessageQueueChanged(Contact *c);
    // ------------------------

    // -- Send calls --
    void SendEvent(MessageEvent *ev);

    // -- Set Status --
    void setStatus(const Status st);
    Status getStatus() const;

    // -- Contact List --
    void addContact(Contact& c);
    void removeContact(const unsigned int uin);
    Contact* getContact(const unsigned int uin);
    void fetchSimpleContactInfo(Contact* c);
    void fetchDetailContactInfo(Contact* c);

    void setLoginServerHost(const string& host);
    string getLoginServerHost() const;

    void setLoginServerPort(const unsigned short& port);
    unsigned short getLoginServerPort() const;

    void setBOSServerOverridePort(const bool& b);
    bool getBOSServerOverridePort() const;

    void setBOSServerPort(const unsigned short& port);
    unsigned short getBOSServerPort() const;

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

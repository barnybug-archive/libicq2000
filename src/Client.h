/**
 * libICQ2000 Client header
 *
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
    void SignalAddSocket(int fd, AddSocketHandleEvent::Mode m);
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
   
    bool getInvisible() const { return m_invisible; }
    void setInvisible(bool i) { m_invisible = i; }
    void setUIN(unsigned int uin) { m_uin = uin; }
    unsigned int getUIN() const { return m_uin; }
    void setPassword(const string& password) { m_password = password; }
    string getPassword() const { return m_password; }

    bool setTranslationMap(const string& szMapFileName);
    const string& getTranslationMapFileName() { return m_translator.getMapFileName(); }
    const string& getTranslationMapName() { return m_translator.getMapName(); }
    bool usingDefaultMap() { return m_translator.usingDefaultMap(); }

    // -- Signals --
    Signal1<void,ConnectedEvent*> connected;
    Signal1<void,DisconnectedEvent*> disconnected;
    Signal1<bool,MessageEvent*,StopOnTrueMarshal> messaged;
    Signal1<void,MessageEvent*> messageack;
    Signal1<void,ContactListEvent*> contactlist;
    Signal1<void,NewUINEvent*> newuin;
    Signal1<void,RateInfoChangeEvent*> rate;
    Signal1<void,LogEvent*> logger;
    Signal1<void,SocketEvent*> socket;
    Signal1<void,MyStatusChangeEvent*> statuschanged;
    // -------------

    // -- Signal Dispatchers --
    void SignalUserInfoChange(Contact *c);
    void SignalMessageQueueChanged(Contact *c);
    // ------------------------

    // -- Ping server --
    void PingServer();

    // -- Send calls --
    void SendEvent(MessageEvent *ev);

    // -- Set Status --
    void setStatus(const Status st);
    /* This can be called when connected to set the initial status
     * when you do connect, or can be called whilst connected to
     * change status
     */
    Status getStatus() const;

    // -- Contact List --
    void addContact(Contact& c);
    void removeContact(const unsigned int uin);
    Contact* getContact(const unsigned int uin);
    void fetchSimpleContactInfo(Contact* c);
    void fetchDetailContactInfo(Contact* c);

    void setLoginServerHost(const string& host) { m_authorizerHostname = host; }
    string getLoginServerHost() const { return m_authorizerHostname; }

    void setLoginServerPort(const unsigned short& port) { m_authorizerPort = port; }
    unsigned short getLoginServerPort() const { return m_authorizerPort; }

    void setBOSServerOverridePort(const bool& b) { m_bosOverridePort = b; }
    bool getBOSServerOverridePort() const { return m_bosOverridePort; }

    void setBOSServerPort(const unsigned short& port) { m_bosPort = port; }
    unsigned short getBOSServerPort() const { return m_bosPort; }

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
    void Poll();
    void socket_cb(int fd, SocketEvent::Mode m);

    // might be useful for select calls within client
    // or library, eg. gtkmm
    // (deprecated - clients should do Client::socket.connect() and listen
    //  for socket events now)
    int getSocketHandle() { return m_serverSocket.getSocketHandle(); }

    void RegisterUIN();

    /* isConnected() is a convenience for the
     * client, it should correspond exactly to ConnectedEvents
     * & DisconnectedEvents the client gets
     */
    bool isConnected() const;
    
  };
}

#endif

/*
 * libICQ2000 Client
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

#include "TLV.h"
#include "UserInfoBlock.h"

#include "Client.h"

#include "sstream_fix.h"

#include <sigc++/bind.h>

using std::ostringstream;
using std::endl;

namespace ICQ2000 {

  Client::Client() : m_recv(&m_translator){
    Init();
  }

  Client::Client(const unsigned int uin, const string& password) : m_uin(uin), m_password(password), m_recv(&m_translator) {
    Init();
  }

  Client::~Client() {
    if (m_cookie_data)
      delete [] m_cookie_data;
    Disconnect(DisconnectedEvent::REQUESTED);
  }

  void Client::Init() {
    m_authorizerHostname = "login.icq.com";
    m_authorizerPort = 5190;
    m_bosOverridePort = false;

    m_state = NOT_CONNECTED;
    
    m_cookie_data = NULL;
    m_cookie_length = 0;

    m_status = STATUS_OFFLINE;
    m_invisible = false;

    m_ext_ip = 0;

    m_cookiecache.setDefaultTimeout(30);
    // 30 seconds is hopefully enough for even the slowest connections
    m_cookiecache.expired.connect( slot(this,&Client::ICBMCookieCache_expired_cb) );

    m_dccache.setDefaultTimeout(30);
    // set timeout on direct connections to 30 seconds
    // this will be increased once they are established
    m_dccache.expired.connect( slot(this,&Client::dccache_expired_cb) );
  }

  unsigned short Client::NextSeqNum() {
    m_client_seq_num = ++m_client_seq_num & 0x7fff;
    return m_client_seq_num;
  }

  unsigned int Client::NextRequestID() {
    m_requestid = ++m_requestid & 0x7fffffff;
    return m_requestid;
  }

  void Client::ConnectAuthorizer(State state) {
    SignalLog(LogEvent::INFO, "Client connecting");
    try {
      /*
       * all sorts of SocketExceptions can be thrown
       * here - for
       * - sockets not being created
       * - DNS lookup failures
       */
      m_serverSocket.setRemoteHost(m_authorizerHostname.c_str());
      m_serverSocket.setRemotePort(m_authorizerPort);
      m_serverSocket.setBlocking(false);
      m_serverSocket.Connect();
    } catch(SocketException e) {
      // signal connection failure
      ostringstream ostr;
      ostr << "Failed to connect to Authorizer: " << e.what();
      SignalLog(LogEvent::ERROR, ostr.str());
      SignalDisconnect(DisconnectedEvent::FAILED_LOWLEVEL);
      return;
    }
    
    SignalAddSocket( m_serverSocket.getSocketHandle(), SocketEvent::WRITE );

    // randomize sequence number
    srand(time(0));
    m_client_seq_num = (unsigned short)(0x7fff*(rand()/(RAND_MAX+1.0)));
    m_requestid = (unsigned int)(0x7fffffff*(rand()/(RAND_MAX+1.0)));

    m_state = state;
  }

  void Client::DisconnectAuthorizer() {
    SignalRemoveSocket( m_serverSocket.getSocketHandle() );
    m_serverSocket.Disconnect();
    m_state = NOT_CONNECTED;
  }

  void Client::ConnectBOS() {
    try {
      m_serverSocket.setRemoteHost(m_bosHostname.c_str());
      m_serverSocket.setRemotePort(m_bosPort);
      m_serverSocket.setBlocking(false);
      m_serverSocket.Connect();
    } catch(SocketException e) {
      ostringstream ostr;
      ostr << "Failed to connect to BOS server: " << e.what();
      SignalLog(LogEvent::ERROR, ostr.str());
      SignalDisconnect(DisconnectedEvent::FAILED_LOWLEVEL);
      return;
    }

    SignalAddSocket( m_serverSocket.getSocketHandle(), SocketEvent::WRITE );

    m_state = BOS_AWAITING_CONN_ACK;
  }

  void Client::DisconnectBOS() {
    m_state = NOT_CONNECTED;

    SignalRemoveSocket( m_serverSocket.getSocketHandle() );
    m_serverSocket.Disconnect();
    if (m_listenServer.isStarted()) {
      SignalRemoveSocket( m_listenServer.getSocketHandle() );
      m_listenServer.Disconnect();
      DisconnectDirectConns();
    }
  }

  void Client::DisconnectDirectConns() {
    m_dccache.removeAll();
    m_uinmap.clear();
  }

  void Client::DisconnectDirectConn(int fd) {
    if (m_dccache.exists(fd)) {
      DirectClient *dc = m_dccache[fd];
      unsigned int uin = dc->getUIN();
      if ( m_uinmap.count(uin) > 0 && m_uinmap[uin] == dc) m_uinmap.erase(uin);
      m_dccache.remove(fd);
    }
  }

  // ------------------ Signal Dispatchers -----------------
  void Client::SignalConnect() {
    m_state = BOS_LOGGED_IN;
    ConnectedEvent ev;
    connected.emit(&ev);
  }

  void Client::SignalDisconnect(DisconnectedEvent::Reason r) {
    DisconnectedEvent ev(r);
    disconnected.emit(&ev);

    // ensure all contacts return to Offline
    ContactList::iterator curr = m_contact_list.begin();

    if (m_status != STATUS_OFFLINE) {
      m_status = STATUS_OFFLINE;
      MyStatusChangeEvent ev(m_status);
      statuschanged.emit( &ev );
    }

    while(curr != m_contact_list.end()) {
      Status old_st = (*curr).getStatus();
      if ( old_st != STATUS_OFFLINE ) {
	(*curr).setStatus(STATUS_OFFLINE);
	StatusChangeEvent ev(&(*curr), (*curr).getStatus(), old_st);
	contactlist.emit(&ev);
      }
      ++curr;
    }
  }

  void Client::SignalAddSocket(int fd, SocketEvent::Mode m) {
    AddSocketHandleEvent ev( fd, m );
    socket.emit(&ev);
  }

  void Client::SignalRemoveSocket(int fd) {
    RemoveSocketHandleEvent ev(fd);
    socket.emit(&ev);
  }

  void Client::SignalMessage(MessageSNAC *snac) {
    Contact *contact;
    MessageEvent *e = NULL;
    ICQSubType *st = snac->getICQSubType();
    if (st == NULL) return;

    if (st->getType() == MSG_Type_Normal) {
      NormalICQSubType *nst = static_cast<NormalICQSubType*>(st);
      
      contact = lookupICQ( nst->getSource() );
      e = new NormalMessageEvent(contact,
				 nst->getMessage(), nst->isMultiParty() );
      
      if (nst->isAdvanced()) SendAdvancedACK(snac);

    } else if (st->getType() == MSG_Type_URL) {
      URLICQSubType *ust = static_cast<URLICQSubType*>(st);
      
      contact = lookupICQ( ust->getSource() );
      e = new URLMessageEvent(contact,
			      ust->getMessage(),
			      ust->getURL());

      if (ust->isAdvanced()) SendAdvancedACK(snac);

    } else if (st->getType() == MSG_Type_SMS) {
      SMSICQSubType *sst = static_cast<SMSICQSubType*>(st);
      
      if (sst->getSMSType() == SMSICQSubType::SMS) {
	contact = lookupMobile(sst->getSender());
	e = new SMSMessageEvent(contact,
				sst->getMessage(),
				sst->getSource(),
				sst->getSenders_network(),
				sst->getTime());

      } else if (sst->getSMSType() == SMSICQSubType::SMS_Receipt) {
	contact = lookupMobile(sst->getDestination());
	e = new SMSReceiptEvent(contact,
				sst->getMessage(),
				sst->getMessageId(),
				sst->getSubmissionTime(),
				sst->getDeliveryTime(),
				sst->delivered());
      }

    } else if (st->getType() == MSG_Type_AuthReq) {
      AuthReqICQSubType *ust = static_cast<AuthReqICQSubType*>(st);
      
      contact = lookupICQ( ust->getSource() );
      e = new AuthReqEvent(contact,ust->getNick(),ust->getFirstName(),
			       ust->getLastName(),ust->getEmail(),
                               ust->getMessage());

    } else if (st->getType() == MSG_Type_AuthRej) {
      AuthRejICQSubType *ust = static_cast<AuthRejICQSubType*>(st);
    
      contact = lookupICQ( ust->getSource() );
      e = new AuthAckEvent(contact, ust->getMessage(), false);

    } else if (st->getType() == MSG_Type_AuthAcc) {
      AuthAccICQSubType *ust = static_cast<AuthAccICQSubType*>(st);
    
      contact = lookupICQ( ust->getSource() );
      e = new AuthAckEvent(contact, true);

      }

    if (e != NULL) {
      contact->addPendingMessage(e);
      if (messaged.emit(e)) {
        contact->erasePendingMessage(e);
        SignalMessageQueueChanged(contact);
      }
    }

  }


  void Client::SignalMessageACK(MessageACKSNAC *snac) {
    UINRelatedSubType *st = snac->getICQSubType();
    if (st == NULL) return;
    Contact *contact = lookupICQ( st->getSource() );

    unsigned char type = st->getType();
    if (type == MSG_Type_AutoReq_Away
	|| type == MSG_Type_AutoReq_Occ
	|| type == MSG_Type_AutoReq_NA
	|| type == MSG_Type_AutoReq_DND
	|| type == MSG_Type_AutoReq_FFC) {
      AwayMsgSubType *ast = static_cast<AwayMsgSubType*>(st);
      ICBMCookie c = snac->getICBMCookie();
      if ( m_cookiecache.exists( c ) ) {
	MessageEvent *ev = m_cookiecache[c];
	AwayMessageEvent *aev = dynamic_cast<AwayMessageEvent*>(ev);
	if (aev != NULL) {
	  aev->setMessage( ast->getMessage() );
	  aev->setFinished(true);
	  aev->setDelivered(true);
	  aev->setDirect(false);
	  messageack.emit(aev);
	} else {
	  SignalLog(LogEvent::WARN, "Received ACK for Away Message when Message Event types don't match");
	}
	m_cookiecache.remove(c);
      } else {
	SignalLog(LogEvent::WARN, "Received ACK for unknown message");
      }
    } else if (type == MSG_Type_Normal
	       || type == MSG_Type_URL) {
      ICBMCookie c = snac->getICBMCookie();
      if ( m_cookiecache.exists( c ) ) {
	MessageEvent *ev = m_cookiecache[c];
	ev->setFinished(true);
	ev->setDelivered(true);
	ev->setDirect(false);
	messageack.emit(ev);
	
	m_cookiecache.remove(c);
      } else {
	SignalLog(LogEvent::WARN, "Received ACK for unknown message");
      }
      
    }

  }

  void Client::dc_messaged_cb(MessageEvent *ev) {
    Contact *contact = ev->getContact();
    contact->addPendingMessage(ev);
    if (messaged.emit(ev)) {
      contact->erasePendingMessage(ev);
      SignalMessageQueueChanged(contact);
    }
  }

  void Client::dc_messageack_cb(MessageEvent *ev) {
    messageack.emit(ev);

    if (!ev->isFinished()) {
      ev->getContact()->setDirect(false);
      // attempt to deliver via server instead
      SendViaServer(ev);
    }
  }

  void Client::SignalSrvResponse(SrvResponseSNAC *snac) {
    if (snac->getType() == SrvResponseSNAC::OfflineMessagesComplete) {

      /* We are now meant to ACK this to say
       * the we have got the offline messages
       * and the server can dispose of storing
       * them
       */
      SendOfflineMessagesACK();

    } else if (snac->getType() == SrvResponseSNAC::OfflineMessage) {

      unsigned int uin = snac->getSenderUIN();
      Contact *contact = lookupICQ(uin);
      ICQSubType *st = snac->getICQSubType();

      MessageEvent *e = NULL;
      if (st->getType() == MSG_Type_Normal) {
	NormalICQSubType *nst = static_cast<NormalICQSubType*>(st);
	e = new NormalMessageEvent(contact,
				   nst->getMessage(),
				   snac->getTime(), nst->isMultiParty() );
      } else if (st->getType() == MSG_Type_URL) {
	URLICQSubType *ust = static_cast<URLICQSubType*>(st);
	e = new URLMessageEvent(contact,
				ust->getMessage(),
				ust->getURL(),
				snac->getTime());
      } else if (st->getType() == MSG_Type_AuthReq) {
        AuthReqICQSubType *ust = static_cast<AuthReqICQSubType*>(st);
        e = new AuthReqEvent(contact,ust->getNick(),ust->getFirstName(),
			     ust->getLastName(),ust->getEmail(),
                             ust->getMessage(), snac->getTime());
      } else if (st->getType() == MSG_Type_AuthRej) {
        AuthRejICQSubType *ust = static_cast<AuthRejICQSubType*>(st);
        e = new AuthAckEvent(contact, ust->getMessage(), false, 
                             snac->getTime());
      } else if (st->getType() == MSG_Type_AuthAcc) {
        AuthAccICQSubType *ust = static_cast<AuthAccICQSubType*>(st);
        e = new AuthAckEvent(contact, true, snac->getTime());
      }
      
      if (e != NULL) {
	contact->addPendingMessage(e);
        if (messaged.emit(e)) {
          contact->erasePendingMessage(e);
          SignalMessageQueueChanged(contact);
        }
      }
      
    } else if (snac->getType() == SrvResponseSNAC::SMS_Error) {
      // mmm
    } else if (snac->getType() == SrvResponseSNAC::SMS_Response) {
      
      unsigned int reqid = snac->RequestID();
      if ( m_reqidcache.exists( reqid ) ) {
	RequestIDCacheValue *v = m_reqidcache[ reqid ];
	
	if ( v->getType() == RequestIDCacheValue::SMSMessage ) {
	  SMSEventCacheValue *uv = static_cast<SMSEventCacheValue*>(v);
	  SMSMessageEvent *ev = uv->getEvent();

	  if (snac->deliverable()) {
	    ev->setFinished(true);
	    ev->setDelivered(true);
	    ev->setDirect(false);
	    messageack.emit(ev);
	    m_reqidcache.remove( reqid );
	  } else {
	    if (snac->getErrorParam() != "DUPLEX RESPONSE") {
	      // ignore DUPLEX RESPONSE since I always get that
	      ev->setFinished(true);
	      ev->setDelivered(false);
	      ev->setDirect(false);
	      messageack.emit(ev);
	      m_reqidcache.remove( reqid );
	    }
	  }
	
	} else {
	  throw ParseException("Request ID cached value is not for an SMS Message");
	}
      } else {
	throw ParseException("Received an SMS response for unknown request id");
      }
      
    } else if (snac->getType() == SrvResponseSNAC::SimpleUserInfo) {
      // update Contact
      if ( m_contact_list.exists( snac->getUIN() ) ) {
	Contact& c = m_contact_list[ snac->getUIN() ];
	c.setAlias( snac->getAlias() );
	c.setEmail( snac->getEmail() );
	c.setFirstName( snac->getFirstName() );
	c.setLastName( snac->getLastName() );
	UserInfoChangeEvent ev(&c);
	contactlist.emit(&ev);
      }
      
    } else if (snac->getType() == SrvResponseSNAC::RMainHomeInfo) {

      try {
	Contact* c = getUserInfoCacheContact( snac->RequestID() );
	c->setMainHomeInfo( snac->getMainHomeInfo() );
	UserInfoChangeEvent ev(c);
	contactlist.emit(&ev);
      } catch(ParseException e) {
	SignalLog(LogEvent::WARN, e.what());
      }
	
    } else if (snac->getType() == SrvResponseSNAC::RHomepageInfo) {

      try {
	Contact* c = getUserInfoCacheContact( snac->RequestID() );
	c->setHomepageInfo( snac->getHomepageInfo() );
	UserInfoChangeEvent ev(c);
	contactlist.emit(&ev);
      } catch(ParseException e) {
	SignalLog(LogEvent::WARN, e.what());
      }

    } else if (snac->getType() == SrvResponseSNAC::RAboutInfo) {

      try {
	Contact* c = getUserInfoCacheContact( snac->RequestID() );
	c->setAboutInfo( snac->getAboutInfo() );
	UserInfoChangeEvent ev(c);
	contactlist.emit(&ev);
      } catch(ParseException e) {
	SignalLog(LogEvent::WARN, e.what());
      }

    }
  }
  
  Contact* Client::getUserInfoCacheContact(unsigned int reqid) {

    if ( m_reqidcache.exists( reqid ) ) {
      RequestIDCacheValue *v = m_reqidcache[ reqid ];

      if ( v->getType() == RequestIDCacheValue::UserInfo ) return v->getContact();
      else throw ParseException("Request ID cached value is not for a User Info request");

    } else {
      throw ParseException("Received a UserInfo response for unknown request id");
    }

  }

  void Client::SignalUINResponse(UINResponseSNAC *snac) {
    unsigned int uin = snac->getUIN();
    NewUINEvent e(uin);
    newuin.emit(&e);
  }

  void Client::SignalUINRequestError() {
    NewUINEvent e(0,false);
    newuin.emit(&e);
  }
  
  void Client::SignalRateInfoChange(RateInfoChangeSNAC *snac) {
    RateInfoChangeEvent e(snac->getCode(), snac->getRateClass(),
			  snac->getWindowSize(), snac->getClear(),
			  snac->getAlert(), snac->getLimit(),
			  snac->getDisconnect(), snac->getCurrentAvg(),
			  snac->getMaxAvg());
    rate.emit(&e);
  }

  void Client::SignalLog(LogEvent::LogType type, const string& msg) {
    LogEvent ev(type,msg);
    logger.emit(&ev);
  }
  
  void Client::ICBMCookieCache_expired_cb(MessageEvent *ev) {
    SignalLog(LogEvent::WARN, "Message timeout without receiving ACK");
    ev->setFinished(true);
    ev->setDelivered(false);
    ev->setDirect(false);
    messageack.emit(ev);
  }

  void Client::dccache_expired_cb(DirectClient *dc) {
    SignalLog(LogEvent::WARN, "Direct connection timeout reached");
    unsigned int uin = dc->getUIN();
    if ( m_uinmap.count(uin) > 0 && m_uinmap[uin] == dc) m_uinmap.erase(uin);
  }

  void Client::dc_connected_cb(DirectClient *dc) {
    m_uinmap[ dc->getUIN() ] = dc;
    m_dccache.setTimeout(dc->getfd(), 600);
    // once we are properly connected a direct
    // connection will only timeout after 10 mins
  }

  void Client::dc_log_cb(LogEvent *ev) {
    logger.emit(ev);
  }

  void Client::dc_socket_cb(SocketEvent *ev) {
    socket.emit(ev);
  }

  void Client::SignalUserOnline(BuddyOnlineSNAC *snac) {
    const UserInfoBlock& userinfo = snac->getUserInfo();
    if (m_contact_list.exists(userinfo.getUIN())) {
      Contact& c = m_contact_list[userinfo.getUIN()];
      Status old_st = c.getStatus();
      c.setDirect(true); // reset flags when a user goes online
      c.setStatus( MapICQStatusToStatus(userinfo.getStatus()) );
      c.setInvisible( MapICQStatusToInvisible(userinfo.getStatus()) );
      c.setExtIP( userinfo.getExtIP() );
      c.setLanIP( userinfo.getLanIP() );
      c.setExtPort( userinfo.getExtPort() );
      c.setLanPort( userinfo.getLanPort() );
      c.setTCPVersion( userinfo.getTCPVersion() );
      StatusChangeEvent ev(&c, c.getStatus(), old_st);
      contactlist.emit(&ev);

      ostringstream ostr;
      ostr << "Received Buddy Online for " << c.getAlias() << " (" << c.getUIN() << ") from server";
      SignalLog(LogEvent::INFO, ostr.str() );
    } else {
      ostringstream ostr;
      ostr << "Received Status change for user not on contact list: " << userinfo.getUIN();
      SignalLog(LogEvent::WARN, ostr.str());
    }
  }

  void Client::SignalUserOffline(BuddyOfflineSNAC *snac) {
    const UserInfoBlock& userinfo = snac->getUserInfo();
    if (m_contact_list.exists(userinfo.getUIN())) {
      Contact& c = m_contact_list[userinfo.getUIN()];
      Status old_st = c.getStatus();
      c.setStatus(STATUS_OFFLINE);
      StatusChangeEvent ev(&c, c.getStatus(), old_st);
      contactlist.emit(&ev);

      ostringstream ostr;
      ostr << "Received Buddy Offline for " << c.getAlias() << " (" << c.getUIN() << ") from server";
      SignalLog(LogEvent::INFO, ostr.str() );
    } else {
      ostringstream ostr;
      ostr << "Received Status change for user not on contact list: " << userinfo.getUIN();
      SignalLog(LogEvent::WARN, ostr.str());
    }
  }

  // ------------------ Outgoing packets -------------------

  unsigned int Client::FLAPHeader(Buffer& b, unsigned char channel) {
    b << (unsigned char) 42;
    b << channel;
    b << NextSeqNum();
    b << (unsigned short) 0; // this is filled out later
    return b.size();
  }

  void Client::FLAPFooter(Buffer& b, unsigned int d) {
    unsigned short len;
    len = b.size() - d;
    b[d-2] = len >> 8;
    b[d-1] = len & 0xFF;
  }

  void Client::SendAuthReq() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x01);

    b << (unsigned int)0x00000001;

    b << ScreenNameTLV(Contact::UINtoString(m_uin))
      << PasswordTLV(m_password)
      << ClientProfileTLV("ICQ Inc. - Product of ICQ (TM).2000b.4.63.1.3279.85")
      << ClientTypeTLV(266)
      << ClientVersionMajorTLV(4)
      << ClientVersionMinorTLV(63)
      << ClientICQNumberTLV(1)
      << ClientBuildMajorTLV(3279)
      << ClientBuildMinorTLV(85)
      << LanguageTLV("en")
      << CountryCodeTLV("us");

    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Authorisation Request");
    Send(b);
  }

  void Client::SendNewUINReq() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x01);

    b << (unsigned int)0x00000001;
    FLAPFooter(b,d);
    Send(b);
    b.clear();

    d = FLAPHeader(b,0x02);
    UINRequestSNAC sn(m_password);
    b << sn;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending New UIN Request");
    Send(b);
  }
    
  void Client::SendCookie() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x01);

    b << (unsigned int)0x00000001;

    b << CookieTLV(m_cookie_data, m_cookie_length);

    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Login Cookie");
    Send(b);
  }
    
  void Client::SendCapabilities() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    CapabilitiesSNAC cs;
    b << cs;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Capabilities");
    Send(b);
  }

  void Client::SendSetUserInfo() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    SetUserInfoSNAC cs;
    b << cs;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Set User Info");
    Send(b);
  }

  void Client::SendRateInfoRequest() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    RequestRateInfoSNAC rs;
    b << rs;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Rate Info Request");
    Send(b);
  }
  
  void Client::SendRateInfoAck() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    RateInfoAckSNAC rs;
    b << rs;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Rate Info Ack");
    Send(b);
  }

  void Client::SendPersonalInfoRequest() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    PersonalInfoRequestSNAC us;
    b << us;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Personal Info Request");
    Send(b);
  }

  void Client::SendAddICBMParameter() {
    Buffer b(&m_translator);
    unsigned int d = FLAPHeader(b,0x02);
    MsgAddICBMParameterSNAC ms;
    b << ms;
    FLAPFooter(b,d);
    SignalLog(LogEvent::INFO, "Sending Add ICBM Parameter");
    Send(b);
  }

  void Client::SendLogin() {
    Buffer b(&m_translator);
    unsigned int d;

    // startup listening server at this point, so we
    // know the listening port and ip
    m_listenServer.StartServer();
    SignalAddSocket( m_listenServer.getSocketHandle(), SocketEvent::READ );
    ostringstream ostr;
    ostr << "Server listening on " << IPtoString( m_serverSocket.getLocalIP() ) << ":" << m_listenServer.getPort();
    SignalLog(LogEvent::INFO, ostr.str());

    if (!m_contact_list.empty()) {
      d = FLAPHeader(b,0x02);
      AddBuddySNAC abs(m_contact_list);
      b << abs;
      FLAPFooter(b,d);
    }

    d = FLAPHeader(b,0x02);
    SetStatusSNAC sss(MapStatusToICQStatus(m_status, m_invisible));

    // explicitly set status to offline. If the user set the status
    // before calling Connect and we don't do this, we'll miss the
    // status change upon the user info reception and will not emit
    // the statuschanged signal correctly
    m_status = STATUS_OFFLINE;

    sss.setSendExtra(true);
    sss.setIP( m_serverSocket.getLocalIP() );
    sss.setPort( m_listenServer.getPort() );
    b << sss;
    FLAPFooter(b,d);

    //    d = FLAPHeader(b,0x02);
    //    SetIdleSNAC sis;
    //    b << sis;
    //    FLAPFooter(b,d);

    d = FLAPHeader(b,0x02);
    ClientReadySNAC crs;
    b << crs;
    FLAPFooter(b,d);

    d = FLAPHeader(b,0x02);
    SrvRequestOfflineSNAC ssnac(m_uin);
    b << ssnac;
    FLAPFooter(b,d);

    SignalLog(LogEvent::INFO, "Sending Contact List, Status, Client Ready and Offline Messages Request");
    Send(b);

    SignalConnect();
    m_last_server_ping = time(NULL);
  }

  void Client::SendOfflineMessagesRequest() {
    Buffer b(&m_translator);
    unsigned int d;

    d = FLAPHeader(b,0x02);
    SrvRequestOfflineSNAC ssnac(m_uin);
    b << ssnac;
    FLAPFooter(b,d);

    SignalLog(LogEvent::INFO, "Sending Offline Messages Request");
    Send(b);
  }


  void Client::SendOfflineMessagesACK() {
    Buffer b(&m_translator);
    unsigned int d;

    d = FLAPHeader(b,0x02);
    SrvAckOfflineSNAC ssnac(m_uin);
    b << ssnac;
    FLAPFooter(b,d);

    SignalLog(LogEvent::INFO, "Sending Offline Messages ACK");
    Send(b);
  }

  void Client::SendAdvancedACK(MessageSNAC *snac) {
    ICQSubType *st = snac->getICQSubType();
    if (st == NULL || dynamic_cast<UINRelatedSubType*>(st) == NULL ) return;
    UINRelatedSubType *ust = dynamic_cast<UINRelatedSubType*>(snac->grabICQSubType());

    Buffer b(&m_translator);
    unsigned int d;
    
    d = FLAPHeader(b,0x02);
    
    MessageACKSNAC ssnac( snac->getICBMCookie(), ust );
    b << ssnac;
    FLAPFooter(b,d);

    SignalLog(LogEvent::INFO, "Sending Advanced Message ACK");
    Send(b);
  }

  void Client::Send(Buffer& b) {
    try {
      ostringstream ostr;
      ostr << "Sending packet to Server" << endl << b;
      SignalLog(LogEvent::PACKET, ostr.str());
      m_serverSocket.Send(b);
    } catch(SocketException e) {
      ostringstream ostr;
      ostr << "Failed to send: " << e.what();
      SignalLog(LogEvent::ERROR, ostr.str());
      Disconnect(DisconnectedEvent::FAILED_LOWLEVEL);
    }
  }    

  // ------------------ Incoming packets -------------------

  void Client::RecvFromServer() {

    try {
      while (m_serverSocket.connected()) {
	if (!m_serverSocket.Recv(m_recv)) break;
	ostringstream ostr;
	ostr << "Received packet from Server" << endl << m_recv;
	SignalLog(LogEvent::PACKET, ostr.str());
	Parse();
      }
    } catch(SocketException e) {
      ostringstream ostr;
      ostr << "Failed on recv: " << e.what();
      SignalLog(LogEvent::ERROR, ostr.str());
      Disconnect(DisconnectedEvent::FAILED_LOWLEVEL);
    }
  }

  void Client::Parse() {
    
    // -- FLAP header --

    unsigned char start_byte, channel;
    unsigned short seq_num, data_len;

    // process FLAP(s) in packet

    if (m_recv.empty()) return;

    while (!m_recv.empty()) {
      m_recv.setPos(0);

      m_recv >> start_byte;
      if (start_byte != 42) {
	m_recv.clear();
	SignalLog(LogEvent::WARN, "Invalid Start Byte on FLAP");
	return;
      }

      /* if we don't have at least six bytes we don't have enough
       * info to determine if we have the whole of the FLAP
       */
      if (m_recv.remains() < 5) return;
      
      m_recv >> channel;
      m_recv >> seq_num; // check sequence number - todo
      
      m_recv >> data_len;
      if (m_recv.remains() < data_len) return; // waiting for more of the FLAP

      /* Copy into another Buffer which is passed
       * onto the separate parse code that way
       * multiple FLAPs in one packet are split up
       */
      Buffer sb(&m_translator);
      m_recv.chopOffBuffer( sb, data_len+6 );
      sb.advance(6);

      // -- FLAP body --
      
      ostringstream ostr;
      
      switch(channel) {
      case 1:
	ParseCh1(sb,seq_num);
	break;
      case 2:
	ParseCh2(sb,seq_num);
	break;
      case 3:
	ParseCh3(sb,seq_num);
	break;
      case 4:
	ParseCh4(sb,seq_num);
	break;
      default:
	ostr << "FLAP on unrecognised channel 0x" << hex << (int)channel;
	SignalLog(LogEvent::WARN, ostr.str());
	break;
      }

      if (sb.beforeEnd()) {
	/* we assert that parsing code eats uses all data
	 * in the FLAP - seems useful to know when they aren't
	 * as it probably means they are faulty
	 */
	ostringstream ostr;
	ostr  << "Buffer pointer not at end after parsing FLAP was: 0x" << hex << sb.pos()
	      << " should be: 0x" << sb.size();
	SignalLog(LogEvent::WARN, ostr.str());
      }
      
    }

  }

  void Client::ParseCh1(Buffer& b, unsigned short seq_num) {

    if (b.remains() == 4 && (m_state == AUTH_AWAITING_CONN_ACK || 
			     m_state == UIN_AWAITING_CONN_ACK)) {

      // Connection Acknowledge - first packet from server on connection
      unsigned int unknown;
      b >> unknown; // always 0x0001

      if (m_state == AUTH_AWAITING_CONN_ACK) {
	SendAuthReq();
	SignalLog(LogEvent::INFO, "Connection Acknowledge from server");
	m_state = AUTH_AWAITING_AUTH_REPLY;
      } else if (m_state == UIN_AWAITING_CONN_ACK) {
	SendNewUINReq();
	SignalLog(LogEvent::INFO, "Connection Acknowledge from server");
	m_state = UIN_AWAITING_UIN_REPLY;
      }

    } else if (b.remains() == 4 && m_state == BOS_AWAITING_CONN_ACK) {

      SignalLog(LogEvent::INFO, "Connection Acknowledge from server");

      // Connection Ack, send the cookie
      unsigned int unknown;
      b >> unknown; // always 0x0001

      SendCookie();
      m_state = BOS_AWAITING_LOGIN_REPLY;

    } else {
      SignalLog(LogEvent::WARN, "Unknown packet received on channel 0x01");
    }

  }

  void Client::ParseCh2(Buffer& b, unsigned short seq_num) {
    InSNAC *snac;
    try {
      snac = ParseSNAC(b);
    } catch(ParseException e) {
      ostringstream ostr;
      ostr << "Problem parsing SNAC: " << e.what();
      SignalLog(LogEvent::WARN, ostr.str());
      return;
    }

    switch(snac->Family()) {
      
    case SNAC_FAM_GEN:
      switch(snac->Subtype()) {
      case SNAC_GEN_ServerReady:
	SignalLog(LogEvent::INFO, "Received Server Ready from server");
	SendCapabilities();
	break;
      case SNAC_GEN_RateInfo:
	SignalLog(LogEvent::INFO, "Received Rate Information from server");
	SendRateInfoAck();
	SendPersonalInfoRequest();
	SendAddICBMParameter();
	SendSetUserInfo();
	SendLogin();
	break;
      case SNAC_GEN_CapAck:
	SignalLog(LogEvent::INFO, "Received Capabilities Ack from server");
	SendRateInfoRequest();
	break;
      case SNAC_GEN_UserInfo:
	SignalLog(LogEvent::INFO, "Received User Info from server");
	HandleUserInfoSNAC(static_cast<UserInfoSNAC*>(snac));
	break;
      case SNAC_GEN_MOTD:
	SignalLog(LogEvent::INFO, "Received MOTD from server");
	//	SignalConnect();
	/* take the MOTD as the sign we
	 * are online proper
	 * - unfortunately they seemed to have stopped sending that
	 *   so this isn't the sign anymore.
	 */
	break;
      case SNAC_GEN_RateInfoChange:
	SignalLog(LogEvent::INFO, "Received Rate Info Change from server");
	SignalRateInfoChange(static_cast<RateInfoChangeSNAC*>(snac));
	break;
      }
      break;

    case SNAC_FAM_BUD:
      switch(snac->Subtype()) {
      case SNAC_BUD_Online:
	SignalUserOnline(static_cast<BuddyOnlineSNAC*>(snac));
	break;
      case SNAC_BUD_Offline:
	SignalUserOffline(static_cast<BuddyOfflineSNAC*>(snac));
	break;
      }
      break;

    case SNAC_FAM_MSG:
      switch(snac->Subtype()) {
      case SNAC_MSG_Message:
	SignalLog(LogEvent::INFO, "Received Message from server");
	SignalMessage(static_cast<MessageSNAC*>(snac));
	break;
      case SNAC_MSG_MessageACK:
	SignalLog(LogEvent::INFO, "Received Message ACK from server");
	SignalMessageACK(static_cast<MessageACKSNAC*>(snac));
	break;
      }
      break;

    case SNAC_FAM_SRV:
      switch(snac->Subtype()) {
      case SNAC_SRV_Response:
	SignalLog(LogEvent::INFO, "Received Server Response from server");
	SignalSrvResponse(static_cast<SrvResponseSNAC*>(snac));
	break;
      }
      break;
    case SNAC_FAM_UIN:
      switch(snac->Subtype()) {
      case SNAC_UIN_Response:
	SignalLog(LogEvent::INFO, "Received UIN Response from server");
	SignalUINResponse(static_cast<UINResponseSNAC*>(snac));
	break;
      case SNAC_UIN_RequestError:
	SignalLog(LogEvent::ERROR, "Received UIN Request Error from server");
	SignalUINRequestError();
	break;
      }
      break;
	
	
    } // switch(Family)

    if (dynamic_cast<RawSNAC*>(snac)) {
      ostringstream ostr;
      ostr << "Unknown SNAC packet received - Family: 0x" << hex << snac->Family()
	   << " Subtype: 0x" << snac->Subtype();
      SignalLog(LogEvent::WARN, ostr.str());
    }

    delete snac;

  }

  void Client::ParseCh3(Buffer& b, unsigned short seq_num) {
    SignalLog(LogEvent::INFO, "Received packet on channel 0x03");
  }

  void Client::ParseCh4(Buffer& b, unsigned short seq_num) {
    if (m_state == AUTH_AWAITING_AUTH_REPLY || m_state == UIN_AWAITING_UIN_REPLY) {
      // An Authorisation Reply / Error
      TLVList tlvlist;
      tlvlist.Parse(b, TLV_ParseMode_Channel04, (short unsigned int)-1);

      if (tlvlist.exists(TLV_Cookie) && tlvlist.exists(TLV_Redirect)) {

	RedirectTLV *r = static_cast<RedirectTLV*>(tlvlist[TLV_Redirect]);
	ostringstream ostr;
	ostr << "Redirected to: " << r->getHost();
	if (r->getPort() != 0) ostr << " port: " << dec << r->getPort();
	SignalLog(LogEvent::INFO, ostr.str());

	m_bosHostname = r->getHost();
	if (!m_bosOverridePort)	{
	  if (r->getPort() != 0) m_bosPort = r->getPort();
	  else m_bosPort = m_authorizerPort;
	}

	// Got our cookie - yum yum :-)
	CookieTLV *t = static_cast<CookieTLV*>(tlvlist[TLV_Cookie]);
	m_cookie_length = t->Length();

	if (m_cookie_data) delete [] m_cookie_data;
	m_cookie_data = new unsigned char[m_cookie_length];

	memcpy(m_cookie_data, t->Value(), m_cookie_length);

	SignalLog(LogEvent::INFO, "Authorisation accepted");
	
	DisconnectAuthorizer();
	ConnectBOS();

      } else {
	// Problemo
	DisconnectedEvent::Reason st;

	if (tlvlist.exists(TLV_ErrorCode)) {
	  ErrorCodeTLV *t = static_cast<ErrorCodeTLV*>(tlvlist[TLV_ErrorCode]);
	  ostringstream ostr;
	  ostr << "Error logging in Error Code: " << t->Value();
	  SignalLog(LogEvent::ERROR, ostr.str());
	  switch(t->Value()) {
	  case 0x01:
	    st = DisconnectedEvent::FAILED_BADUSERNAME;
	    break;
	  case 0x02:
	    st = DisconnectedEvent::FAILED_TURBOING;
	    break;
	  case 0x03:
	    st = DisconnectedEvent::FAILED_BADPASSWORD;
	    break;
	  case 0x05:
	    st = DisconnectedEvent::FAILED_MISMATCH_PASSWD;
	    break;
	  case 0x18:
	    st = DisconnectedEvent::FAILED_TURBOING;
	    break;
	  default:
	    st = DisconnectedEvent::FAILED_UNKNOWN;
	  }
	} else if (m_state == AUTH_AWAITING_AUTH_REPLY) {
	    SignalLog(LogEvent::ERROR, "Error logging in, no error code given(?!)");
	    st = DisconnectedEvent::FAILED_UNKNOWN;
	} else {
	  st = DisconnectedEvent::REQUESTED;
	}
	SignalDisconnect(st); // signal client (error)
	DisconnectAuthorizer();
      }

    } else {

      TLVList tlvlist;
      tlvlist.Parse(b, TLV_ParseMode_Channel04, (short unsigned int)-1);

      DisconnectedEvent::Reason st;
      
      if (tlvlist.exists(TLV_DisconnectReason)) {
	DisconnectReasonTLV *t = static_cast<DisconnectReasonTLV*>(tlvlist[TLV_DisconnectReason]);
	  switch(t->Value()) {
	  case 0x0001:
	    st = DisconnectedEvent::FAILED_DUALLOGIN;
	    break;
	  default:
	    st = DisconnectedEvent::FAILED_UNKNOWN;
	  }

	} else {
	  SignalLog(LogEvent::WARN, "Unknown packet received on channel 4, disconnecting");
	  st = DisconnectedEvent::FAILED_UNKNOWN;
	}
	SignalDisconnect(st); // signal client (error)
	DisconnectBOS();
    }

  }

  // -----------------------------------------------------

  /*
   *  Poll must be called regularly (at least every 60 seconds)
   *  but I recommended 5 seconds, so timeouts work with good
   *  granularity.
   */
  void Client::Poll() {
    time_t now = time(NULL);
    if (now > m_last_server_ping + 60) {
      PingServer();
      m_last_server_ping = now;

    }

    // take the opportunity to clearout caches
    m_reqidcache.clearoutPoll();
    m_cookiecache.clearoutPoll();
    m_dccache.clearoutPoll();
  }

  void Client::socket_cb(int fd, SocketEvent::Mode m) {

    if ( fd == m_serverSocket.getSocketHandle() ) {
      /*
       * File descriptor is the socket we have open to server
       */

      /*
	if (m & SocketEvent::WRITE) SignalLog(LogEvent::INFO, "socket_cb for write");
	if (m & SocketEvent::READ) SignalLog(LogEvent::INFO, "socket_cb for read");
	if (m & SocketEvent::EXCEPTION) SignalLog(LogEvent::INFO, "socket_cb for exception");

	if (m_serverSocket.getState() == TCPSocket::NOT_CONNECTED) SignalLog(LogEvent::INFO, "server socket in state NOT_CONNECTED");
	if (m_serverSocket.getState() == TCPSocket::NONBLOCKING_CONNECT) SignalLog(LogEvent::INFO, "server socket in state NONBLOCKING_CONNECT");
	if (m_serverSocket.getState() == TCPSocket::CONNECTED) SignalLog(LogEvent::INFO, "server socket in state CONNECTED");
      */

      if (m_serverSocket.getState() == TCPSocket::NONBLOCKING_CONNECT
	  && (m & SocketEvent::WRITE)) {
	// the non-blocking connect has completed (good/bad)

	SignalRemoveSocket(fd);
	// no longer select on write

	try {
	  m_serverSocket.FinishNonBlockingConnect();
	} catch(SocketException e) {
	  // signal connection failure
	  ostringstream ostr;
	  ostr << "Failed on non-blocking connect: " << e.what();
	  SignalLog(LogEvent::ERROR, ostr.str());
	  Disconnect(DisconnectedEvent::FAILED_LOWLEVEL);
	  return;
	}

	SignalAddSocket(fd, SocketEvent::READ);
	// select on read now
	
      } else if (m_serverSocket.getState() == TCPSocket::CONNECTED && (m & SocketEvent::READ)) { 
	RecvFromServer();
      } else {
	SignalLog(LogEvent::ERROR, "Server socket in inconsistent state!");
	Disconnect(DisconnectedEvent::FAILED_LOWLEVEL);
      }
      
    } else if ( fd == m_listenServer.getSocketHandle() ) {
      /*
       * File descriptor is the listening socket - someone is connected in
       */

      TCPSocket *sock = m_listenServer.Accept();
      DirectClient *dc = new DirectClient(sock, &m_contact_list, m_uin, m_ext_ip, m_listenServer.getPort(), &m_translator);
      m_dccache[ sock->getSocketHandle() ] = dc;
      dc->logger.connect( slot(this, &Client::dc_log_cb) );
      dc->messaged.connect( slot(this, &Client::dc_messaged_cb) );
      dc->messageack.connect( slot(this, &Client::dc_messageack_cb) );
      dc->connected.connect( SigC::bind<DirectClient*>( slot(this, &Client::dc_connected_cb), dc ) );
      dc->socket.connect( slot(this, &Client::dc_socket_cb) );
      SignalAddSocket( sock->getSocketHandle(), SocketEvent::READ );

    } else {
      /*
       * File descriptor is a direct connection we have open to someone
       *
       */

      DirectClient *dc;
      if (m_dccache.exists(fd)) {
	dc = m_dccache[fd];
      } else {
	SignalLog(LogEvent::ERROR, "Problem: Unassociated socket");
	return;
      }

      TCPSocket *sock = dc->getSocket();
      if (sock->getState() == TCPSocket::NONBLOCKING_CONNECT
	  && (m & SocketEvent::WRITE)) {
	// the non-blocking connect has completed (good/bad)

	SignalRemoveSocket(fd);
	// no longer select on write

	try {
	  sock->FinishNonBlockingConnect();
	} catch(SocketException e) {
	  // signal connection failure
	  ostringstream ostr;
	  ostr << "Failed on non-blocking connect for direct connection: " << e.what();
	  SignalLog(LogEvent::ERROR, ostr.str());
	  DisconnectDirectConn( fd );
	  return;
	}

	SignalAddSocket(fd, SocketEvent::READ);
	// select on read now
	
	dc->FinishNonBlockingConnect();

      } else if (sock->getState() == TCPSocket::CONNECTED && (m & SocketEvent::READ)) { 
	try {
	  dc->Recv();
	} catch(DisconnectedException e) {
	  // tear down connection
	  SignalLog(LogEvent::WARN, e.what());
	  DisconnectDirectConn( fd );
	}
      } else {
	SignalLog(LogEvent::ERROR, "Direct Connection socket in inconsistent state!");
	DisconnectDirectConn( fd );
      }
      
    }
  }

  void Client::Connect() {
    if (m_state == NOT_CONNECTED)
      ConnectAuthorizer(AUTH_AWAITING_CONN_ACK);
  }

  void Client::RegisterUIN() {
    if (m_state == NOT_CONNECTED)
      ConnectAuthorizer(UIN_AWAITING_CONN_ACK);
  }

  bool Client::isConnected() const {
    return (m_state == BOS_LOGGED_IN);
  }

  void Client::HandleUserInfoSNAC(UserInfoSNAC *snac) {
    // this should only be personal info
    const UserInfoBlock &ub = snac->getUserInfo();
    if (ub.getUIN() == m_uin) {
      // currently only interested in our external IP
      // - we might be behind NAT
      if (ub.getExtIP() != 0) m_ext_ip = ub.getExtIP();

      // Check for status change
      Status newstat = MapICQStatusToStatus( ub.getStatus() );
      if( m_status != newstat ) {
        m_status = newstat;
        MyStatusChangeEvent ev(m_status);
        statuschanged.emit( &ev );
      }
    }
  }

  void Client::SendViaServer(MessageEvent *ev) {
    Contact *c = ev->getContact();

    if (m_status == STATUS_OFFLINE) {
      ev->setFinished(true);
      ev->setDelivered(false);
      ev->setDirect(false);
      messageack.emit(ev);
      delete ev;
      return;
    }

    if (ev->getType() == MessageEvent::Normal
	|| ev->getType() == MessageEvent::URL) {

      /*
       * Normal messages and URL messages sent via the server
       * can be sent as advanced for ICQ2000 users online, in which
       * case they can be ACKed, otherwise there is no way of
       * knowing if it's received
       */


      ICQSubType *ist;
      if (ev->getType() == MessageEvent::Normal) {
	NormalMessageEvent *nv = static_cast<NormalMessageEvent*>(ev);
	ist = new NormalICQSubType(nv->getMessage(), c->getUIN(), c->acceptAdvancedMsgs());
      } else if (ev->getType() == MessageEvent::URL) {
	URLMessageEvent *uv = static_cast<URLMessageEvent*>(ev);
	ist = new URLICQSubType(uv->getMessage(), uv->getURL(), m_uin, c->getUIN(), c->acceptAdvancedMsgs());
      }

      MsgSendSNAC msnac(ist);

      if (c->acceptAdvancedMsgs()) {
	msnac.setAdvanced(true);
	msnac.setSeqNum( c->nextSeqNum() );
	ICBMCookie ck = m_cookiecache.generateUnique();
	msnac.setICBMCookie( ck );
	m_cookiecache.insert( ck, ev );
      } else { 
	ev->setFinished(true);
	ev->setDelivered(true);
	ev->setDirect(false);
	messageack.emit(ev);
	
	delete ev;
      }
      
      Buffer b(&m_translator);
      unsigned int d;
      d = FLAPHeader(b,0x02);
      b << msnac;
      FLAPFooter(b,d);
      Send(b);
      delete ist;

    } else if (ev->getType() == MessageEvent::AwayMessage) {

      /*
       * Away message requests send via the server only
       * work for ICQ2000 clients online, otherwise there
       * is no way of getting the away message, so we signal the ack
       * to the client as non-delivered
       */

      if (c->acceptAdvancedMsgs()) {
	ICQSubType *ist = new AwayMsgSubType( c->getStatus(), c->getUIN() );
	MsgSendSNAC msnac(ist, true);
	msnac.setSeqNum( c->nextSeqNum() );
	ICBMCookie ck = m_cookiecache.generateUnique();
	msnac.setICBMCookie( ck );
	m_cookiecache.insert( ck, ev );
	Buffer b(&m_translator);
	unsigned int d;
	d = FLAPHeader(b,0x02);
	b << msnac;
	FLAPFooter(b,d);
	Send(b);

	delete ist;

      } else {
	ev->setFinished(true);
	ev->setDelivered(false);
	messageack.emit(ev);

	delete ev;
	return;
      }

    } else if (ev->getType() == MessageEvent::AuthReq) {
      AuthReqEvent *uv = static_cast<AuthReqEvent*>(ev);
      AuthReqICQSubType uist(uv->getMessage(), m_uin, c->getUIN(),
                             c->acceptAdvancedMsgs());

      Buffer b(&m_translator);
      unsigned int d;
      d = FLAPHeader(b,0x02);
      MsgSendSNAC msnac(&uist);
      b << msnac;
      FLAPFooter(b,d);
      Send(b);

    } else if (ev->getType() == MessageEvent::AuthAck) {
      AuthAckEvent *uv = static_cast<AuthAckEvent*>(ev);
      ICQSubType *uist;
      if(uv->isGranted())
        uist=new AuthAccICQSubType(m_uin, c->getUIN(), 
                                        c->acceptAdvancedMsgs());
      else
        uist=new AuthRejICQSubType(uv->getMessage(), m_uin, c->getUIN(), 
                                        c->acceptAdvancedMsgs());
      
      Buffer b(&m_translator);
      unsigned int d;
      d = FLAPHeader(b,0x02);
      MsgSendSNAC msnac(uist);
      b << msnac;
      FLAPFooter(b,d);
      Send(b);

      delete ev;

    } else if (ev->getType() == MessageEvent::SMS) {
      SMSMessageEvent *sv = static_cast<SMSMessageEvent*>(ev);
      SrvSendSNAC ssnac(sv->getMessage(), c->getMobileNo(), m_uin, "", sv->getRcpt());

      unsigned int reqid = NextRequestID();
      m_reqidcache.insert( reqid, new SMSEventCacheValue( sv ) );
      ssnac.setRequestID( reqid );

      Buffer b(&m_translator);
      unsigned int d;
      d = FLAPHeader(b,0x02);
      b << ssnac;
      FLAPFooter(b,d);
      Send(b);
    }

  }

  DirectClient* Client::ConnectDirect(Contact *c) {
    DirectClient *dc;
    if (m_uinmap.count(c->getUIN()) == 0) {
      /*
       * If their external IP != internal IP then it's
       * only worth trying if their external IP == my external IP
       * (when we are behind the same masq box)
       */
      if ( c->getExtIP() != c->getLanIP() && m_ext_ip != c->getExtIP() ) return NULL;
      if ( c->getLanIP() == 0 ) return NULL;
      SignalLog(LogEvent::INFO, "Establishing direct connection");
      dc = new DirectClient(c, m_uin, m_ext_ip, m_listenServer.getPort(), &m_translator);
      dc->logger.connect( slot(this, &Client::dc_log_cb) );
      dc->messaged.connect( slot(this, &Client::dc_messaged_cb) );
      dc->messageack.connect( slot(this, &Client::dc_messageack_cb) );
      dc->connected.connect( SigC::bind<DirectClient*>( slot(this, &Client::dc_connected_cb), dc ) );
      dc->socket.connect( slot(this, &Client::dc_socket_cb) );

      try {
	dc->Connect();
      } catch(DisconnectedException e) {
	SignalLog(LogEvent::WARN, e.what());
	delete dc;
	return NULL;
      } catch(SocketException e) {
	SignalLog(LogEvent::WARN, e.what());
	delete dc;
	return NULL;
      } catch(...) {
	SignalLog(LogEvent::WARN, "Uncaught exception");
	return NULL;
      }

      m_dccache[ dc->getfd() ] = dc;
      m_uinmap[c->getUIN()] = dc;

    } else {
      dc = m_uinmap[c->getUIN()];
    }
    return dc;
  }

  bool Client::SendDirect(MessageEvent *ev) {
    Contact *c = ev->getContact();
    if (!c->getDirect()) return false;
    DirectClient *dc = ConnectDirect(c);
    if (dc == NULL) return false;
    dc->SendEvent(ev);
    return true;
  }

  void Client::SendEvent(MessageEvent *ev) {
    Contact *c = ev->getContact();
    if (ev->getType() == MessageEvent::Normal
	|| ev->getType() == MessageEvent::URL
	|| ev->getType() == MessageEvent::AwayMessage) {
      if (!SendDirect(ev)) SendViaServer(ev);
    } else {
      SendViaServer(ev);
    }
  }
  
  void Client::PingServer() {
    Buffer b(&m_translator);
    unsigned int d;
    d = FLAPHeader(b,0x05);
    FLAPFooter(b,d);
    Send(b);
  }

  void Client::setStatus(const Status st) {
    if (m_state == BOS_LOGGED_IN) {
      if (st == STATUS_OFFLINE) {
	Disconnect(DisconnectedEvent::REQUESTED);
	return;
      }

      Buffer b(&m_translator);
      unsigned int d;
      
      d = FLAPHeader(b,0x02);
      SetStatusSNAC sss(MapStatusToICQStatus(st, m_invisible));
      b << sss;
      FLAPFooter(b,d);
      
      Send(b);
    } else {
      // We'll set this as the initial status upon Connect()
      m_status = st;
      if (st != STATUS_OFFLINE) Connect();
      if (m_state != NOT_CONNECTED && st == STATUS_OFFLINE) Disconnect(DisconnectedEvent::REQUESTED);
    }
  }

  Status Client::getStatus() const {
    return m_status;
  }

  void Client::addContact(Contact& c) {

    if (!m_contact_list.exists(c.getUIN())) {

      Contact& m_contact = m_contact_list.add(c);
      SignalUserAdded(&m_contact);

      if (m_contact.isICQContact() && m_state == BOS_LOGGED_IN) {
	Buffer b(&m_translator);
	unsigned int d;
	d = FLAPHeader(b,0x02);
	AddBuddySNAC abs(m_contact);
	b << abs;
	FLAPFooter(b,d);

	Send(b);

	// fetch detailed userinfo from server
	fetchDetailContactInfo(&m_contact);
      }
    }

  }

  void Client::removeContact(const unsigned int uin) {
    if (m_contact_list.exists(uin)) {
      Contact &c = m_contact_list[uin];
      SignalUserRemoved(&c);
      if (m_contact_list[uin].isICQContact() && m_state == BOS_LOGGED_IN) {
	Buffer b(&m_translator);
	unsigned int d;
	d = FLAPHeader(b,0x02);
	RemoveBuddySNAC rbs(Contact::UINtoString(uin));
	b << rbs;
	FLAPFooter(b,d);
	
	Send(b);
      }

      // remove all direct connections for that contact
      m_dccache.removeContact(&c);
      
      // remove all pending messages for that contact
      m_cookiecache.removeContact(&c);

      // remove all pending request ids for that contact
      m_reqidcache.removeContact(&c);

      m_contact_list.remove(uin);
    }
  }
  
  Contact* Client::lookupICQ(unsigned int uin) {
    if (!m_contact_list.exists(uin)) {
      Contact c(uin);
      addContact(c);
    }
    return &(m_contact_list[uin]);
  }

  Contact* Client::lookupMobile(const string& m) {
    if (!m_contact_list.exists(m)) {
      Contact c(m,m);
      addContact(c);
    }
    return &(m_contact_list[m]);
  }

  

  void Client::SignalUserAdded(Contact *c) {
    UserAddedEvent ev(c);
    contactlist.emit(&ev);
  }

  void Client::SignalUserRemoved(Contact *c) {
    UserRemovedEvent ev(c);
    contactlist.emit(&ev);
  }

  void Client::SignalUserInfoChange(Contact *c) {
    UserInfoChangeEvent ev(c);
    contactlist.emit(&ev);
  }

  void Client::SignalMessageQueueChanged(Contact *c) {
    MessageQueueChangedEvent ev(c);
    contactlist.emit(&ev);
  }

  Contact* Client::getContact(const unsigned int uin) {
    if (m_contact_list.exists(uin)) {
      return &m_contact_list[uin];
    } else {
      return NULL;
    }
  }

  void Client::fetchSimpleContactInfo(Contact *c) {
    Buffer b(&m_translator);
    unsigned int d;

    if ( !c->isICQContact() ) return;

    d = FLAPHeader(b,0x02);
    SrvRequestSimpleUserInfo ssnac( m_uin, c->getUIN() );
    b << ssnac;
    FLAPFooter(b,d);

    Send(b);
  }

  void Client::fetchDetailContactInfo(Contact *c) {
    Buffer b(&m_translator);
    unsigned int d;

    if ( !c->isICQContact() ) return;

    d = FLAPHeader(b,0x02);
    unsigned int reqid = NextRequestID();
    m_reqidcache.insert( reqid, new UserInfoCacheValue(c) );
    SrvRequestDetailUserInfo ssnac( m_uin, c->getUIN() );
    ssnac.setRequestID( reqid );
    b << ssnac;
    FLAPFooter(b,d);

    Send(b);
  }

  void Client::Disconnect(DisconnectedEvent::Reason r) {
    if (m_state == NOT_CONNECTED) return;

    SignalLog(LogEvent::INFO, "Client disconnecting");

    SignalDisconnect(r);
    DisconnectInt();
  }

  void Client::DisconnectInt() {
    if (m_state == NOT_CONNECTED) return;

    if (m_state == AUTH_AWAITING_CONN_ACK || m_state == AUTH_AWAITING_AUTH_REPLY|| 
        m_state == UIN_AWAITING_CONN_ACK || m_state == UIN_AWAITING_UIN_REPLY) {
      DisconnectAuthorizer();
    } else {
      DisconnectBOS();
    }
  }

  unsigned short Client::MapStatusToICQStatus(Status st, bool inv) {
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
    }

    if (inv) s |= STATUS_FLAG_INVISIBLE;
    return s;
  }

  Status Client::MapICQStatusToStatus(unsigned short st) {
    if (st & STATUS_FLAG_DND) return STATUS_DND;
    else if (st & STATUS_FLAG_NA) return STATUS_NA;
    else if (st & STATUS_FLAG_OCCUPIED) return STATUS_OCCUPIED;
    else if (st & STATUS_FLAG_FREEFORCHAT) return STATUS_FREEFORCHAT;
    else if (st & STATUS_FLAG_AWAY) return STATUS_AWAY;
    else return STATUS_ONLINE;
  }

  bool Client::MapICQStatusToInvisible(unsigned short st) {
    return (st & STATUS_FLAG_INVISIBLE);
  }

  bool Client::setTranslationMap(const string& szMapFileName) { 
    try{
      m_translator.setTranslationMap(szMapFileName);
    } catch (TranslatorException e) {
      SignalLog(LogEvent::WARN, e.what());
      return false; 
    }
    return true;
  }

}

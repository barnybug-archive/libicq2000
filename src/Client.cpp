/*
 * libICQ2000 Client
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

#include <libicq2000/TLV.h>
#include <libicq2000/UserInfoBlock.h>

#include <libicq2000/Client.h>
#include <libicq2000/SNAC.h>

#include "sstream_fix.h"

#include <sigc++/bind.h>

using std::ostringstream;
using std::endl;

namespace ICQ2000 {

  /**
   *  Constructor for creating the Client object.  Use this when
   *  uin/password are unavailable at time of creation, they can
   *  always be set later.
   */
  Client::Client() : m_recv(&m_translator){
    Init();
  }

  /**
   *  Constructor for creating the Client object.  Use this when the
   *  uin/password are available at time of creation, to save having
   *  to set them later.
   *
   *  @param uin the owner's uin
   *  @param password the owner's password
   */
  Client::Client(const unsigned int uin, const string& password) : m_uin(uin), m_password(password), m_recv(&m_translator) {
    Init();
  }

  /**
   *  Destructor for the Client object.  This will free up all
   *  resources used by Client, including any Contact objects. It also
   *  automatically disconnects if you haven't done so already.
   */
  Client::~Client() {
    if (m_cookie_data)
      delete [] m_cookie_data;
    Disconnect(DisconnectedEvent::REQUESTED);
  }

  void Client::Init() {
    m_authorizerHostname = "login.icq.com";
    m_authorizerPort = 5190;
    m_bosOverridePort = false;

    m_in_dc = true;
    m_out_dc = true;
    
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

    m_reqidcache.expired.connect( slot(this, &Client::reqidcache_expired_cb) );
    
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
    }
    DisconnectDirectConns();
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

    if (m_status != STATUS_OFFLINE) {
      m_status = STATUS_OFFLINE;
      MyStatusChangeEvent ev(m_status);
      statuschanged.emit( &ev );
    }

    // ensure all contacts return to Offline
    ContactList::iterator curr = m_contact_list.begin();
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

    unsigned short type = st->getType();
    if (type == MSG_Type_Normal) {
      NormalICQSubType *nst = static_cast<NormalICQSubType*>(st);
      
      contact = lookupICQ( nst->getSource() );
      e = new NormalMessageEvent(contact,
				 nst->getMessage(), nst->isMultiParty() );
      
      if (nst->isAdvanced()) SendAdvancedACK(snac);

    } else if (type == MSG_Type_URL) {
      URLICQSubType *ust = static_cast<URLICQSubType*>(st);
      
      contact = lookupICQ( ust->getSource() );
      e = new URLMessageEvent(contact,
			      ust->getMessage(),
			      ust->getURL());

      if (ust->isAdvanced()) SendAdvancedACK(snac);

    } else if (type == MSG_Type_SMS) {
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

    } else if (type == MSG_Type_AuthReq) {
      AuthReqICQSubType *ust = static_cast<AuthReqICQSubType*>(st);
      
      contact = lookupICQ( ust->getSource() );
      e = new AuthReqEvent(contact,ust->getNick(),ust->getFirstName(),
			       ust->getLastName(),ust->getEmail(),
                               ust->getMessage());

    } else if (type == MSG_Type_AuthRej) {
      AuthRejICQSubType *ust = static_cast<AuthRejICQSubType*>(st);
    
      contact = lookupICQ( ust->getSource() );
      e = new AuthAckEvent(contact, ust->getMessage(), false);

    } else if (type == MSG_Type_AuthAcc) {
      AuthAccICQSubType *ust = static_cast<AuthAccICQSubType*>(st);
    
      contact = lookupICQ( ust->getSource() );
      e = new AuthAckEvent(contact, true);

    } else if (type == MSG_Type_AutoReq_Away
	       || type == MSG_Type_AutoReq_Occ
	       || type == MSG_Type_AutoReq_NA
	       || type == MSG_Type_AutoReq_DND
	       || type == MSG_Type_AutoReq_FFC)
    {
      AwayMsgSubType *ast = static_cast<AwayMsgSubType*>(st);
      contact = lookupICQ(ast->getSource());
      AwayMessageEvent aev(contact);
      want_auto_resp.emit(&aev);
      ast->setMessage(aev.getMessage());

      ostringstream dude;
      dude << contact->getAlias() << " (" << contact->getStringUIN() << ")";
      if (ast->isAdvanced()) {
	ostringstream ostr;
	// Has to be dude.str(), the ostringstream itself will not <<... weird
	ostr << "Sending auto response through server to " << dude.str();
	SignalLog(LogEvent::INFO, ostr.str());
	SendAdvancedACK(snac);
      }
      else {
	ostringstream ostr;
	ostr << "Got malformed away message request from " << dude.str();
	SignalLog(LogEvent::WARN, ostr.str());
      }
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
    UINICQSubType *st = snac->getICQSubType();
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

  void Client::SignalMessageOfflineUser(MessageOfflineUserSNAC *snac) {
    /**
     *  Mmm.. it'd be nice to use this as an ack for messages but it's
     *  not consistently sent for all messages through the server
     *  doesn't seem to be sent for normal (non-advanced) messages to
     *  online users.
     */
    ICBMCookie c = snac->getICBMCookie();

    if ( m_cookiecache.exists( c ) ) {
      MessageEvent *ev = m_cookiecache[c];
      ev->setFinished(false);
      ev->setDelivered(false);
      ev->setDirect(false);
      messageack.emit(ev);

    } else {
      SignalLog(LogEvent::WARN, "Received Offline ACK for unknown message");
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

      if ( m_reqidcache.exists( snac->RequestID() ) ) {
	RequestIDCacheValue *v = m_reqidcache[ snac->RequestID() ];

	if ( v->getType() == RequestIDCacheValue::Search ) {
	  SearchCacheValue *sv = static_cast<SearchCacheValue*>(v);

	  SearchResultEvent *ev = sv->getEvent();
	  if (snac->isEmptyContact()) {
	    ev->setLastContactAdded( NULL );
	  } else {
	    Contact c( snac->getUIN() );
	    c.setAlias(snac->getAlias());
	    c.setFirstName(snac->getFirstName());
	    c.setLastName(snac->getLastName());
	    c.setEmail(snac->getEmail());
	    c.setStatus(snac->getStatus());
	    c.setAuthReq(snac->getAuthReq());
	    ContactList& cl = ev->getContactList();
	    ev->setLastContactAdded( &(cl.add(c)) );

	    if (snac->isLastInSearch())
	      ev->setNumberMoreResults( snac->getNumberMoreResults() );
	      
	  }

	  if (snac->isLastInSearch()) ev->setFinished(true);

	  search_result.emit(ev);

	  if (ev->isFinished()) {
	    delete ev;
	    m_reqidcache.remove( snac->RequestID() );
	  }
	  
	} else {
	  SignalLog(LogEvent::WARN, "Request ID cached value is not for a Search request");
	}
	
      } else {
	if ( m_contact_list.exists( snac->getUIN() ) ) {
	  // update Contact
	  Contact& c = m_contact_list[ snac->getUIN() ];
	  c.setAlias( snac->getAlias() );
	  c.setEmail( snac->getEmail() );
	  c.setFirstName( snac->getFirstName() );
	  c.setLastName( snac->getLastName() );
	  UserInfoChangeEvent ev(&c);
	}
      }
      
    } else if (snac->getType() == SrvResponseSNAC::SearchSimpleUserInfo) {

      if ( m_reqidcache.exists( snac->RequestID() ) ) {
	RequestIDCacheValue *v = m_reqidcache[ snac->RequestID() ];

	if ( v->getType() == RequestIDCacheValue::Search ) {
	  SearchCacheValue *sv = static_cast<SearchCacheValue*>(v);

	  SearchResultEvent *ev = sv->getEvent();
	  if (snac->isEmptyContact()) {
	    ev->setLastContactAdded( NULL );
	  } else {
	    Contact c( snac->getUIN() );
	    c.setAlias(snac->getAlias());
	    c.setFirstName(snac->getFirstName());
	    c.setLastName(snac->getLastName());
	    c.setEmail(snac->getEmail());
	    c.setStatus(snac->getStatus());
	    c.setAuthReq(snac->getAuthReq());
	    ContactList& cl = ev->getContactList();
	    ev->setLastContactAdded( &(cl.add(c)) );

	    if (snac->isLastInSearch())
	      ev->setNumberMoreResults( snac->getNumberMoreResults() );
	      
	  }

	  if (snac->isLastInSearch()) ev->setFinished(true);

	  search_result.emit(ev);

	  if (ev->isFinished()) {
	    delete ev;
	    m_reqidcache.remove( snac->RequestID() );
	  }
	  
	} else {
	  SignalLog(LogEvent::WARN, "Request ID cached value is not for a Search request");
	}
	
      } else {
	SignalLog(LogEvent::WARN, "Received a Search Result for unknown request id");
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

    } else if (snac->getType() == SrvResponseSNAC::RWorkInfo) {

      try {
	Contact* c = getUserInfoCacheContact( snac->RequestID() );
	c->setWorkInfo( snac->getWorkInfo() );
	UserInfoChangeEvent ev(c);
	contactlist.emit(&ev);
      } catch(ParseException e) {
	SignalLog(LogEvent::WARN, e.what());
      }

    } else if (snac->getType() == SrvResponseSNAC::RBackgroundInfo) {

      try {
	Contact* c = getUserInfoCacheContact( snac->RequestID() );
	c->setBackgroundInfo( snac->getBackgroundInfo() );
	UserInfoChangeEvent ev(c);
	contactlist.emit(&ev);
      } catch(ParseException e) {
	SignalLog(LogEvent::WARN, e.what());
      }

    } else if (snac->getType() == SrvResponseSNAC::RInterestInfo) {

      try {
	Contact* c = getUserInfoCacheContact( snac->RequestID() );
	c->setInterestInfo( snac->getPersonalInterestInfo() );
	UserInfoChangeEvent ev(c);
	contactlist.emit(&ev);
      } catch(ParseException e) {
	SignalLog(LogEvent::WARN, e.what());
      }

    } else if (snac->getType() == SrvResponseSNAC::REmailInfo) {

      try {
	Contact* c = getUserInfoCacheContact( snac->RequestID() );
	c->setEmailInfo( snac->getEmailInfo() );
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

      if ( v->getType() == RequestIDCacheValue::UserInfo ) {
	UserInfoCacheValue *uv = static_cast<UserInfoCacheValue*>(v);
	return uv->getContact();
      }
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
    SignalLog(LogEvent::WARN, "Message timeout without receiving ACK, sending offline");
    SendViaServerNormal(ev);
  }

  void Client::reqidcache_expired_cb(RequestIDCacheValue* v) 
  {
    if ( v->getType() == RequestIDCacheValue::Search ) {
      SearchCacheValue *sv = static_cast<SearchCacheValue*>(v);

      SearchResultEvent *ev = sv->getEvent();
      ev->setLastContactAdded( NULL );
      ev->setExpired(true);
      ev->setFinished(true);
      search_result.emit(ev);
      delete ev;
	  
    }
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
      if (userinfo.getAcceptAdvMsgs() != UserInfoBlock::tri_unknown) {
	c.setAcceptAdvMsgs( userinfo.getAcceptAdvMsgs() == UserInfoBlock::tri_true );
      }
      
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

  Buffer::marker Client::FLAPHeader(Buffer& b, unsigned char channel) 
  {
    b.setBigEndian();
    b << (unsigned char) 42;
    b << channel;
    b << NextSeqNum();
    Buffer::marker mk = b.getAutoSizeShortMarker();
    return mk;
  }
  
  void Client::FLAPFooter(Buffer& b, Buffer::marker& mk) 
  {
    b.setAutoSizeMarker(mk);
  }
  

  void Client::FLAPwrapSNAC(Buffer& b, const OutSNAC& snac)
  {
    Buffer::marker mk = FLAPHeader(b, 0x02);
    b << snac;
    FLAPFooter(b,mk);
  }

  void Client::FLAPwrapSNACandSend(const OutSNAC& snac)
  {
    Buffer b(&m_translator);
    FLAPwrapSNAC(b, snac);
    Send(b);
  }
  
  void Client::SendAuthReq() {
    Buffer b(&m_translator);
    Buffer::marker mk = FLAPHeader(b,0x01);

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

    FLAPFooter(b,mk);
    SignalLog(LogEvent::INFO, "Sending Authorisation Request");
    Send(b);
  }

  void Client::SendNewUINReq() {
    Buffer b(&m_translator);
    Buffer::marker mk;

    mk = FLAPHeader(b,0x01);
    b << (unsigned int)0x00000001;
    FLAPFooter(b,mk);
    Send(b);

    SignalLog(LogEvent::INFO, "Sending New UIN Request");
    FLAPwrapSNACandSend( UINRequestSNAC(m_password) );
  }
    
  void Client::SendCookie() {
    Buffer b(&m_translator);
    Buffer::marker mk = FLAPHeader(b,0x01);

    b << (unsigned int)0x00000001;

    b << CookieTLV(m_cookie_data, m_cookie_length);

    FLAPFooter(b,mk);
    SignalLog(LogEvent::INFO, "Sending Login Cookie");
    Send(b);
  }
    
  void Client::SendCapabilities() {
    SignalLog(LogEvent::INFO, "Sending Capabilities");
    FLAPwrapSNACandSend( CapabilitiesSNAC() );
  }

  void Client::SendSetUserInfo() {
    SignalLog(LogEvent::INFO, "Sending Set User Info");
    FLAPwrapSNACandSend( SetUserInfoSNAC() );
  }

  void Client::SendRateInfoRequest() {
    SignalLog(LogEvent::INFO, "Sending Rate Info Request");
    FLAPwrapSNACandSend( RequestRateInfoSNAC() );
  }
  
  void Client::SendRateInfoAck() {
    SignalLog(LogEvent::INFO, "Sending Rate Info Ack");
    FLAPwrapSNACandSend( RateInfoAckSNAC() );
  }

  void Client::SendPersonalInfoRequest() {
    SignalLog(LogEvent::INFO, "Sending Personal Info Request");
    FLAPwrapSNACandSend( PersonalInfoRequestSNAC() );
  }

  void Client::SendAddICBMParameter() {
    SignalLog(LogEvent::INFO, "Sending Add ICBM Parameter");
    FLAPwrapSNACandSend( MsgAddICBMParameterSNAC() );
  }

  void Client::SendLogin() {
    Buffer b(&m_translator);

    // startup listening server at this point, so we
    // know the listening port and ip
    if (m_in_dc) {
      m_listenServer.StartServer();
      SignalAddSocket( m_listenServer.getSocketHandle(), SocketEvent::READ );
      ostringstream ostr;
      ostr << "Server listening on " << IPtoString( m_serverSocket.getLocalIP() ) << ":" << m_listenServer.getPort();
      SignalLog(LogEvent::INFO, ostr.str());
    } else {
      SignalLog(LogEvent::INFO, "Not starting listening server, incoming Direct connections disabled");
    }

    if (!m_contact_list.empty())
      FLAPwrapSNAC(b, AddBuddySNAC(m_contact_list) );

    if (m_invisible)
      FLAPwrapSNAC(b, AddVisibleSNAC() );
        
    SetStatusSNAC sss(MapStatusToICQStatus(m_status, m_invisible));

    // explicitly set status to offline. If the user set the status
    // before calling Connect and we don't do this, we'll miss the
    // status change upon the user info reception and will not emit
    // the statuschanged signal correctly
    m_status = STATUS_OFFLINE;

    sss.setSendExtra(true);
    sss.setIP( m_serverSocket.getLocalIP() );
    sss.setPort( (m_in_dc ? m_listenServer.getPort() : 0) );
    FLAPwrapSNAC( b, sss );

    FLAPwrapSNAC( b, ClientReadySNAC() );

    FLAPwrapSNAC( b, SrvRequestOfflineSNAC(m_uin) );

    SignalLog(LogEvent::INFO, "Sending Contact List, Status, Client Ready and Offline Messages Request");
    Send(b);

    SignalConnect();
    m_last_server_ping = time(NULL);
  }

  void Client::SendOfflineMessagesRequest() {
    SignalLog(LogEvent::INFO, "Sending Offline Messages Request");
    FLAPwrapSNACandSend( SrvRequestOfflineSNAC(m_uin) );
  }


  void Client::SendOfflineMessagesACK() {
    SignalLog(LogEvent::INFO, "Sending Offline Messages ACK");
    FLAPwrapSNACandSend( SrvAckOfflineSNAC(m_uin) );
  }

  void Client::SendAdvancedACK(MessageSNAC *snac) {
    ICQSubType *st = snac->getICQSubType();
    if (st == NULL || dynamic_cast<UINICQSubType*>(st) == NULL ) return;
    UINICQSubType *ust = dynamic_cast<UINICQSubType*>(snac->grabICQSubType());

    SignalLog(LogEvent::INFO, "Sending Advanced Message ACK");
    FLAPwrapSNACandSend( MessageACKSNAC( snac->getICBMCookie(), ust ) );
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
      case SNAC_MSG_OfflineUser:
	SignalLog(LogEvent::INFO, "Received Message to Offline User from server");
	SignalMessageOfflineUser(static_cast<MessageOfflineUserSNAC*>(snac));
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
    case SNAC_FAM_SBL:
      switch(snac->Subtype()) {
      case SNAC_SBL_List_From_Server:
	SignalLog(LogEvent::INFO, "Received server-based list from server\n");
        SBLListSNAC *sbs = static_cast<SBLListSNAC*>(snac);
        ContactList l = sbs->getContactList();
        ContactList::iterator curr = l.begin();
        while (curr != l.end()) {
            if ((*curr).isICQContact()) 
                SignalServerBasedContact(&(*curr));
            ++curr;
            }
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

  /**
   *  Perform any regular time dependant tasks.
   *
   *  Poll must be called regularly (at least every 60 seconds) but I
   *  recommended 5 seconds, so timeouts work with good granularity.
   *  It is not related to the socket callback and socket listening.
   *  The client must call this Poll fairly regularly, to ensure that
   *  timeouts on message sending works correctly, and that the server
   *  is pinged once every 60 seconds.
   */
  void Client::Poll() {
    time_t now = time(NULL);
    if (now > m_last_server_ping + 60) {
      PingServer();
      m_last_server_ping = now;

    }

    m_reqidcache.clearoutPoll();
    m_cookiecache.clearoutPoll();
    m_dccache.clearoutPoll();
    m_dccache.clearoutMessagesPoll();
  }

  /**
   *  Callback from client to tell library the socket is ready.  The
   *  client must call this method when select says that the file
   *  descriptor is available for mode (read, write or exception).
   *  The client will know what socket descriptors to select on, and
   *  with what mode from the SocketEvent's it receives. It is worth
   *  looking at the shell.cpp example in the examples directory and
   *  fully understanding how it works with select.
   *
   * @param fd the socket descriptor
   * @param m the mode that the socket is available on
   */
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
      
    } else if ( m_in_dc && fd == m_listenServer.getSocketHandle() ) {
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
      dc->want_auto_resp.connect( want_auto_resp.slot() );
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

  /**
   *  Register a new UIN. Although support for this has been coded
   *  there isn't any support for settings your details yet, so being
   *  able to create a new user is largely useless.
   *
   * @todo settings your user information
   */
  void Client::RegisterUIN() {
    if (m_state == NOT_CONNECTED)
      ConnectAuthorizer(UIN_AWAITING_CONN_ACK);
  }

  /**
   *  Boolean to determine if you are connected or not.
   *
   * @return connected
   */
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
      bool newinvis = MapICQStatusToInvisible( ub.getStatus() );
      if( m_status != newstat  ||  m_invisible != newinvis ) {
        m_status = newstat;
        m_invisible = newinvis;
        MyStatusChangeEvent ev(m_status, m_invisible);
        statuschanged.emit( &ev );
      }
    }
  }

  /**
   *  Used for sending a message event from the client.  The Client
   *  should create the specific MessageEvent by dynamically
   *  allocating it with new. The library will take care of deleting
   *  it when appropriate. The MessageEvent will persist whilst the
   *  message has not be confirmed as delivered or failed yet. Exactly
   *  the same MessageEvent is signalled back in the messageack signal
   *  callback, so a client could use pointer equality comparison to
   *  match messages it has sent up to their acks.
   */
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
  
  bool Client::SendDirect(MessageEvent *ev) {
    Contact *c = ev->getContact();
    if (!c->getDirect()) return false;
    DirectClient *dc = ConnectDirect(c);
    if (dc == NULL) return false;
    dc->SendEvent(ev);
    return true;
  }

  DirectClient* Client::ConnectDirect(Contact *c) {
    DirectClient *dc;
    if (m_uinmap.count(c->getUIN()) == 0) {
      if (!m_out_dc) return NULL;
      /*
       * If their external IP != internal IP then it's
       * only worth trying if their external IP == my external IP
       * (when we are behind the same masq box)
       */
      if ( c->getExtIP() != c->getLanIP() && m_ext_ip != c->getExtIP() ) return NULL;
      if ( c->getLanIP() == 0 ) return NULL;
      SignalLog(LogEvent::INFO, "Establishing direct connection");
      dc = new DirectClient(c, m_uin, m_ext_ip, (m_in_dc ? m_listenServer.getPort() : 0), &m_translator);
      dc->logger.connect( slot(this, &Client::dc_log_cb) );
      dc->messaged.connect( slot(this, &Client::dc_messaged_cb) );
      dc->messageack.connect( slot(this, &Client::dc_messageack_cb) );
      dc->connected.connect( SigC::bind<DirectClient*>( slot(this, &Client::dc_connected_cb), dc ) );
      dc->socket.connect( slot(this, &Client::dc_socket_cb) );
      dc->want_auto_resp.connect( want_auto_resp.slot() );

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
	|| ev->getType() == MessageEvent::URL
	|| ev->getType() == MessageEvent::AuthReq
	|| ev->getType() == MessageEvent::AuthAck) {
      
      /*
       * Normal messages and URL messages sent via the server
       * can be sent as advanced for ICQ2000 users online, in which
       * case they can be ACKed, otherwise there is no way of
       * knowing if it's received
       */
      
      if (c->acceptAdvancedMsgs())
	SendViaServerAdvanced(ev);
      else {
	SendViaServerNormal(ev);
	delete ev;
      }
      
    } else if (ev->getType() == MessageEvent::AwayMessage) {

      /*
       * Away message requests sent via the server only
       * work for ICQ2000 clients online, otherwise there
       * is no way of getting the away message, so we signal the ack
       * to the client as non-delivered
       */

      if (c->acceptAdvancedMsgs())
	SendViaServerAdvanced(ev);
      else {
	ev->setFinished(true);
	ev->setDelivered(false);
	ev->setDirect(false);
	messageack.emit(ev);
	delete ev;
      }

    } else if (ev->getType() == MessageEvent::SMS) {

      /*
       * SMS Messages are sent via a completely different mechanism.
       *
       */
      SMSMessageEvent *sv = static_cast<SMSMessageEvent*>(ev);
      SrvSendSNAC ssnac(sv->getMessage(), c->getMobileNo(), m_uin, "", sv->getRcpt());

      unsigned int reqid = NextRequestID();
      m_reqidcache.insert( reqid, new SMSEventCacheValue( sv ) );
      ssnac.setRequestID( reqid );

      FLAPwrapSNACandSend( ssnac );

    }

  }

  void Client::SendViaServerAdvanced(MessageEvent *ev) 
  {
    Contact *c = ev->getContact();
    UINICQSubType *ist = EventToUINICQSubType(ev);
    ist->setAdvanced(true);
    
    MsgSendSNAC msnac(ist);
    msnac.setAdvanced(true);
    msnac.setSeqNum( c->nextSeqNum() );
    ICBMCookie ck = m_cookiecache.generateUnique();
    msnac.setICBMCookie( ck );
    m_cookiecache.insert( ck, ev );
    
    FLAPwrapSNACandSend( msnac );
    
    delete ist;
  }
  
  void Client::SendViaServerNormal(MessageEvent *ev)
  {
    Contact *c = ev->getContact();
    UINICQSubType *ist = EventToUINICQSubType(ev);
    ist->setAdvanced(false);
    
    MsgSendSNAC msnac(ist);
    msnac.setAdvanced(false);

    FLAPwrapSNACandSend( msnac );
    
    ev->setFinished(true);
    ev->setDelivered(true);
    ev->setDirect(false);
    messageack.emit(ev);
  }
  
  UINICQSubType* Client::EventToUINICQSubType(MessageEvent *ev)
  {
    Contact *c = ev->getContact();
    UINICQSubType *ist = NULL;

    if (ev->getType() == MessageEvent::Normal) {

      NormalMessageEvent *nv = static_cast<NormalMessageEvent*>(ev);
      ist = new NormalICQSubType(nv->getMessage(), c->getUIN());

    } else if (ev->getType() == MessageEvent::URL) {

      URLMessageEvent *uv = static_cast<URLMessageEvent*>(ev);
      ist = new URLICQSubType(uv->getMessage(), uv->getURL(), m_uin, c->getUIN());

    } else if (ev->getType() == MessageEvent::AwayMessage) {

      ist = new AwayMsgSubType( c->getStatus(), c->getUIN() );

    } else if (ev->getType() == MessageEvent::AuthReq) {

      AuthReqEvent *uv = static_cast<AuthReqEvent*>(ev);
      ist = new AuthReqICQSubType(uv->getMessage(), m_uin, c->getUIN());

    } else if (ev->getType() == MessageEvent::AuthAck) {

      AuthAckEvent *uv = static_cast<AuthAckEvent*>(ev);
      if(uv->isGranted())
        ist = new AuthAccICQSubType(m_uin, c->getUIN());
      else
        ist = new AuthRejICQSubType(uv->getMessage(), m_uin, c->getUIN());

    }
    return ist;
  }

  void Client::PingServer() {
    Buffer b(&m_translator);
    Buffer::marker mk = FLAPHeader(b,0x05);
    FLAPFooter(b,mk);
    Send(b);
  }

  /**
   *  Set your status. This is used to set your status, as well as to
   *  connect and disconnect from the network. When you wish to
   *  connect to the ICQ network, set status to something other than
   *  STATUS_OFFLINE and connecting will be initiated. When you wish
   *  to disconnect set the status to STATUS_OFFLINE and disconnection
   *  will be initiated.
   *
   * @param st the status
   * @param inv whether to be invisible or not
   */
  void Client::setStatus(const Status st, bool inv) {
    if (m_state == BOS_LOGGED_IN) {
      if (st == STATUS_OFFLINE) {
	Disconnect(DisconnectedEvent::REQUESTED);
	return;
      }

      /*
       * The correct sequence of events are:
       * - when going from visible to invisible
       *   - send the Add to Visible list (or better named Set in this case)
       *   - send set Status
       * - when going from invisible to visible
       *   - send set Status
       *   - send the Add to Invisible list (or better named Set in this case)
       */

      Buffer b(&m_translator);

      if (!m_invisible && inv) {
	// visible -> invisible
	FLAPwrapSNAC( b, AddVisibleSNAC() );
      }
	
      FLAPwrapSNAC( b, SetStatusSNAC(MapStatusToICQStatus(st, inv)) );
      
      if (m_invisible && !inv) {
	// invisible -> visible
	FLAPwrapSNAC( b, AddInvisibleSNAC() );
      }
      
      Send(b);

    } else {
      // We'll set this as the initial status upon Connect()
      m_status = st;
      m_invisible = inv;  
      if (st != STATUS_OFFLINE) Connect();
      if (m_state != NOT_CONNECTED && st == STATUS_OFFLINE) Disconnect(DisconnectedEvent::REQUESTED);
    }
  }

  /**
   *  Get your current status.
   *
   * @return your current status
   */
  Status Client::getStatus() const {
    return m_status;
  }

  /**
   *  Get your invisible status
   * @return Invisible boolean
   *
   */
  bool Client::getInvisible() const
  {
    return m_invisible;
  }

  /**
   *  Add a contact to your list.  The contact passed by reference
   *  need only be a temporary. It is copied within the library before
   *  returning.
   *
   * @param c the contact passed by reference.
   */
  void Client::addContact(Contact& c) {

    if (!m_contact_list.exists(c.getUIN())) {

      Contact& m_contact = m_contact_list.add(c);
      SignalUserAdded(&m_contact);

      if (m_contact.isICQContact() && m_state == BOS_LOGGED_IN) {
	FLAPwrapSNACandSend( AddBuddySNAC(m_contact) );

	// fetch detailed userinfo from server
	fetchDetailContactInfo(&m_contact);
      }
    }

  }

  /**
   *  Remove a contact from your list.
   *
   * @param uin the uin of the contact to be removed
   */
  void Client::removeContact(const unsigned int uin) {
    if (m_contact_list.exists(uin)) {
      Contact &c = m_contact_list[uin];
      SignalUserRemoved(&c);
      if (m_contact_list[uin].isICQContact() && m_state == BOS_LOGGED_IN) {
	FLAPwrapSNACandSend( RemoveBuddySNAC(Contact::UINtoString(uin)) );
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

  

  void Client::SignalServerBasedContact(Contact *c) {
    ServerBasedContactEvent ev(c);
    contactlist.emit(&ev);
  }

  void Client::SignalUserAdded(Contact *c) {
    UserAddedEvent ev(c);
    contactlist.emit(&ev);
  }

  void Client::SignalUserRemoved(Contact *c) {
    UserRemovedEvent ev(c);
    contactlist.emit(&ev);
  }

  /**
   *  Method for indicating the User Info for a contact has changed.
   *  The reason this is not automatically signalled when the set
   *  methods are called on Contact are because large numbers of
   *  setting contact properties, such as on startup of a client would
   *  be very inefficient then. It is trusted that a Client will call
   *  this method when it has finished changing properties on a
   *  Contact, just as the library will.

   * @param c the contact
   */
  void Client::SignalUserInfoChange(Contact *c) {
    UserInfoChangeEvent ev(c);
    contactlist.emit(&ev);
  }

  /**
   *  Method for indicating that the queue of message events
   *  automatically stored in a contact has changed.
   *
   * @todo This will probably change so that it isn't necessary to call this anymore.
   */
  void Client::SignalMessageQueueChanged(Contact *c) {
    MessageQueueChangedEvent ev(c);
    contactlist.emit(&ev);
  }

  /**
   *  Get the Contact object for a given uin.
   *
   * @param uin the uin
   * @return a pointer to the Contact object. NULL if no Contact with
   * that uin exists on your list.
   */
  Contact* Client::getContact(const unsigned int uin) {
    if (m_contact_list.exists(uin)) {
      return &m_contact_list[uin];
    } else {
      return NULL;
    }
  }

  /**
   *  Request the simple contact information for a Contact.  This
   *  consists of the contact alias, firstname, lastname and
   *  email. When the server has replied with the details the library
   *  will signal a user info changed for this contact.
   *
   * @param c contact to fetch info for
   * @see ContactListEvent
   */
  void Client::fetchSimpleContactInfo(Contact *c) {
    Buffer b(&m_translator);

    if ( !c->isICQContact() ) return;

    SignalLog(LogEvent::INFO, "Sending request Simple Userinfo Request");
    FLAPwrapSNACandSend( SrvRequestSimpleUserInfo( m_uin, c->getUIN() ) );
  }

  /**
   *  Request the detailed contact information for a Contact. When the
   *  server has replied with the details the library will signal a
   *  user info changed for this contact.
   *
   * @param c contact to fetch info for
   * @see ContactListEvent
   */
  void Client::fetchDetailContactInfo(Contact *c) {
    if ( !c->isICQContact() ) return;

    SignalLog(LogEvent::INFO, "Sending request Detailed Userinfo Request");

    unsigned int reqid = NextRequestID();
    m_reqidcache.insert( reqid, new UserInfoCacheValue(c) );
    SrvRequestDetailUserInfo ssnac( m_uin, c->getUIN() );
    ssnac.setRequestID( reqid );
    FLAPwrapSNACandSend( ssnac );
  }

  void Client::fetchServerBasedContactList() {
    SignalLog(LogEvent::INFO, "Requesting Server-based contact list");
    FLAPwrapSNACandSend( RequestSBLSNAC() );
  }

  SearchResultEvent* Client::searchForContacts
    (const string& nickname, const string& firstname,
     const string& lastname)
  {
    SearchResultEvent *ev = new SearchResultEvent( SearchResultEvent::ShortWhitepage );

    unsigned int reqid = NextRequestID();
    m_reqidcache.insert( reqid, new SearchCacheValue( ev ) );

    SrvRequestShortWP ssnac( m_uin, nickname, firstname, lastname );
    ssnac.setRequestID( reqid );

    SignalLog(LogEvent::INFO, "Sending short whitepage search");
    FLAPwrapSNACandSend( ssnac );

    return ev;
  }

  SearchResultEvent* Client::searchForContacts
    (const string& nickname, const string& firstname,
     const string& lastname, const string& email,
     unsigned short min_age, unsigned short max_age,
     Sex sex, unsigned char language, const string& city,
     const string& state, unsigned short country,
     const string& company_name, const string& department,
     const string& position, bool only_online)
  {
    SearchResultEvent *ev = new SearchResultEvent( SearchResultEvent::FullWhitepage );

    unsigned int reqid = NextRequestID();
    m_reqidcache.insert( reqid, new SearchCacheValue( ev ) );

    SrvRequestFullWP ssnac( m_uin, nickname, firstname, lastname, email,
			    min_age, max_age, (unsigned char)sex, language, city, state,
			    country, company_name, department, position,
			    only_online);
    ssnac.setRequestID( reqid );

    SignalLog(LogEvent::INFO, "Sending full whitepage search");
    FLAPwrapSNACandSend( ssnac );

    return ev;
  }

  SearchResultEvent* Client::searchForContacts(unsigned int uin)
  {
    SearchResultEvent *ev = new SearchResultEvent( SearchResultEvent::UIN );

    unsigned int reqid = NextRequestID();
    m_reqidcache.insert( reqid, new SearchCacheValue( ev ) );

    SrvRequestSimpleUserInfo ssnac( m_uin, uin );
    ssnac.setRequestID( reqid );

    SignalLog(LogEvent::INFO, "Sending simple user info request");
    FLAPwrapSNACandSend( ssnac );

    return ev;
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

  /**
   *  Set the translation map file to use for character set translation.
   * @param szMapFileName the name of the translation map file
   * @return whether setting translation map was a success
   */
  bool Client::setTranslationMap(const string& szMapFileName) { 
    try{
      m_translator.setTranslationMap(szMapFileName);
    } catch (TranslatorException e) {
      SignalLog(LogEvent::WARN, e.what());
      return false; 
    }
    return true;
  }

  /**
   *  Get the File name of the translation map currently in use.
   * @return filename of translation map
   */
  const string& Client::getTranslationMapFileName() const {
    return m_translator.getMapFileName();
  }

  /**
   *  Get the Name of the translation map currently in use.
   * @return name of translation map
   */
  const string& Client::getTranslationMapName() const {
    return m_translator.getMapName();
  }

  /**
   *  Determine whether the default map (no translation) is in use.
   * @return whether using the default map
   */
  bool Client::usingDefaultMap() const {
    return m_translator.usingDefaultMap();
  }

  /**
   *  set the hostname of the login server.
   *  You needn't touch this normally, it will default automatically to login.icq.com.
   *
   * @param host The host name of the server
   */
  void Client::setLoginServerHost(const string& host) {
    m_authorizerHostname = host;
  }

  /**
   *  get the hostname for the currently set login server.
   *
   * @return the hostname
   */
  string Client::getLoginServerHost() const {
    return m_authorizerHostname;
  }
  
  /**
   *  set the port on the login server to connect to
   *
   * @param port the port number
   */
  void Client::setLoginServerPort(const unsigned short& port) {
    m_authorizerPort = port;
  }

  /**
   *  get the currently set port on the login server.
   *
   * @return the port number
   */
  unsigned short Client::getLoginServerPort() const {
    return m_authorizerPort;
  }
  
  /**
   *  set whether to override the port used to connect to the BOS
   *  server.  If you would like to ignore the port that the login
   *  server tells you to connect to on the BOS server and instead use
   *  your own, set this to true and call setBOSServerPort with the
   *  port you would like to use. This method is largely unnecessary,
   *  if you set a different login port - for example to get through
   *  firewalls that block 5190, the login server will accept it fine
   *  and in the redirect message doesn't specify a port, so the
   *  library will default to using the same one as it used to connect
   *  to the login server anyway.
   *
   * @param b override redirect port
   */
  void Client::setBOSServerOverridePort(const bool& b) {
    m_bosOverridePort = b;
  }

  /**
   *  get whether the BOS redirect port will be overridden.
   *
   * @return override redirect port
   */
  bool Client::getBOSServerOverridePort() const {
    return m_bosOverridePort;
  }
  
  /**
   *  set the port to use to connect to the BOS server. This will only
   *  be used if you also called setBOSServerOverridePort(true).
   *
   * @param port the port number
   */
  void Client::setBOSServerPort(const unsigned short& port) {
    m_bosPort = port;
  }

  /**
   *  get the port that will be used on the BOS server.
   *
   * @return the port number
   */
  unsigned short Client::getBOSServerPort() const {
    return m_bosPort;
  }

  /**
   *  set whether to accept incoming direct connections
   *
   * @d whether to accept incoming direct connections
   */
  void Client::setAcceptInDC(bool d) {
    m_in_dc = d;
    if (m_in_dc && m_listenServer.isStarted()) {
      SignalRemoveSocket( m_listenServer.getSocketHandle() );
      m_listenServer.Disconnect();
    }
  }
  
  /**
   *  get whether to accept incoming direct connections
   *
   * @return whether to accept incoming direct connections
   */
  bool Client::getAcceptInDC() const 
  {
    return m_in_dc;
  }
  
  /**
   *  set whether to make outgoing direct connections
   *
   * @d whether to make outgoing direct connections
   */
  void Client::setUseOutDC(bool d) {
    m_out_dc = d;
  }
  
  /**
   *  get whether to make outgoing direct connections
   *
   * @return whether to make outgoing direct connections
   */
  bool Client::getUseOutDC() const 
  {
    return m_out_dc;
  }
  
}

/*
 * DirectClient
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

#include <libicq2000/DirectClient.h>

#include "sstream_fix.h"

#include <stdlib.h>

using std::ostringstream;
using SigC::slot;

namespace ICQ2000 {

  unsigned char DirectClient::client_check_data[] = {
    "As part of this software beta version Mirabilis is "
    "granting a limited access to the ICQ network, "
    "servers, directories, listings, information and databases (\""
    "ICQ Services and Information\"). The "
    "ICQ Service and Information may databases (\""
    "ICQ Services and Information\"). The "
    "ICQ Service and Information may\0"
  };

  /*
   * Constructor when receiving an incoming connection
   */
  DirectClient::DirectClient(TCPSocket *sock, ContactList *cl, unsigned int uin, unsigned int ext_ip, unsigned short server_port, Translator* translator)
    : m_socket(sock), m_state(WAITING_FOR_INIT),
      m_local_uin(uin), m_local_ext_ip(ext_ip),
      m_local_server_port(server_port),m_translator(translator), 
      m_recv(translator), m_contact_list(cl), m_incoming(true), m_contact(NULL)
  {
    Init();
  }

  /*
   * Constructor for making an outgoing connection
   */
  DirectClient::DirectClient(Contact *c, unsigned int uin, unsigned int ext_ip, unsigned short server_port, Translator *translator)
    : m_state(NOT_CONNECTED), m_local_uin(uin), m_local_ext_ip(ext_ip),
      m_local_server_port(server_port), m_translator(translator),
      m_recv(translator), m_incoming(false), m_contact(c)
  {
    Init();
    m_socket = new TCPSocket();
    m_remote_uin = c->getUIN();
  }

  DirectClient::~DirectClient() {
    m_msgcache.expireAll();
    
    while (!m_msgqueue.empty()) {
      expired_cb( m_msgqueue.front() );
      m_msgqueue.pop_front();
    }

    if ( m_socket->getSocketHandle() > -1) SignalRemoveSocket( m_socket->getSocketHandle() );
    delete m_socket;
  }

  void DirectClient::SignalAddSocket(int fd, SocketEvent::Mode m) {
    AddSocketHandleEvent ev( fd, m );
    socket.emit(&ev);
  }

  void DirectClient::SignalRemoveSocket(int fd) {
    RemoveSocketHandleEvent ev(fd);
    socket.emit(&ev);
  }

  void DirectClient::Init() {
    m_seqnum = 0xFFFF;
    m_msgcache.setDefaultTimeout(30);
    m_msgcache.expired.connect( slot(this, &DirectClient::expired_cb) );
  }

  void DirectClient::Connect() {
    m_remote_tcp_version = m_contact->getTCPVersion();
    if (m_remote_tcp_version >= 7) m_eff_tcp_version = 7;
    else if (m_remote_tcp_version == 6) m_eff_tcp_version = 6;
    else throw DisconnectedException("Cannot direct connect to client with too old TCP version");

    m_socket->setRemoteIP( m_contact->getLanIP() );
    m_socket->setRemotePort( m_contact->getLanPort() );
    m_socket->setBlocking(false);
    m_socket->Connect();
    SignalAddSocket( m_socket->getSocketHandle(), SocketEvent::WRITE );

    m_session_id = (unsigned int)(0xffffffff*(rand()/(RAND_MAX+1.0)));

    m_state = WAITING_FOR_INIT_ACK;
  }

  void DirectClient::FinishNonBlockingConnect() {
    SendInitPacket();
  }

  void DirectClient::expired_cb(MessageEvent *ev) {
    if ( m_contact != NULL ) {
      ev->setFinished(false);
      ev->setDelivered(false);
      ev->setDirect(true);
      messageack.emit(ev);
    } else {
      // discard
      delete ev;
    }
  }

  void DirectClient::Recv() {
    try {
      while ( m_socket->connected() ) {
	if ( !m_socket->Recv(m_recv) ) break;
	ostringstream ostr;
	ostr << "Received packet from " << IPtoString( m_socket->getRemoteIP() ) << ":" << m_socket->getRemotePort() << endl << m_recv;
	SignalLog(LogEvent::DIRECTPACKET, ostr.str());
	Parse();
      }
    } catch(SocketException e) {
      ostringstream ostr;
      ostr << "Failed on recv: " << e.what();
      throw DisconnectedException( ostr.str() );
    } catch(ParseException e) {
      ostringstream ostr;
      ostr << "Failed parsing: " << e.what();
      throw DisconnectedException( ostr.str() );
    }
  }

  void DirectClient::SignalLog(LogEvent::LogType type, const string& msg) {
    LogEvent ev(type,msg);
    logger.emit(&ev);
  }

  void DirectClient::SignalMessageEvent(MessageEvent *ev) {
    messaged.emit(ev);
  }

  void DirectClient::Parse() {
    if (m_recv.empty()) return;

    unsigned short length;

    while (!m_recv.empty()) {
      m_recv.setPos(0);

      m_recv.setEndianness(Buffer::LITTLE);
      m_recv >> length;
      if (m_recv.remains() < length) return; // waiting for more of the packet

      Buffer sb(m_translator);
      m_recv.chopOffBuffer( sb, length+2 );

      if (m_state == WAITING_FOR_INIT) {
	ParseInitPacket(sb);

	if (m_incoming) {
	  SendInitAck();
	  SendInitPacket();
	  m_state = WAITING_FOR_INIT_ACK;
	} else {
	  SendInitAck();
	  if (m_eff_tcp_version == 7) {
	    SendInit2();
	    m_state = WAITING_FOR_INIT2;
	  } else {
	    m_state = CONNECTED;
	    flush_queue();
	    connected.emit();
	  }
	}

      } else if (m_state == WAITING_FOR_INIT_ACK) {
	ParseInitAck(sb);

	if (m_incoming) {
	  // Incoming
	  if (m_eff_tcp_version == 7)
	    m_state = WAITING_FOR_INIT2; // v7 has an extra stage of handshaking
	  else {
	    ConfirmUIN();
	    m_state = CONNECTED;          // v5 is done handshaking now
	    flush_queue();
	    connected.emit();
	  }

	} else {
	  // Outgoing - next packet should be their INIT
	  m_state = WAITING_FOR_INIT;
	}

      } else if (m_state == WAITING_FOR_INIT2) {
	ParseInit2(sb);
	// This is a V7 only packet

	if (m_incoming) {
	  SendInit2();
	  ConfirmUIN();
	}

	m_state = CONNECTED;
	flush_queue();
	connected.emit();

      } else if (m_state == CONNECTED) {
	ParsePacket(sb);
      }


      if (sb.beforeEnd()) {
	/* we assert that parsing code eats uses all data
	 * in the FLAP - seems useful to know when they aren't
	 * as it probably means they are faulty
	 */
	ostringstream ostr;
	ostr  << "Buffer pointer not at end after parsing packet was: 0x" << hex << sb.pos()
	      << " should be: 0x" << sb.size();
	SignalLog(LogEvent::WARN, ostr.str());
      }
      
    }
    
  }

  void DirectClient::ConfirmUIN() {
    if ( m_contact_list->exists(m_remote_uin) ) {
      Contact &c = (*m_contact_list)[ m_remote_uin ];
      if ( (c.getExtIP() == m_local_ext_ip && c.getLanIP() == getIP() )
	   /* They are behind the same masquerading box,
	    * and the Lan IP matches
	    */
	       || c.getExtIP() == getIP()) {
	m_contact = &c;
      } else {
	// spoofing attempt most likely
	ostringstream ostr;
	ostr << "Refusing direct connection from someone that claims to be UIN "
	     << m_remote_uin << " since their IP " << IPtoString( getIP() ) << " != " << IPtoString( c.getExtIP() );
	throw DisconnectedException( ostr.str() );
      }
      
    } else {
      // don't accept direct connections from contacts not on contact list
      throw DisconnectedException("Refusing direct connection to contact not on contact list");
    }
  }

  void DirectClient::SendInitPacket() {
    Buffer b(m_translator);
    b.setEndianness(Buffer::LITTLE);
    b << (unsigned short)( m_eff_tcp_version == 7 ? 0x0030 : 0x002c ); // length

    b << (unsigned char)0xff;    // start byte
    b << (unsigned short)0x0007; // tcp version
    b << (unsigned short)( m_eff_tcp_version == 7 ? 0x002b : 0x0027 ); // second length
    
    b << m_remote_uin;
    b << (unsigned short)0x0000;
    b << (unsigned int)m_local_server_port;

    b << m_local_uin;
    b.setEndianness(Buffer::BIG);
    b << m_local_ext_ip;
    b << m_socket->getLocalIP();
    b << (unsigned char)0x04;    // mode
    b.setEndianness(Buffer::LITTLE);
    b << (unsigned int)m_local_server_port;
    b << m_session_id;

    b << (unsigned int)0x00000050; // unknown
    b << (unsigned int)0x00000003; // unknown
    if (m_eff_tcp_version == 7) 
      b << (unsigned int)0x00000000; // unknown
    
    Send(b);
  }

  void DirectClient::ParseInitPacket(Buffer &b) {
    b.setEndianness(Buffer::LITTLE);
    unsigned short length;
    b >> length;

    unsigned char start_byte;
    b >> start_byte;
    if (start_byte != 0xff) throw ParseException("Init Packet didn't start with 0xff");
    
    unsigned short tcp_version;
    b >> tcp_version;
    b.advance(2); // revision or a length ??

    if (m_incoming) {
      m_remote_tcp_version = tcp_version;
      if (tcp_version <= 5) throw ParseException("Too old client < ICQ99");
      if (tcp_version == 6) m_eff_tcp_version = 6;
      else m_eff_tcp_version = 7;
    } else {
      if (tcp_version != m_remote_tcp_version) throw ParseException("Client claiming different TCP versions");
    }
    
    unsigned int our_uin;
    b >> our_uin;
    if (our_uin != m_local_uin) throw ParseException("Local UIN in Init Packet not same as our Local UIN");

    // 00 00
    // xx xx       senders open port
    // 00 00
    b.advance(6);

    unsigned int remote_uin;
    b >> remote_uin;
    if (m_incoming) {
      m_remote_uin = remote_uin;
    } else {
      if (m_remote_uin != remote_uin) throw ParseException("Remote UIN in Init Packet for Remote Client not what was expected");
    }

    // xx xx xx xx  senders external IP
    // xx xx xx xx  senders lan IP
    b.advance(8);

    b >> m_tcp_flags;

    // xx xx        senders port again
    // 00 00
    b.advance(4);

    // xx xx xx xx  session id
    unsigned int session_id;
    b >> session_id;
    if (m_incoming) {
      m_session_id = session_id;
    } else {
      if (m_session_id != session_id) throw ParseException("Session ID from Remote Client doesn't match the one we sent");
    }

    // 50 00 00 00  unknown 
    // 03 00 00 00  unknown
    b.advance(8);

    if (m_eff_tcp_version == 7) {
      b.advance(4); // 00 00 00 00  unknown
    }

  }

  void DirectClient::ParseInitAck(Buffer &b) {
    b.setEndianness(Buffer::LITTLE);
    unsigned short length;
    b >> length;
    if (length != 4) throw ParseException("Init Ack not as expected");

    unsigned int a;
    b >> a;       // should be 0x00000001 really
  }

  void DirectClient::ParseInit2(Buffer &b) {
    b.setEndianness(Buffer::LITTLE);
    unsigned short length;
    b >> length;
    if (length != 0x0021) throw ParseException("V7 final handshake packet incorrect length");

    unsigned char type;
    b >> type;
    if (type != 0x03) throw ParseException("Expecting V7 final handshake packet, received something else");

    unsigned int unknown;
    b >> unknown // 0x0000000a
      >> unknown;// 0x00000001 on genuine connections, otherwise some weird connections which we drop
    if (unknown != 0x00000001) throw DisconnectedException("Ignoring weird direct connection");
    b.advance(24); // unknowns
  }

  void DirectClient::SendInit2() {
    Buffer b(m_translator);
    b.setEndianness(Buffer::LITTLE);
    b << (unsigned short)0x0021;
    b << (unsigned char) 0x03       // start byte
      << (unsigned int)  0x0000000a // unknown
      << (unsigned int)  0x00000001 // unknown
      << (unsigned int)  (m_incoming ? 0x00000001 : 0x00000000) // unknown
      << (unsigned int)  0x00000000 // unknown
      << (unsigned int)  0x00000000; // unknown
    if (m_incoming) {
      b << (unsigned int) 0x00040001 // unknown
	<< (unsigned int) 0x00000000 // unknown
	<< (unsigned int) 0x00000000; // unknown
    } else {
      b << (unsigned int) 0x00000000 // unknown
	<< (unsigned int) 0x00000000 // unknown
	<< (unsigned int) 0x00040001; // unknown
    }
    Send(b);
  }

  void DirectClient::ParsePacket(Buffer& b) {
    Buffer c(m_translator);
    if (!Decrypt(b, c)) throw ParseException("Decrypting failed");
    ParsePacketInt(c);
  }

  void DirectClient::ParsePacketInt(Buffer& b) {
    b.setEndianness(Buffer::LITTLE);
    unsigned short length;
    b >> length;

    // we should get the decrypted packet in
    unsigned int checksum, foreground, background;
    unsigned short command, seqnum, unknown, ackFlags, msgFlags, version;
    unsigned char subCommand, flags;
    unsigned char junk;
    string msg;
    ostringstream ostr;

    if (m_eff_tcp_version == 7) {
      unsigned char start_byte;
      b >> start_byte;
      if (start_byte != 0x02) throw ParseException("Message Packet didn't start with 0x02");
    }

    b >> checksum
      >> command
      >> unknown // 0x000e
      >> seqnum;

    b.advance(12); // unknown 3 ints

    ICQSubType *i = ICQSubType::ParseICQSubType(b, true);
    if (dynamic_cast<UINRelatedSubType*>(i) == NULL) throw ParseException("Unknown ICQ subtype");
    UINRelatedSubType *icqsubtype = dynamic_cast<UINRelatedSubType*>(i);

    icqsubtype->setSeqNum(seqnum);

    if (command == 0) throw ParseException("Invalid TCP Packet");

    unsigned short type = icqsubtype->getType();

    MessageEvent *ev;

    switch(command) {

    case V6_TCP_START:

      if (type == MSG_Type_Normal) {
	NormalICQSubType *nst = static_cast<NormalICQSubType*>(icqsubtype);
	ev = new NormalMessageEvent(m_contact, nst->getMessage(), nst->isMultiParty());
	SignalMessageEvent(ev);

	SendPacketAck(icqsubtype);

      } else if (type == MSG_Type_URL) {
	URLICQSubType *ust = static_cast<URLICQSubType*>(icqsubtype);
	ev = new URLMessageEvent(m_contact, ust->getMessage(), ust->getURL());
	SignalMessageEvent(ev);

	SendPacketAck(icqsubtype);
      } else if (type == MSG_Type_AutoReq_Away
	       || type == MSG_Type_AutoReq_Occ
	       || type == MSG_Type_AutoReq_NA
	       || type == MSG_Type_AutoReq_DND
	       || type == MSG_Type_AutoReq_FFC)
      {
        AwayMsgSubType *ast = static_cast<AwayMsgSubType*>(icqsubtype);
	AwayMessageEvent aev(m_contact);
	want_auto_resp.emit(&aev);
        ast->setMessage(aev.getMessage());

	ostringstream ostr;
	ostr << "Sending direct auto response to "
	   << m_contact->getAlias() << " (" << m_contact->getStringUIN() << ")";
	SignalLog(LogEvent::INFO, ostr.str());
	
        SendPacketAck(icqsubtype);
      }

      break;

    case V6_TCP_ACK:
      if ( m_msgcache.exists(seqnum) ) {
	MessageEvent *ev = m_msgcache[seqnum];
	ev->setFinished(true);
	ev->setDelivered(true);
	ev->setDirect(true);

	if (ev->getType() == MessageEvent::AwayMessage) {
	  AwayMessageEvent *aev = static_cast<AwayMessageEvent*>(ev);

	  if (icqsubtype->getType() == MSG_Type_AutoReq_Away
	      || icqsubtype->getType() == MSG_Type_AutoReq_Occ
	      || icqsubtype->getType() == MSG_Type_AutoReq_NA
	      || icqsubtype->getType() == MSG_Type_AutoReq_DND
	      || icqsubtype->getType() == MSG_Type_AutoReq_FFC) {
	    AwayMsgSubType *ast = static_cast<AwayMsgSubType*>(icqsubtype);
	    aev->setMessage( ast->getMessage() ); // fill out the away message in the ACK
	  } else {
	    SignalLog(LogEvent::WARN, "Away Message ACKed by remote client as wrong type");
	  }
	}

	messageack.emit(ev);

	m_msgcache.remove(seqnum);
	delete ev;
      } else {
	SignalLog(LogEvent::WARN, "Received Direct ACK for unknown message");
      }      
      break;

    default:
      ostr << "Unknown TCP Command received 0x" << command;
      throw ParseException( ostr.str() );
    }

    delete icqsubtype;

    b >> junk
      >> version;
  }

  bool DirectClient::Decrypt(Buffer& in, Buffer& out) {

    if (m_eff_tcp_version >= 6) {
      // Huge *thanks* to licq for this code
    
      unsigned long hex, key, B1, M1;
      unsigned int i;
      unsigned char X1, X2, X3;
      unsigned int correction;

      if (m_eff_tcp_version == 7) correction = 3;
      else correction = 2;

      unsigned int size = in.size()-correction;
      
      in.setEndianness(Buffer::LITTLE);
      out.setEndianness(Buffer::LITTLE);

      unsigned short length;
      in >> length;
      out << length;

      if (m_eff_tcp_version == 7) {
	unsigned char start_byte;
	in >> start_byte;
	out << start_byte;
      }

      unsigned int check;
      in >> check;
      out << check;

      // main XOR key
      key = 0x67657268 * size + check;
      
      for(i=4; i<(size+3)/4; i+=4) {
	hex = key + client_check_data[i&0xFF];
	
	out << (unsigned char)(in.UnpackChar() ^ (hex&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>8)&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>16)&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>24)&0xFF));
      }
      
      unsigned char c;
      while (in.remains()) {
	in >> c;
	out << c;
      }

      B1 = (out[4+correction]<<24) | (out[6+correction]<<16) | (out[4+correction]<<8) | (out[6+correction]<<0);
      
      // special decryption
      B1 ^= check;
      
      // validate packet
      M1 = (B1 >> 24) & 0xFF;
      if(M1 < 10 || M1 >= size) return false;

      X1 = out[M1+correction] ^ 0xFF;
      if(((B1 >> 16) & 0xFF) != X1) return false;
      
      X2 = ((B1 >> 8) & 0xFF);
      if(X2 < 220) {
	X3 = client_check_data[X2] ^ 0xFF;
	if((B1 & 0xFF) != X3) return false;
      }
    }

    ostringstream ostr;
    ostr << "Decrypted Direct packet from "  << IPtoString( m_socket->getRemoteIP() ) << ":" << m_socket->getRemotePort() << endl << out;
    SignalLog(LogEvent::DIRECTPACKET, ostr.str());
      
    return true;
  }

  void DirectClient::Encrypt(Buffer& in, Buffer& out) {

    ostringstream ostr;
    ostr << "Unencrypted packet to "  << IPtoString( m_socket->getRemoteIP() ) << ":" << m_socket->getRemotePort() << endl << in;
    SignalLog(LogEvent::DIRECTPACKET, ostr.str());
      
    if (m_eff_tcp_version == 6 || m_eff_tcp_version == 7) {
      // Huge *thanks* to licq for this code
    
      unsigned long hex, key, B1, M1;
      unsigned int i, check;
      unsigned char X1, X2, X3;
      unsigned int size = in.size();

      in.setEndianness(Buffer::LITTLE);
      out.setEndianness(Buffer::LITTLE);

      if (m_eff_tcp_version == 7) {
	// correction for next byte
	out << (unsigned short)(size + 1);
	out << (unsigned char)0x02;
      } else {
	out << (unsigned short)size;
      }

      // calculate verification data
      M1 = (rand() % ((size < 255 ? size : 255)-10))+10;
      X1 = in[M1] ^ 0xFF;
      X2 = rand() % 220;
      X3 = client_check_data[X2] ^ 0xFF;

      B1 = (in[4]<<24)|(in[6]<<16)|(in[4]<<8)|(in[6]);

      // calculate checkcode
      check = (M1 << 24) | (X1 << 16) | (X2 << 8) | X3;
      check ^= B1;

      out << check;

      // main XOR key
      key = 0x67657268 * size + check;
      
      // XORing the actual data
      in.advance(4);
      for(i=4;i<(size+3)/4;i+=4){
	hex = key + client_check_data[i&0xFF];

	out << (unsigned char)(in.UnpackChar() ^ (hex&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>8)&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>16)&0xFF));
	out << (unsigned char)(in.UnpackChar() ^ ((hex>>24)&0xFF));
      }

      unsigned char c;
      while (in.remains()) {
	in >> c;
	out << c;
      }

    }

  }

  void DirectClient::SendInitAck() {
    Buffer b(m_translator);
    b.setEndianness(Buffer::LITTLE);
    b << (unsigned short)0x0004;
    b << (unsigned int)0x00000001;
    Send(b);
  }

  void DirectClient::SendPacketAck(UINRelatedSubType *icqsubtype) {
    Buffer b(m_translator);

    b.setEndianness(Buffer::LITTLE);
    b << (unsigned int)0x00000000 // checksum (filled in by Encrypt)
      << V6_TCP_ACK
      << (unsigned short)0x000e
      << icqsubtype->getSeqNum()
      << (unsigned int)0x00000000
      << (unsigned int)0x00000000
      << (unsigned int)0x00000000;
    icqsubtype->setACK(true);
    icqsubtype->Output(b);
    Buffer c(m_translator);
    Encrypt(b,c);
    Send(c);
  }

  void DirectClient::SendPacketEvent(MessageEvent *ev) {
    Buffer b(m_translator);

    unsigned short seqnum = NextSeqNum();

    Contact *co = ev->getContact();

    ICQSubType *ist = NULL;
    if (ev->getType() == MessageEvent::Normal) {
      NormalMessageEvent *nv = static_cast<NormalMessageEvent*>(ev);
      ist = new NormalICQSubType(nv->getMessage(), co->getUIN(), true);
    } else if (ev->getType() == MessageEvent::URL) {
      URLMessageEvent *uv = static_cast<URLMessageEvent*>(ev);
      ist = new URLICQSubType(uv->getMessage(), uv->getURL(), m_local_uin, co->getUIN(), true);
    } else if (ev->getType() == MessageEvent::AwayMessage) {
      ist = new AwayMsgSubType( co->getStatus(), co->getUIN() );
    }
    if (ist == NULL) return;

    b.setEndianness(Buffer::LITTLE);
    b << (unsigned int)0x00000000 // checksum (filled in by Encrypt)
      << V6_TCP_START
      << (unsigned short)0x000e
      << seqnum
      << (unsigned int)0x00000000
      << (unsigned int)0x00000000
      << (unsigned int)0x00000000;
    ist->Output(b);

    Buffer c(m_translator);
    Encrypt(b,c);
    Send(c);

    delete ist;

    m_msgcache.insert(seqnum, ev);
  }

  void DirectClient::Send(Buffer &b) {
    try {
      ostringstream ostr;
      ostr << "Sending packet to "  << IPtoString( m_socket->getRemoteIP() ) << ":" << m_socket->getRemotePort() << endl << b;
      SignalLog(LogEvent::DIRECTPACKET, ostr.str());
      m_socket->Send(b);
    } catch(SocketException e) {
      ostringstream ostr;
      ostr << "Failed to send: " << e.what();
      throw DisconnectedException( ostr.str() );
    }
  }

  void DirectClient::SendEvent(MessageEvent *ev) {

    if (m_state == CONNECTED) {
      // send straight away
      SendPacketEvent(ev);
    } else {
      // queue message
      m_msgqueue.push_back(ev);
    }

  }

  void DirectClient::flush_queue() {
    while (!m_msgqueue.empty()) {
      SendPacketEvent( m_msgqueue.front() );
      m_msgqueue.pop_front();
    }
  }

  unsigned short DirectClient::NextSeqNum() {
    return m_seqnum--;
  }

  unsigned int DirectClient::getUIN() const { return m_remote_uin; }

  unsigned int DirectClient::getIP() const { return m_socket->getRemoteIP(); }

  unsigned short DirectClient::getPort() const { return m_socket->getRemotePort(); }

  int DirectClient::getfd() const { return m_socket->getSocketHandle(); }

  TCPSocket* DirectClient::getSocket() const { return m_socket; }

  void DirectClient::setContact(Contact *c) { m_contact = c; }

  Contact* DirectClient::getContact() const { return m_contact; }

  DirectClientException::DirectClientException() { }
  DirectClientException::DirectClientException(const string& text) : m_errortext(text) { }

  const char* DirectClientException::what() const throw() { return m_errortext.c_str(); }

  DisconnectedException::DisconnectedException(const string& text) : DirectClientException(text) { }
  
}

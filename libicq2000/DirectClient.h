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

#ifndef DIRECTCLIENT_H
#define DIRECTCLIENT_H

#include <list>
#include <string>

#include <sigc++/signal_system.h>

#include <stdlib.h>

#include <libicq2000/socket.h>
#include <libicq2000/buffer.h>
#include <libicq2000/events.h>
#include <libicq2000/exceptions.h>
#include <libicq2000/ICQ.h>
#include <libicq2000/Contact.h>
#include <libicq2000/ContactList.h>
#include <libicq2000/SeqNumCache.h>
#include <libicq2000/Translator.h>

using std::string;
using std::list;
using std::exception;
using SigC::Signal0;
using SigC::Signal1;

namespace ICQ2000 {

  class DirectClient : public SigC::Object {
   private:
    enum State { NOT_CONNECTED,
		 WAITING_FOR_INIT,
		 WAITING_FOR_INIT_ACK,
		 WAITING_FOR_INIT2,
		 CONNECTED };

    State m_state;

    TCPSocket *m_socket;
    Buffer m_recv;

    Contact *m_contact;
    ContactList *m_contact_list;

    bool m_incoming;

    unsigned short m_remote_tcp_version;
    unsigned int m_remote_uin;
    unsigned char m_tcp_flags;
    unsigned short m_eff_tcp_version;

    unsigned int m_local_uin, m_local_ext_ip, m_session_id;
    unsigned short m_local_server_port;

    void Parse();
    void ParseInitPacket(Buffer &b);
    void ParseInitAck(Buffer &b);
    void ParseInit2(Buffer &b);
    void ParsePacket(Buffer& b);
    void ParsePacketInt(Buffer& b);

    void SendInitAck();
    void SendInitPacket();
    void SendInit2();
    void SendPacketEvent(MessageEvent *ev);
    void SendPacketAck(UINRelatedSubType *i);
    void Send(Buffer &b);
    
    bool Decrypt(Buffer& in, Buffer& out);
    void Encrypt(Buffer& in, Buffer& out);
    static unsigned char client_check_data[];
    Translator *m_translator;
    SeqNumCache m_msgcache;
    list<MessageEvent*> m_msgqueue;
    unsigned short m_seqnum;

    unsigned short NextSeqNum();
    void ConfirmUIN();

    void expired_cb(MessageEvent *ev);
    void flush_queue();

    void Init();

    void SignalAddSocket(int fd, SocketEvent::Mode m);
    void SignalRemoveSocket(int fd);

   public:
    DirectClient(TCPSocket *sock, ContactList *cl, unsigned int uin, unsigned int ext_ip, unsigned short server_port, Translator* translator);
    DirectClient(Contact *c, unsigned int uin, unsigned int ext_ip, unsigned short server_port, Translator *translator);
    ~DirectClient();

    void Connect();
    void FinishNonBlockingConnect();
    void Recv();

    // ------------------ Signal dispatchers -----------------
    void SignalLog(LogEvent::LogType type, const string& msg);
    void SignalMessageEvent(MessageEvent *ev);
    // ------------------  Signals ---------------------------
    Signal1<void,LogEvent*> logger;
    Signal1<void,MessageEvent*> messaged;
    Signal1<void,MessageEvent*> messageack;
    Signal1<void,SocketEvent*> socket;
    Signal0<void> connected;

    unsigned int getUIN() const;
    unsigned int getIP() const;
    unsigned short getPort() const;
    int getfd() const;
    TCPSocket* getSocket() const;

    void setContact(Contact* c);
    Contact* getContact() const;
    void SendEvent(MessageEvent* ev);
  };

  class DirectClientException : public exception {
   private:
    string m_errortext;
    
   public:
    DirectClientException();
    DirectClientException(const string& text);
    ~DirectClientException() throw() { }

    const char* what() const throw();
  };
  
  class DisconnectedException : public DirectClientException {
   public:
    DisconnectedException(const string& text);
  };

}

#endif

/*
 * TCPSocket class
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

#include "sstream_fix.h"
#include <algorithm>

#include "socket.h"

using std::ostringstream;
using std::istringstream;
//using std::copy;

// StringtoIP and IPtoString both work on host order ip address expressed as unsigned int's
unsigned int StringtoIP(const string& s) {
  istringstream istr(s);
  unsigned char d1,d2,d3,d4;
  unsigned int b1,b2,b3,b4;
  istr >> b1 >> d1 >> b2 >> d2 >> b3 >> d3 >> b4;
  if (!istr) return 0;
  istr >> d4;
  if (istr) return 0;

  if (d1 == '.' && d2 == '.' && d3 == '.'
      && b1 < 256 && b2 < 256 && b3 < 256 && b4 < 256) {
    return (b1 << 24) | (b2 << 16) | (b3 << 8) | (b4 << 0);
  } else {
    return 0;
  }
};

string IPtoString(unsigned int ip) {
  ostringstream ostr;
  ostr << (ip >> 24) << "."
       << ((ip >> 16) & 0xff) << "."
       << ((ip >> 8) & 0xff) << "."
       << (ip & 0xff);
  return ostr.str();
}

TCPSocket::TCPSocket()
  : socketDescriptor(-1), blocking(false), m_state(NOT_CONNECTED)
{
  memset(&remoteAddr, 0, sizeof(remoteAddr));
}

TCPSocket::TCPSocket( int fd, struct sockaddr_in addr )
  : socketDescriptor(fd), remoteAddr(addr), blocking(false), m_state(CONNECTED)
{
  socklen_t localLen = sizeof(struct sockaddr_in);
  getsockname( socketDescriptor, (struct sockaddr *)&localAddr, &localLen );

  fcntlSetup();
}

TCPSocket::~TCPSocket() {
  Disconnect();
}

void TCPSocket::Connect() {
  if (m_state != NOT_CONNECTED) throw SocketException("Already connected");

  socketDescriptor = socket(AF_INET,SOCK_STREAM,0);
  if (socketDescriptor == -1) throw SocketException("Couldn't create socket");
  remoteAddr.sin_family = AF_INET;

  fcntlSetup();

  if (connect(socketDescriptor,(struct sockaddr *)&remoteAddr,sizeof(struct sockaddr)) == -1) {
    if (errno == EINPROGRESS) {
      m_state = NONBLOCKING_CONNECT;
      return; // non-blocking connect
    }

    close(socketDescriptor);
    socketDescriptor = -1;
    throw SocketException("Couldn't connect socket");
  }

  socklen_t localLen = sizeof(struct sockaddr_in);
  getsockname( socketDescriptor, (struct sockaddr *)&localAddr, &localLen );

  m_state = CONNECTED;
}

void TCPSocket::FinishNonBlockingConnect() {
  // this should be called for non blocking connects
  // after the socket is writeable
  int so_error;
  socklen_t optlen = sizeof(so_error);
  if (getsockopt(socketDescriptor, SOL_SOCKET, SO_ERROR, &so_error, &optlen) == -1 || so_error != 0) {
    close(socketDescriptor);
    socketDescriptor = -1;
    throw SocketException("Couldn't connect socket");
  }

  // success
  socklen_t localLen = sizeof(struct sockaddr_in);
  getsockname( socketDescriptor, (struct sockaddr *)&localAddr, &localLen );

  m_state = CONNECTED;
}

void TCPSocket::Disconnect() {
  if (socketDescriptor != -1) {
    close(socketDescriptor);
    socketDescriptor = -1;
  }
  m_state = NOT_CONNECTED;
}

int TCPSocket::getSocketHandle() { return socketDescriptor; }

TCPSocket::State TCPSocket::getState() const { return m_state; }

bool TCPSocket::connected() {
  return (m_state == CONNECTED);
}

void TCPSocket::setBlocking(bool b) {
  blocking = b;
  fcntlSetup();
}

bool TCPSocket::isBlocking() const {
  return blocking;
}

void TCPSocket::fcntlSetup() {
  if (socketDescriptor != -1) {
    int f = fcntl(socketDescriptor, F_GETFL);
    if (blocking) fcntl(socketDescriptor, F_SETFL, f & ~O_NONBLOCK);
    else fcntl(socketDescriptor, F_SETFL, f | O_NONBLOCK);
  }
}

void TCPSocket::Send(Buffer& b) {
  if (!connected()) throw SocketException("Not connected");

  int ret;
  unsigned int sent = 0;

  unsigned char data[b.size()];
  copy( b.begin(), b.end(), data );

  while (sent < b.size())
  {
    ret = send(socketDescriptor, data + sent, b.size() - sent, 0);
    if (ret == -1) throw SocketException("Sending on socket");
    sent += ret;
  }
}

bool TCPSocket::Recv(Buffer& b) {
  if (!connected()) throw SocketException("Not connected");

  unsigned char buffer[max_receive_size];

  int ret = recv(socketDescriptor, buffer, max_receive_size, 0);
  if (ret == 0) throw SocketException( "Other end closed connection" );
  if (ret == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) return false;
    else throw SocketException( strerror(errno) );
  }

  b.Pack(buffer,ret);
  return true;
}

void TCPSocket::setRemoteHost(const char *host) {
  remoteAddr.sin_addr.s_addr = gethostname(host);
}

void TCPSocket::setRemotePort(unsigned short port) {
  remoteAddr.sin_port = htons(port);
}

void TCPSocket::setRemoteIP(unsigned int ip) {
  remoteAddr.sin_addr.s_addr = htonl(ip);
}

unsigned short TCPSocket::getRemotePort() const {
  return ntohs( remoteAddr.sin_port );
}

unsigned int TCPSocket::getRemoteIP() const {
  return ntohl( remoteAddr.sin_addr.s_addr );
}

unsigned short TCPSocket::getLocalPort() const {
  return ntohs( localAddr.sin_port );
}

unsigned int TCPSocket::getLocalIP() const {
  return ntohl( localAddr.sin_addr.s_addr );
}


// returns ip address of host in network byte order

unsigned long TCPSocket::gethostname(const char *hostname) {

  unsigned int ip = htonl( StringtoIP(hostname) );
  if (ip != 0) return ip;


  // try and resolve hostname
  struct hostent *hostEnt;
  if ((hostEnt = gethostbyname(hostname)) == NULL || hostEnt->h_addrtype != AF_INET) {
    throw SocketException("DNS lookup failed");
  } else {
    return *((unsigned long *)(hostEnt->h_addr));
  }
}

/**
 * TCPServer class
 */
TCPServer::TCPServer() {
  socketDescriptor = -1;
}

TCPServer::~TCPServer() {
  Disconnect();
}

void TCPServer::StartServer() {

  if (socketDescriptor != -1) throw SocketException("Already listening");
  
  socketDescriptor = socket( AF_INET, SOCK_STREAM, 0 );
  if (socketDescriptor < 0) throw SocketException("Couldn't create socket");
  
  /*
   * don't bother with bind, we don't care which port
   * it picks to listen on, just so long as we can find
   * out which it is
   *   localAddr.sin_family = AF_INET;
   *   localAddr.sin_addr.s_addr = INADDR_ANY;
   *   localAddr.sin_port = 0;
   *
   *   if ( bind( socketDescriptor,
   *   (struct sockaddr *)&localAddr,
   *   sizeof(struct sockaddr) ) < 0 ) throw SocketException("Couldn't bind socket");
   */

  listen( socketDescriptor, 5 );
  // queue size of 5 should be sufficient
  
  socklen_t localLen = sizeof(struct sockaddr_in);
  getsockname( socketDescriptor, (struct sockaddr *)&localAddr, &localLen );
}

unsigned short TCPServer::getPort() const {
  return ntohs( localAddr.sin_port );
}

unsigned int TCPServer::getIP() const {
  return ntohl( localAddr.sin_addr.s_addr );
}

TCPSocket* TCPServer::Accept() {
  int newsockfd;
  socklen_t remoteLen;
  struct sockaddr_in remoteAddr;

  if (socketDescriptor == -1) throw SocketException("Not connected");

  remoteLen = sizeof(remoteAddr);
  newsockfd = accept( socketDescriptor,
		      (struct sockaddr *) &remoteAddr, 
		      &remoteLen );
  if (newsockfd < 0) throw SocketException("Error on accept");

  return new TCPSocket( newsockfd, remoteAddr );
}

int TCPServer::getSocketHandle() { return socketDescriptor; }

void TCPServer::Disconnect() {
  if (socketDescriptor != -1) {
    close(socketDescriptor);
    socketDescriptor = -1;
  }
}

bool TCPServer::isStarted() const { return socketDescriptor != -1; }

/**
 * SocketException class
 */
SocketException::SocketException(const string& text) : m_errortext(text) { }

const char* SocketException::what() const throw() {
  return m_errortext.c_str();
}

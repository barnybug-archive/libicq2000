
#include <iostream>
#include "sstream_fix.h"
#include <set>

#include "Client.h"
#include "events.h"
#include "constants.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#include <signal.h>

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

using namespace ICQ2000;
using namespace std;

/*
 * shell.cpp - an example of a simple command line client that uses
 * the ickle ICQ2000 libraries to automatically respond to messages
 * and sms's.
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 *
 * Examples of usage:
 *  ./ickle-shell uin pass cat
 *   will echo back every message sent to it
 *
 *  ./ickle-shell uin pass fortune
 *   will reply with a fortune cookie :-)
 *
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

// ------------------------------------------------------------------
void usage(const char *c);
void processCommandLine(int argc, char *argv[]);

char *password, *shellcmd;
unsigned int uin;
bool respond;

// ------------------------------------------------------------------
// Pipe execution class
// ------------------------------------------------------------------
class PipeExec {
 private:
  FILE *fStdIn, *fStdOut;
  int pid;

 public:
  PipeExec();
  ~PipeExec();

  bool Open(const char *cmd);
  void Read(char *buf, int size);
  void Write(const char *buf);
  void CloseInput();
  void Close();
};

// ------------------------------------------------------------------
// Simple Client declaration
// ------------------------------------------------------------------

class SimpleClient : public SigC::Object {
 private:
  ICQ2000::Client icqclient;
  set<int> rfdl, wfdl, efdl;
  
 public:
  SimpleClient(unsigned int uin, const string& pass);

  void run();

  // -- Callbacks --
  void connected_cb(ConnectedEvent *c);
  void disconnected_cb(DisconnectedEvent *c);
  bool message_cb(MessageEvent *c);
  void logger_cb(LogEvent *c);
  void contactlist_cb(ContactListEvent *c);
  void socket_cb(SocketEvent *ev);
};


// ------------------------------------------------------------------

SimpleClient::SimpleClient(unsigned int uin, const string& pass)
  : icqclient(uin, pass) {

  // set up Callbacks
  icqclient.connected.connect(slot(this,&SimpleClient::connected_cb));
  icqclient.disconnected.connect(slot(this,&SimpleClient::disconnected_cb));
  icqclient.messaged.connect(slot(this,&SimpleClient::message_cb));
  icqclient.logger.connect(slot(this,&SimpleClient::logger_cb));
  icqclient.contactlist.connect(slot(this,&SimpleClient::contactlist_cb));
  icqclient.socket.connect(slot(this,&SimpleClient::socket_cb));
}

void SimpleClient::run() {

  icqclient.setStatus(STATUS_ONLINE);

  while(1) {
    fd_set rfds, wfds, efds;
    struct timeval tv;

    int max_fd = -1;
    
    // this could be done much better..

    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);

    cout << "----------------------------------------" << endl;
    cout << "Selecting on:" << endl;
    cout << " Read: ";
    set<int>::iterator curr = rfdl.begin();
    while (curr != rfdl.end()) {
      FD_SET(*curr, &rfds);
      if (*curr > max_fd) max_fd = *curr;
      cout << *curr << " ";
      ++curr;
    }
    cout << endl;

    curr = wfdl.begin();
    cout << " Write: ";
    while (curr != wfdl.end()) {
      FD_SET(*curr, &wfds);
      if (*curr > max_fd) max_fd = *curr;
      cout << *curr << " ";
      ++curr;
    }
    cout << endl;

    curr = efdl.begin();
    cout << " Exception: ";
    while (curr != efdl.end()) {
      FD_SET(*curr, &efds);
      if (*curr > max_fd) max_fd = *curr;
      cout << *curr << " ";
      ++curr;
    }
    cout << endl;
    cout << "----------------------------------------" << endl;

    // should poll icqclient every 5 seconds once connected
    struct timeval *tvptr;
    if (icqclient.isConnected()) {
      tvptr = &tv;
      tv.tv_sec = 5;
      tv.tv_usec = 0;
    } else {
      tvptr = NULL;
    }

    int ret = select(max_fd+1, &rfds, &wfds, &efds, tvptr);
    if (ret) {
      /*
       * Care must be taken here, when iterating through the sets of
       * file descriptors the Client::socket_cb()'s may signal
       * Adding/Removing - modifying the sets inplace whilst we are
       * iterating through them. Use a temporary copy for safety.
       *
       */

      set<int>rfdt = rfdl;
      curr = rfdt.begin();
      while (curr != rfdt.end()) {
	if ( FD_ISSET( *curr, &rfds ) ) icqclient.socket_cb( *curr, SocketEvent::READ );
	++curr;
      }

      set<int>wfdt = wfdl;
      curr = wfdt.begin();
      while (curr != wfdt.end()) {
	if ( FD_ISSET( *curr, &wfds ) ) icqclient.socket_cb( *curr, SocketEvent::WRITE );
	++curr;
      }

      set<int>efdt = efdl;
      curr = efdt.begin();
      while (curr != efdt.end()) {
	if ( FD_ISSET( *curr, &efds ) ) icqclient.socket_cb( *curr, SocketEvent::EXCEPTION );
	++curr;
      }
      
    } else {
      if (icqclient.isConnected()) {
	icqclient.Poll();
      }
    }	
  }

  // never reached
  icqclient.setStatus(STATUS_OFFLINE);
}

void SimpleClient::socket_cb(SocketEvent *ev) {
  
  if (dynamic_cast<AddSocketHandleEvent*>(ev) != NULL) {
    AddSocketHandleEvent *cev = dynamic_cast<AddSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();

    cout << "connecting socket " << fd << endl;

    if (cev->isRead()) rfdl.insert(fd);
    if (cev->isWrite()) wfdl.insert(fd);
    if (cev->isException()) efdl.insert(fd);

  } else if (dynamic_cast<RemoveSocketHandleEvent*>(ev) != NULL) {
    RemoveSocketHandleEvent *cev = dynamic_cast<RemoveSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();

    cout << "disconnecting socket " << fd << endl;

    rfdl.erase(fd);
    wfdl.erase(fd);
    efdl.erase(fd);
  }
  
}

void SimpleClient::connected_cb(ConnectedEvent *c) {
  cout << "ickle-shell: Connected" << endl;
}

void SimpleClient::disconnected_cb(DisconnectedEvent *c) {
  if (c->getReason() == DisconnectedEvent::REQUESTED) {
    cout << "ickle-shell: Disconnected as requested" << endl;
  } else {
    cout << "ickle-shell: Problem connecting: ";
    switch(c->getReason()) {
    case DisconnectedEvent::FAILED_LOWLEVEL:
      cout << "Socket problems";
      break;
    case DisconnectedEvent::FAILED_BADUSERNAME:
      cout << "Bad Username";
      break;
    case DisconnectedEvent::FAILED_TURBOING:
      cout << "Turboing";
      break;
    case DisconnectedEvent::FAILED_BADPASSWORD:
      cout << "Bad Password";
      break;
    case DisconnectedEvent::FAILED_MISMATCH_PASSWD:
      cout << "Username and Password did not match";
      break;
    case DisconnectedEvent::FAILED_UNKNOWN:
      cout << "Unknown";
      break;
    }
    cout << endl;
  }
}

bool SimpleClient::message_cb(MessageEvent *c) {

  if (c->getType() == MessageEvent::Normal) {

    NormalMessageEvent *msg = static_cast<NormalMessageEvent*>(c);
    cout << "ickle-shell: Message received: " << msg->getMessage() << " from " << msg->getSenderUIN() << endl;

    if (respond) {
      PipeExec pp;
      pp.Open( shellcmd );
      string m = msg->getMessage();
      pp.Write( m.c_str() );
      char ret[4097];
      pp.CloseInput();
      pp.Read( ret, 4097 );
      pp.Close();
      NormalMessageEvent *sv = new NormalMessageEvent( c->getContact(), ret );
      icqclient.SendEvent( sv );

      cout << "ickle-shell: Autoresponded with " << ret << endl;
    }

  } else if (c->getType() == MessageEvent::SMS) {

    SMSMessageEvent *smsmsg = static_cast<SMSMessageEvent*>(c);
    cout << "ickle-shell: SMS received: from: " << smsmsg->getSender() << ": " << smsmsg->getMessage() << endl;
    
    if (respond) {
      PipeExec pp;
      pp.Open( shellcmd );
      string m = smsmsg->getMessage();
      pp.Write( m.c_str() );
      char ret[4097];
      pp.CloseInput();
      pp.Read( ret, 4097 );
      pp.Close();
      SMSMessageEvent *sv = new SMSMessageEvent( c->getContact(), ret, true );
      icqclient.SendEvent( sv );

      cout << "ickle-shell: Autoresponded with " << ret << endl;
    }

  } else if (c->getType() == MessageEvent::SMS_Receipt) {

    SMSReceiptEvent *rcpt = static_cast<SMSReceiptEvent*>(c);
    if (rcpt->delivered()) {
      cout << "ickle-shell: SMS " << rcpt->getMessage() << " delivered" << endl;
    } else {
      cout << "ickle-shell: SMS " << rcpt->getMessage() << " not delivered" << endl;
    }
  }

  return true;
}

void SimpleClient::logger_cb(LogEvent *c) {

  switch(c->getType()) {
  case LogEvent::INFO:
    cout << "[34m";
    break;
  case LogEvent::WARN:
    cout << "[31m";
    break;
  case LogEvent::PACKET:
    cout << "[32m";
    break;
  }

  cout << c->getMessage() << endl;
  cout << "[39m";
}

void SimpleClient::contactlist_cb(ContactListEvent *c) {
  if (c->getType() == ContactListEvent::StatusChange) {
    StatusChangeEvent *u = static_cast<StatusChangeEvent*>(c);
    cout << "ickle-shell: User " << u->getUIN() << " went ";
    switch(u->getStatus()) {
    case STATUS_ONLINE:
      cout << "online";
      break;
    case STATUS_DND:
      cout << "DND";
      break;
    case STATUS_NA:
      cout << "NA";
      break;
    case STATUS_OCCUPIED:
      cout << "occupied";
      break;
    case STATUS_FREEFORCHAT:
      cout << "free for chat";
      break;
    case STATUS_AWAY:
      cout << "away";
      break;
    case STATUS_OFFLINE:
      cout << "offline";
      break;
    default:
      cout << "unknown";
    }
    cout << endl;

  }
}

// ------------------------------------------------------
// Pipe Execution class
// ------------------------------------------------------

PipeExec::PipeExec() : fStdIn(NULL), fStdOut(NULL)
{ }

PipeExec::~PipeExec() { 
  Close();
}

bool PipeExec::Open(const char *shellcmd) {
  int pdes_out[2], pdes_in[2];

  if (pipe(pdes_out) < 0) return false;
  if (pipe(pdes_in) < 0) return false;

  switch (pid = fork())
  {
    case -1:                        /* Error. */
    {
      close(pdes_out[0]);
      close(pdes_out[1]);
      close(pdes_in[0]);
      close(pdes_in[1]);
      return false;
      /* NOTREACHED */
    }
  case 0:                         /* Child. */
    {
      if (pdes_out[1] != STDOUT_FILENO) {
        dup2(pdes_out[1], STDOUT_FILENO);
        close(pdes_out[1]);
      }
      
      close(pdes_out[0]);
      
      if (pdes_in[0] != STDIN_FILENO) {
        dup2(pdes_in[0], STDIN_FILENO);
	close(pdes_in[0]);
      }
      close(pdes_in[1]);
      system(shellcmd);
      exit(0);
      /* NOTREACHED */
    }
  }

  /* Parent; assume fdopen can't fail. */
  fStdOut = fdopen(pdes_out[0], "r");
  close(pdes_out[1]);
  fStdIn = fdopen(pdes_in[1], "w");
  close(pdes_in[0]);

  // Set both streams to line buffered
#ifdef SETVBUF_REVERSED
  setvbuf(fStdOut, _IOLBF, (char*)NULL, 0);
  setvbuf(fStdIn, _IOLBF, (char*)NULL, 0);
#else
  setvbuf(fStdOut, (char*)NULL, _IOLBF, 0);
  setvbuf(fStdIn, (char*)NULL, _IOLBF, 0);
#endif

  return true;
}

void PipeExec::Read(char *buf, int size) {
  int pos = 0;
  int c;
  while (((c = fgetc(fStdOut)) != EOF) && (pos < size)) buf[pos++] = (unsigned char)c;
  buf[pos] = '\0';
}

void PipeExec::Write(const char *buf) {
  fprintf(fStdIn, "%s", buf);
}

void PipeExec::CloseInput() {
  fclose(fStdIn);
  fStdIn = NULL;
}

void PipeExec::Close() {
   int r, pstat;
   struct timeval tv = { 0, 200000 };

   // Close the file descriptors
   if (fStdOut != NULL) fclose(fStdOut);
   if (fStdIn != NULL) fclose(fStdIn);
   fStdOut = fStdIn = NULL;

   if (pid == 0) return;

   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);

   // Return if child has exited or there was an inor
   if (r == pid || r == -1) return;
     
   // Give the process another .2 seconds to die
   select(0, NULL, NULL, NULL, &tv);
   
   // Still there?
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) return;
     
   // Try and kill the process
   if (kill(pid, SIGTERM) == -1) return;
   
   // Give it 1 more second to die
   tv.tv_sec = 1;
   tv.tv_usec = 0;
   select(0, NULL, NULL, NULL, &tv);
   
   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) return;

   // Kill, kill, keeeiiiiilllllll!!
   kill(pid, SIGKILL);
   // Now he will die for sure
   waitpid(pid, &pstat, 0);

}

// --------------------------------------------
// main ---------------------------------------
// --------------------------------------------

int main(int argc, char *argv[]) {
  cout << "-- ickle-shell - a simple command line implementation using --" << endl
       << "--               the ickle ICQ2000 libraries                --" << endl << endl;

  processCommandLine( argc, argv );
  
  SimpleClient cl(uin, password);
  cl.run();
}

void usage(const char *progname) {
  cerr << "Usage: " << progname << " [options] uin password shellcommand" << endl
       << " -h              This screen" << endl
       << " -n              Never respond with an SMS" << endl;
  exit (1);
}

void processCommandLine( int argc, char *argv[] ) {

  int i = 0;

  respond = true;

  while ( ( i = getopt( argc, argv, "hn" ) ) > 0) {
    switch(i) {
    case 'n':
      respond = false;
      break;
    default:
      usage ( argv[0] );
      break;
    }
  }

  if (optind != argc-3) usage (argv[0]);

  istringstream istr( argv[optind] );
  istr >> uin;
  password = argv[optind+1];
  shellcmd = argv[optind+2];

}

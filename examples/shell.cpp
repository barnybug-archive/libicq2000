/*
 * shell.cpp - an example of a simple command line client that uses
 * the ickle ICQ2000 libraries to automatically respond to messages
 * and sms's.
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
 * Examples of usage:
 *  ./ickle-shell uin pass cat
 *   will echo back every message sent to it
 *
 *  ./ickle-shell uin pass fortune
 *   will reply with a fortune cookie :-)
 *
 *
 * This example uses two helper classes:
 *
 * - PipeExec: a class that handles forking a shell command, feeding
 * it input, collected it's output, and terminating/killing it. This
 * class has no particular relevance to libicq2000, and it just here
 * for the coolness of the example. :-)
 *
 * - Select: this class wraps the select() system call up in a nice
 * interface so we don't have to worry about file descriptor lists,
 * etc.. we just register callbacks with it and pass these back to the
 * library. Often the library you use for your user interface will
 * handle this for you too - ie. ncurses, gtk+ or gtkmm. If you are
 * building you own interface from scratch, then this class might come
 * in handy, with a few extensions..
 *
 */

#include "shell.h"

// ------------------------------------------------------------------
//  Simple Client
// ------------------------------------------------------------------

SimpleClient::SimpleClient(unsigned int uin, const string& pass)
  : icqclient(uin, pass) {

  /*
   * set up the libicq2000 callbacks: the sigslot callback system is used
   * extensively in libicq2000, and when an event happens we can
   * register callbacks for methods to be called, which in turn will
   * be called when the relevant event happens
   */
  icqclient.connected.connect(this,&SimpleClient::connected_cb);
  icqclient.disconnected.connect(this,&SimpleClient::disconnected_cb);
  icqclient.messaged.connect(this,&SimpleClient::message_cb);
  icqclient.logger.connect(this,&SimpleClient::logger_cb);
  icqclient.contact_status_change_signal.connect(this,&SimpleClient::contact_status_change_cb);
  icqclient.socket.connect(this,&SimpleClient::socket_cb);

  input.socket_signal.connect( this, &SimpleClient::select_socket_cb );
}

void SimpleClient::run() {

  icqclient.setStatus(STATUS_ONLINE);

  while (1) {
    /*
     * the input object (a Select object, written to wrap a nicer C++
     * wrapper round the C select(2) system call), handles the lists
     * of file descriptors we need to select on for read or write
     * (multiplex).
     */
    bool b = input.run(5000);

    if (b) icqclient.Poll();
    // timeout was hit - poll the server
  }
  
  // never reached
  icqclient.setStatus(STATUS_OFFLINE);
}

/*
 * this callback will be called when the library has a socket
 * descriptor that needs select'ing on (or not selecting on any
 * longer), we pass it up to the higher-level Select object which
 * wraps all the select system call stuff nicely -- a graphical
 * toolkit might be the other way to handle this for you (for
 * example the way gtkmm does it)
 */
void SimpleClient::socket_cb(SocketEvent *ev) {

  if (dynamic_cast<AddSocketHandleEvent*>(ev) != NULL) {
    // the library requests we start selecting on a socket

    AddSocketHandleEvent *cev = dynamic_cast<AddSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();

    cout << "connecting socket " << fd << endl;

    // register this socket with our Select object
    input.add( fd,
	       // the socket file descriptor to add
	       (Select::SocketInputCondition) 
	       ((cev->isRead() ? Select::Read : 0) |
		(cev->isWrite() ? Select::Write : 0) |
		(cev->isException() ? Select::Exception : 0))
	       // the mode to select on it on
	       );

  } else if (dynamic_cast<RemoveSocketHandleEvent*>(ev) != NULL) {
    // the library requests we stop selecting on a socket

    RemoveSocketHandleEvent *cev = dynamic_cast<RemoveSocketHandleEvent*>(ev);
    int fd = cev->getSocketHandle();

    cout << "disconnecting socket " << fd << endl;
    input.remove( fd );
  }
  
}

/*
 * registered to receive the Select callbacks
 */
void SimpleClient::select_socket_cb(int fd, Select::SocketInputCondition cond)
{
  // inform the library (it is always only library sockets registered with
  // the Select object)
  icqclient.socket_cb(fd,
		      (ICQ2000::SocketEvent::Mode)cond
		      // dirty-hack, since they are binary compatible :-)
		      );
}

/*
 * called when the library has connected
 */
void SimpleClient::connected_cb(ConnectedEvent *c) {
  cout << "ickle-shell: Connected" << endl;
}

/*
 * this callback is called when the library needs to indicated
 * connecting failed or we've been disconnected for whatever
 * reason.
 */
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
    default:
      break;
    }
    cout << endl;
  }
}

/*
 * this callback is called when someone messages you. Here we do the
 * fun part of this example, which is to pass the message to a shell
 * command, and send them back the results.
 */
void SimpleClient::message_cb(MessageEvent *c) {

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

}

/*
 * this callback is for debug information the library logs - it is
 * generally very useful, although not immediately useful for any
 * users of your program. It allows great flexibility, as the Client
 * decides where the messages go and some of the formatting of them -
 * so they could be displayed in a Dialog, for example. Here they're
 * dumped out to stdout, with some pretty colours showing the level.
 */
void SimpleClient::logger_cb(LogEvent *c) {

  cout << "libicq2000 message: ";

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
  default:
    break;
  }

  cout << c->getMessage() << endl;
  cout << "[39m";
}

/*
 * this callback is called when a contact on your list changes their
 * status
 */
void SimpleClient::contact_status_change_cb(StatusChangeEvent *ev)
{
  cout << "ickle-shell: User " << ev->getUIN() << " went ";
  switch(ev->getStatus()) {
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

// --------------------------------------------
//  main
// --------------------------------------------

int main(int argc, char *argv[]) {
  cout << "-- ickle-shell - a simple command line implementation using --" << endl
       << "--               the libICQ2000 libraries                   --" << endl << endl;

  processCommandLine( argc, argv );
  
  SimpleClient cl(uin, password);
  cl.run();
}

/*
 * display usage information
 */
void usage(const char *progname) {
  cerr << "Usage: " << progname << " [options] uin password shellcommand" << endl
       << " -h              This screen" << endl
       << " -n              Never respond with an SMS" << endl;
  exit (1);
}

/*
 * process the command line arguments with getopt, the values are
 * stored in global variables for simplicity (not neatness :-)
 */
void processCommandLine( int argc, char *argv[] ) {
  int i = 0;

  if (optind != argc-3) usage (argv[0]);
  // too few arguments, show usage info

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

  istringstream istr( argv[optind] );
  istr >> uin; // read out the UIN as an int

  password = argv[optind+1];
  shellcmd = argv[optind+2];
}

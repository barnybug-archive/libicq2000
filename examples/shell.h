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
 * See the sourcecode for shell.cpp for more info.
 *
 */

#include <iostream>
#include <map>
#include "sstream_fix.h"

#include <libicq2000/Client.h>
#include <libicq2000/events.h>
#include <libicq2000/constants.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif

#include "Select.h"
#include "PipeExec.h"

using namespace ICQ2000;
using namespace std;

// ------------------------------------------------------------------
//  global declarations (ughh..)
// ------------------------------------------------------------------
void usage(const char *c);
void processCommandLine(int argc, char *argv[]);

char *password, *shellcmd;
unsigned int uin;
bool respond;

// ------------------------------------------------------------------
//  Simple Client declaration
// ------------------------------------------------------------------

class SimpleClient : public sigslot::has_slots<> {
 private:
  ICQ2000::Client icqclient;
  Select input;
  std::map<int, SigC::Connection> m_sockets;

 protected:

  // -- Callbacks from libicq2000 --
  void connected_cb(ConnectedEvent *c);
  void disconnected_cb(DisconnectedEvent *c);
  void message_cb(MessageEvent *c);
  void logger_cb(LogEvent *c);
  void contact_status_change_cb(StatusChangeEvent *ev);
  void socket_cb(SocketEvent *ev);

  // -- Callbacks from our Select object --
  void select_socket_cb(int fd, Select::SocketInputCondition cond);
  
 public:
  SimpleClient(unsigned int uin, const string& pass);

  void run();

};

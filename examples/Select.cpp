/*
 * Select (Helper) class - handles list of file descriptors to make
 * select calls a little easier and C++ier.
 *
 * Copyright (C) 2002 Barnaby Gray <barnaby@beedesign.co.uk>
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

#include "Select.h"

Select::Select()
{ }

SigC::Connection Select::connect(const SlotType &s,
			 int source,
			 SocketInputCondition condition)
{
  // this is all SigC voodoo.. I wouldn't concern yourself with it,
  // unless you're really keen
  SigC::SlotData* sd=(SigC::SlotData*)(s.obj());

  // add to map
  if (condition & Read) rfdl.insert( make_pair(source, (Callback*)&(sd->data_)) );
  if (condition & Write) wfdl.insert( make_pair(source, (Callback*)&(sd->data_)) );
  if (condition & Exception) efdl.insert( make_pair(source, (Callback*)&(sd->data_)) );

  // connect it
  sd->connect();

  // register disconnection data
  sd->list_.insert_direct(sd->list_.begin(),
                          new SelectSigCNode());
  return sd;
}

bool Select::run(unsigned int interval)
{
  // fd_set's are the beasts that the select() syscall uses.. man 3
  // select for more info.
  fd_set rfds, wfds, efds;
  struct timeval tv;
  
  int max_fd = -1;
  
  // clear select lists
  FD_ZERO(&rfds);
  FD_ZERO(&wfds);
  FD_ZERO(&efds);
  
  SocketMap::iterator curr = rfdl.begin();
  while (curr != rfdl.end()) {
    FD_SET((*curr).first, &rfds);
    if ((*curr).first > max_fd) max_fd = (*curr).first;
    ++curr;
  }
  
  curr = wfdl.begin();
  while (curr != wfdl.end()) {
    FD_SET((*curr).first, &wfds);
    if ((*curr).first > max_fd) max_fd = (*curr).first;
    ++curr;
  }
  
  curr = efdl.begin();
  while (curr != efdl.end()) {
    FD_SET((*curr).first, &efds);
    if ((*curr).first > max_fd) max_fd = (*curr).first;
    ++curr;
  }
  
  // set up timeout
  tv.tv_sec = interval / 1000;
  tv.tv_usec = (interval % 1000) * 1000;
  
  int ret = select(max_fd+1, &rfds, &wfds, &efds, &tv);
  if (ret) {
    /*
     * Care must be taken here, when iterating through the sets of
     * file descriptors the Client::socket_cb()'s may signal
     * Adding/Removing - modifying the sets inplace whilst we are
     * iterating through them. Use a temporary copy for safety.
     *
     */
    
  } else {
    // timeout
    return true;
  }	
}

SelectSigCNode::SelectSigCNode()
{ }

SelectSigCNode::~SelectSigCNode()
{ }

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

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include "Select.h"

Select::Select()
{ }

void Select::add(int source, SocketInputCondition condition)
{
  if (condition & Read) rfdl.insert(source);
  if (condition & Write) wfdl.insert(source);
  if (condition & Exception) efdl.insert(source);
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
  
  SocketSet::iterator curr = rfdl.begin();
  while (curr != rfdl.end()) {
    FD_SET(*curr, &rfds);
    if (*curr > max_fd) max_fd = *curr;
    ++curr;
  }
  
  curr = wfdl.begin();
  while (curr != wfdl.end()) {
    FD_SET(*curr, &wfds);
    if (*curr > max_fd) max_fd = *curr;
    ++curr;
  }
  
  curr = efdl.begin();
  while (curr != efdl.end()) {
    FD_SET(*curr, &efds);
    if (*curr > max_fd) max_fd = *curr;
    ++curr;
  }
  
  // set up timeout
  tv.tv_sec = interval / 1000;
  tv.tv_usec = (interval % 1000) * 1000;
  
  int ret = select(max_fd+1, &rfds, &wfds, &efds, &tv);
  if (ret) {
    /*
     * Find the descriptor that is ready and make the callback.
     */

    // read descriptors
    bool done = false;
    SocketSet::const_iterator curr = rfdl.begin();
    while (!done && curr != rfdl.end()) {
      if ( FD_ISSET( *curr, &rfds ) ) {
	socket_signal.emit(*curr, Read);
	done = true;
      }
      ++curr;
    }

    curr = wfdl.begin();
    while (!done && curr != wfdl.end()) {
      if ( FD_ISSET( *curr, &wfds ) ) {
	socket_signal.emit(*curr, Write);
	done = true;
      }
      ++curr;
    }
    
    curr = efdl.begin();
    while (!done && curr != efdl.end()) {
      if ( FD_ISSET( *curr, &efds ) ) {
	socket_signal.emit(*curr, Exception);
	done = true;
      }
      ++curr;
    }
    
  } else {
    // timeout
    return true;
  }

  return false;
}

void Select::remove(int fd)
{
  // unmap this file descriptor
  if (rfdl.count(fd) != 0) {
    rfdl.erase(fd);
    return;
  }
  if (wfdl.count(fd) != 0) {
    wfdl.erase(fd);
    return;
  }
  if (efdl.count(fd) != 0) {
    efdl.erase(fd);
    return;
  }
  
}

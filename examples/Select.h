/*
 * Select (Helper) class - handles list of file descriptors to make
 * select calls a little easier and C++ier.
 *
 * Limitations: Doesn't support more than one of READ/WRITE/EXCEPTION
 * on a particular descriptor at once (the disconnection code isn't
 * intelligent enough to)
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

#include <set>
#include "libicq2000/sigslot.h"

#ifndef EXAMPLE_SOCKET_H
#define EXAMPLE_SOCKET_H

// ------------------------------------------------------------------
// Select class
// ------------------------------------------------------------------

class Select {
 public:
  // enums
  enum SocketInputCondition
  {
    Read      = 1 << 0,
    Write     = 1 << 1,
    Exception = 1 << 2
  };
  // this enum is used as a 'bitmask' of the input conditions

 private:
  // the lists of file descriptors
  typedef std::set<int> SocketSet;
  SocketSet rfdl, wfdl, efdl;

 public:
  Select();

  /**
   * Registers the socket source to be signalled on condition.
   */
  void add(int source, SocketInputCondition condition);

  /**
   * Execute the select on the sockets, until one has an event, or
   * until the interval specified expires.
   *
   * @param maximum time before returning (in milliseconds)
   * @return boolean, true if the timeout was hit, false otherwise
   */
  bool run(unsigned int interval = 0);

  /**
   *  Unregister the socket source.
   */
  void remove(int fd);

  /**
   *  The signal object that socket events are signalled on.
   */
  sigslot::signal2<int, SocketInputCondition> socket_signal;
};

#endif // EXAMPLE_SOCKET_H

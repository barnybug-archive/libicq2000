/*
 * DCCache
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

#ifndef DCCACHE_H
#define DCCACHE_H

#include <libicq2000/Cache.h>

#include <sigc++/signal_system.h>

using SigC::Signal1;

namespace ICQ2000 {

  // fd -> DirectClient
  class DCCache : public Cache<int, DirectClient*> {
   public:
    DCCache() { }

    void removeItem(const DCCache::literator& l) {
      delete ((*l).getValue());
      Cache<int, DirectClient*>::removeItem(l);
    }

    void expireItem(const DCCache::literator& l) {
      expired.emit( (*l).getValue() );
      Cache<int, DirectClient*>::expireItem(l);
    }

    void removeContact(Contact *c) {
      literator curr = m_list.begin();
      literator next = curr;
      while ( curr != m_list.end() ) {
	DirectClient *dc = (*curr).getValue();
	++next;
	if ( dc->getContact() == c ) {
	  dc->setContact(NULL); // invalidate contact so the DC doesn't attempt to redeliver
	  removeItem(curr);
	}
	curr = next;
      }
    }

    void clearoutMessagesPoll() {
      literator curr = m_list.begin();
      literator next = curr;
      while ( curr != m_list.end() ) {
	DirectClient *dc = (*curr).getValue();
	dc->clearoutMessagesPoll();
	++curr;
      }
    }

    Signal1<void,DirectClient*> expired;
  };
  
}

#endif

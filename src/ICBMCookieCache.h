/*
 * ICBMCookieCache
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

#ifndef ICBMCOOKIECACHE_H
#define ICBMCOOKIECACHE_H

#include "Cache.h"
#include "ICBMCookie.h"
#include "events.h"

#include <sigc++/signal_system.h>

using SigC::Signal1;

namespace ICQ2000 {

  class ICBMCookieCache : public Cache<ICBMCookie, MessageEvent*> {
   public:
    ICBMCookieCache() { }

    void removeItem(const ICBMCookieCache::literator& l) {
      delete ((*l).getValue());
      Cache<ICBMCookie, MessageEvent*>::removeItem(l);
    }

    void expireItem(const ICBMCookieCache::literator& l) {
      expired.emit( (*l).getValue() );
      Cache<ICBMCookie, MessageEvent*>::expireItem(l);
    }

    ICBMCookie generateUnique() const {
      ICBMCookie c;
      c.generate();
      while (exists(c)) c.generate();
      return c;
    }

    void removeContact(Contact *c) {
      literator curr = m_list.begin();
      literator next = curr;
      while ( curr != m_list.end() ) {
	++next;
	MessageEvent *ev = (*curr).getValue();
	if ( ev->getContact() == c ) removeItem(curr);
	curr = next;
      }
    }

    Signal1<void,MessageEvent*> expired;

  };
}

#endif

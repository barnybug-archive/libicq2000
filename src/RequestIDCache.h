/*
 * RequestIDCache
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

#ifndef REQUESTIDCACHE_H
#define REQUESTIDCACHE_H

#include "Cache.h"

namespace ICQ2000 {

  class RequestIDCacheValue {
   public:
    enum Type {
      UserInfo,
      SMSMessage
    };

    virtual Contact* getContact() const = 0;
    virtual Type getType() const = 0;
  };

  class UserInfoCacheValue : public RequestIDCacheValue {
   private:
    Contact *m_contact;

   public:
    UserInfoCacheValue(Contact *c) : m_contact(c) { }
    unsigned int getUIN() const { return m_contact->getUIN(); }
    Contact* getContact() const { return m_contact; }

    Type getType() const { return UserInfo; }
  };

  class SMSEventCacheValue : public RequestIDCacheValue {
   private:
    SMSMessageEvent *m_ev;

   public:
    SMSEventCacheValue( SMSMessageEvent *ev ) : m_ev(ev) { }
    ~SMSEventCacheValue() { delete m_ev; }
    SMSMessageEvent* getEvent() const { return m_ev; }
    Contact* getContact() const { return m_ev->getContact(); }

    Type getType() const { return SMSMessage; }
  };

  class RequestIDCache : public Cache<unsigned int, RequestIDCacheValue*> {
   public:
    RequestIDCache() { }

    void removeItem(const RequestIDCache::literator& l) {
      delete ((*l).getValue());
      Cache<unsigned int, RequestIDCacheValue*>::removeItem(l);
    }

    void removeContact(Contact *c) {
      literator curr = m_list.begin();
      literator next = curr;
      while ( curr != m_list.end() ) {
	++next;
	RequestIDCacheValue *cv = (*curr).getValue();
	if ( cv->getContact() == c ) removeItem(curr);
	curr = next;
      }
    }

  };
  
}

#endif

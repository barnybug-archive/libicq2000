/*
 * RequestIDCache
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

#ifndef REQUESTIDCACHE_H
#define REQUESTIDCACHE_H

#include <libicq2000/Cache.h>

#include <sigc++/signal_system.h>

namespace ICQ2000 {

  class RequestIDCacheValue {
   public:
    enum Type {
      UserInfo,
      SMSMessage,
      Search
    };

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

  class SearchCacheValue : public RequestIDCacheValue {
   private:
    SearchResultEvent *m_ev;

   public:
    SearchCacheValue( SearchResultEvent *ev ) : m_ev(ev) { }
    SearchResultEvent* getEvent() const { return m_ev; }
    Type getType() const { return Search; }
  };

  class RequestIDCache : public Cache<unsigned int, RequestIDCacheValue*> {
   public:
    RequestIDCache() { }

    SigC::Signal1<void,RequestIDCacheValue*> expired;
    
    void expireItem(const RequestIDCache::literator& l) {
      expired.emit( (*l).getValue() );
      Cache<unsigned int, RequestIDCacheValue*>::expireItem(l);
    }

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
	if ( cv->getType() == RequestIDCacheValue::UserInfo ) {
	  UserInfoCacheValue *ccv = static_cast<UserInfoCacheValue*>(cv);
	  if (ccv->getContact() == c) removeItem(curr);
	} else if (cv->getType() == RequestIDCacheValue::SMSMessage ) {
	  SMSEventCacheValue *ccv = static_cast<SMSEventCacheValue*>(cv);
	  if (ccv->getContact() == c) removeItem(curr);
	}
	
	curr = next;
      }
    }

  };
  
}

#endif

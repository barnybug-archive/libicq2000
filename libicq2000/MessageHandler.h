/**
 * MessageHandler
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

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <sigc++/signal_system.h>

#include <libicq2000/Contact.h>
#include <libicq2000/events.h>

namespace ICQ2000 {
  
  class ContactList;
  class UINICQSubType;
  class MessageEvent;
  class ICQMessageEvent;

  class MessageHandler : public SigC::Object {
   private:
    ContactRef m_self_contact;
    ContactList *m_contact_list;
    
    ICQMessageEvent* UINICQSubTypeToEvent(UINICQSubType *st, ContactRef contact);
    UINICQSubType* EventToUINICQSubType(MessageEvent *ev);

    void SignalLog(LogEvent::LogType type, const std::string& msg);
    
  public:
    MessageHandler(ContactRef self, ContactList *cl);

    // incoming messages
    bool handleIncoming(UINICQSubType* icq);

    // outgoing messages
    UINICQSubType* handleOutgoing(MessageEvent *ev);

    // incoming ACKs
    void handleIncomingACK(MessageEvent *ev, UINICQSubType* icq);

    SigC::Signal1<void,MessageEvent*> messaged;
    SigC::Signal1<void,MessageEvent*> messageack;
    SigC::Signal1<void,ICQMessageEvent*> want_auto_resp;
    SigC::Signal1<void,LogEvent*> logger;
  };
}

#endif

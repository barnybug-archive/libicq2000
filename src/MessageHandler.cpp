/*
 * Message Handler
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

#include <libicq2000/MessageHandler.h>
#include <libicq2000/ContactList.h>
#include <libicq2000/SNAC-MSG.h>
#include <libicq2000/SNAC-SRV.h>
#include "sstream_fix.h"

namespace ICQ2000 {

  MessageHandler::MessageHandler(ContactRef self, ContactList *cl)
    : m_self_contact(self), m_contact_list(cl)
  { }
  
  /*
   * This method handles:
   * - Receiving incoming messages
   * - Signalling the message
   * - Setting up the UINICQSubType for the ACK
   * (the actual ACK is sent in the caller - dependent on whether it
   *  was direct/thru server)
   */
  bool MessageHandler::handleIncoming(UINICQSubType *ust)
  {
    ContactRef contact;
    unsigned int uin = ust->getSource();

    if (m_contact_list->exists(uin)) {
      contact = m_contact_list->lookup_uin(uin);
    } else {
      contact = ContactRef( new Contact(uin) );
    }
    
    /* update the status of the contact with that sent in the packet */
    unsigned short s = ust->getStatus();
    contact->setStatus( Contact::MapICQStatusToStatus(s),
			Contact::MapICQStatusToInvisible(s) );


    ICQMessageEvent *ev = UINICQSubTypeToEvent(ust, contact);
    if (ev->getType() != MessageEvent::AwayMessage) {
      messaged.emit(ev);
    } else {
      ev->setDelivered(true);
    }

    Status st = m_self_contact->getStatus();

    /* request away message if we are not online */
    if (st != STATUS_ONLINE) {
      want_auto_resp.emit(ev);
    }

    /* update the UINICQSubType */

    ust->setACK(true);

    if (ev->isDelivered()) {
      /* set accept-status of ACK */
      switch(st) {
      case STATUS_ONLINE:
	ust->setStatus(AcceptStatus_Online);
	break;
      case STATUS_AWAY:
	ust->setStatus(AcceptStatus_Away);
	break;
      case STATUS_NA:
	ust->setStatus(AcceptStatus_NA);
	break;
      case STATUS_OCCUPIED:
	ust->setStatus(AcceptStatus_Occ_Accept);
	break;
      default:
	ust->setStatus(AcceptStatus_Online);
      }
    } else {
      MessageEvent::DeliveryFailureReason r = ev->getDeliveryFailureReason();
      /* set accept-status of ACK */
      switch(r) {
      case MessageEvent::Failed_Denied:
	ust->setStatus(AcceptStatus_Denied);
	break;
      case MessageEvent::Failed_Occupied:
	ust->setStatus(AcceptStatus_Occupied);
	break;
      case MessageEvent::Failed_DND:
	ust->setStatus(AcceptStatus_DND);
	break;
      default:
	ust->setStatus(AcceptStatus_Denied);
      }
    }

    /* away message is always sent to someone if you are away */
    if (st != STATUS_ONLINE) {
      ust->setAwayMessage( ev->getAwayMessage() );
    } else {
      ust->setAwayMessage("");
    }

    delete ev;

    return true;
  }

  /*
   * This method handles:
   * - Setting up the UINICQSubType for Sending messages
   * (the actual message is sent by the caller -
   *  dependent on whether it is going direct/thru server)
   */
  UINICQSubType* MessageHandler::handleOutgoing(MessageEvent *ev)
  {
    UINICQSubType *icq = EventToUINICQSubType(ev);

    /* our status is sent in packets */
    icq->setStatus( Contact::MapStatusToICQStatus(m_self_contact->getStatus(), m_self_contact->isInvisible() ) );
    icq->setDestination( ev->getContact()->getUIN() );
    icq->setSource( m_self_contact->getUIN() );

    return icq;
  }

  void MessageHandler::handleIncomingACK(MessageEvent *ev, UINICQSubType *icq)
  {
    ICQMessageEvent *aev = dynamic_cast<ICQMessageEvent*>(ev);
    if (aev == NULL) return;
    
    aev->setAwayMessage( icq->getAwayMessage() );
    aev->setFinished(true);

    switch(icq->getStatus()) {
    case AcceptStatus_Online:
      aev->setDelivered(true);
      break;
    case AcceptStatus_Denied:
      aev->setDelivered(false);
      aev->setDeliveryFailureReason(MessageEvent::Failed_Denied);
      break;
    case AcceptStatus_Away:
      aev->setDelivered(true);
      aev->getContact()->setStatus(STATUS_AWAY);
      break;
    case AcceptStatus_Occupied:
      aev->setDelivered(false);
      aev->setDeliveryFailureReason(MessageEvent::Failed_Occupied);
      aev->getContact()->setStatus(STATUS_OCCUPIED);
      break;
    case AcceptStatus_DND:
      aev->setDelivered(false);
      aev->setDeliveryFailureReason(MessageEvent::Failed_DND);
      aev->getContact()->setStatus(STATUS_DND);
      break;
    case AcceptStatus_Occ_Accept:
      aev->setDelivered(true);
      aev->getContact()->setStatus(STATUS_OCCUPIED);
      break;
    case AcceptStatus_NA:
      aev->setDelivered(true);
      aev->getContact()->setStatus(STATUS_NA);
      break;
    default:
      {
	ostringstream ostr;
	ostr << "Unknown accept-status in ACK: " << icq->getStatus() << endl;
	SignalLog( LogEvent::WARN, ostr.str() );
      }
    }
    
    messageack.emit(ev);
  }
  
  /**
   *  Convert a UINICQSubType into an Event
   */
  ICQMessageEvent* MessageHandler::UINICQSubTypeToEvent(UINICQSubType *st, ContactRef contact)
  {
    ICQMessageEvent *e = NULL;
    unsigned short type = st->getType();
    
    switch(type) {

    case MSG_Type_Normal:
    {
      NormalICQSubType *nst = static_cast<NormalICQSubType*>(st);
      e = new NormalMessageEvent(contact,
				 nst->getMessage(), nst->isMultiParty() );
      break;
    }

    case MSG_Type_URL:
    {
      URLICQSubType *ust = static_cast<URLICQSubType*>(st);
      e = new URLMessageEvent(contact,
			      ust->getMessage(),
			      ust->getURL());
      break;
    }

    case MSG_Type_AuthReq:
    {
      AuthReqICQSubType *ust = static_cast<AuthReqICQSubType*>(st);
      e = new AuthReqEvent(contact, ust->getMessage());
      break;
    }

    case MSG_Type_AuthRej:
    {
      AuthRejICQSubType *ust = static_cast<AuthRejICQSubType*>(st);
      e = new AuthAckEvent(contact, ust->getMessage(), false);
      break;
    }

    case MSG_Type_AuthAcc:
    {
      e = new AuthAckEvent(contact, true);
      break;
    }

    case MSG_Type_AutoReq_Away:
    case MSG_Type_AutoReq_Occ:
    case MSG_Type_AutoReq_NA:
    case MSG_Type_AutoReq_DND:
    case MSG_Type_AutoReq_FFC:
    {
      e = new AwayMessageEvent(contact);
      break;
    }

    case MSG_Type_UserAdd:
    {
      e = new UserAddEvent(contact);
      break;
    }

    default:
      break;

    } // end of switch
    
    if (e != NULL) {
      e->setUrgent( st->isUrgent() );
      e->setToContactList( st->isToContactList() );
    }
    
    return e;
  }
  
  /**
   * Convert a MessageEvent into a UINICQSubType
   */
  UINICQSubType* MessageHandler::EventToUINICQSubType(MessageEvent *ev)
  {
    ContactRef c = ev->getContact();
    UINICQSubType *ist = NULL;

    if (ev->getType() == MessageEvent::Normal) {

      NormalMessageEvent *nv = static_cast<NormalMessageEvent*>(ev);
      ist = new NormalICQSubType(nv->getMessage());

    } else if (ev->getType() == MessageEvent::URL) {

      URLMessageEvent *uv = static_cast<URLMessageEvent*>(ev);
      ist = new URLICQSubType(uv->getMessage(), uv->getURL());

    } else if (ev->getType() == MessageEvent::AwayMessage) {

      ist = new AwayMsgSubType( c->getStatus() );

    } else if (ev->getType() == MessageEvent::AuthReq) {

      AuthReqEvent *uv = static_cast<AuthReqEvent*>(ev);
      ist = new AuthReqICQSubType(m_self_contact->getAlias(),
				  m_self_contact->getFirstName(),
				  m_self_contact->getLastName(),
				  m_self_contact->getEmail(),
				  m_self_contact->getAuthReq(),
				  uv->getMessage());

    } else if (ev->getType() == MessageEvent::AuthAck) {

      AuthAckEvent *uv = static_cast<AuthAckEvent*>(ev);
      if(uv->isGranted())
        ist = new AuthAccICQSubType();
      else
        ist = new AuthRejICQSubType(uv->getMessage());
    } else if (ev->getType() == MessageEvent::UserAdd) {

      ist = new UserAddICQSubType(m_self_contact->getAlias(),
				  m_self_contact->getFirstName(),
				  m_self_contact->getLastName(),
				  m_self_contact->getEmail(),
				  m_self_contact->getAuthReq());
    }
    
    ICQMessageEvent *iev;
    if (ist != NULL && (iev = dynamic_cast<ICQMessageEvent*>(ev)) != NULL) {
      ist->setUrgent( iev->isUrgent() );
      ist->setToContactList( iev->isToContactList() );
    }
    
    return ist;
  }

  void MessageHandler::SignalLog(LogEvent::LogType type, const string& msg) {
    LogEvent ev(type,msg);
    logger.emit(&ev);
  }


  
}

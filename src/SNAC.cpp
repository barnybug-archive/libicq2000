/*
 * SNACs
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
 */

#include "SNAC.h"

namespace ICQ2000 {

  InSNAC* ParseSNAC(Buffer& b) {
    unsigned short family, subtype;
    b >> family
      >> subtype;

    InSNAC *snac = NULL;

    switch(family) {

    case SNAC_FAM_GEN:
      switch(subtype) {
      case SNAC_GEN_ServerReady:
	snac = new ServerReadySNAC();
	break;
      case SNAC_GEN_RateInfo:
	snac = new RateInfoSNAC();
	break;
      case SNAC_GEN_CapAck:
	snac = new CapAckSNAC();
	break;
      case SNAC_GEN_UserInfo:
	snac = new UserInfoSNAC();
	break;
      case SNAC_GEN_MOTD:
	snac = new MOTDSNAC();
	break;
      case SNAC_GEN_RateInfoChange:
	snac = new RateInfoChangeSNAC();
	break;
      }
      break;

    case SNAC_FAM_BUD:
      switch(subtype) {
      case SNAC_BUD_Online:
	snac = new BuddyOnlineSNAC();
	break;
      case SNAC_BUD_Offline:
	snac = new BuddyOfflineSNAC();
	break;
      }
      break;

    case SNAC_FAM_MSG:
      switch(subtype) {
      case SNAC_MSG_Message:
	snac = new MessageSNAC();
	break;
      case SNAC_MSG_MessageACK:
	snac = new MessageACKSNAC();
	break;
      case SNAC_MSG_SentOffline:
	snac = new MessageSentOfflineSNAC();
	break;
      }
      break;

    case SNAC_FAM_SRV:
      switch(subtype) {
      case SNAC_SRV_Response:
	snac = new SrvResponseSNAC();
	break;
      }
      break;

    case SNAC_FAM_UIN:
      switch(subtype) {
      case SNAC_UIN_RequestError:
	snac = new UINRequestErrorSNAC();
	break;
      case SNAC_UIN_Response:
	snac = new UINResponseSNAC();
	break;
      }
      break;
    }
    
    if (snac == NULL) {
      // unrecognised SNAC
      // parse as a RawSNAC
      snac = new RawSNAC(family, subtype);
    }

    snac->Parse(b);

    return snac;

  }

}

/*
 * Constants
 * External constants that clients will use
 *
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
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

#ifndef CONSTANTS_H
#define CONSTANTS_H

enum Status {
  STATUS_ONLINE,
  STATUS_AWAY,
  STATUS_NA,
  STATUS_OCCUPIED,
  STATUS_DND,
  STATUS_FREEFORCHAT,
  STATUS_OFFLINE,
};

static const char* Status_text[] = { "Online",
				     "Away",
				     "N/A",
				     "Occupied",
				     "DND",
				     "Free for chat",
				     "Offline" };

static const unsigned int SMS_Max_Length = 160;

static const unsigned int String_Limit = 16384; // A sensible limit

#endif

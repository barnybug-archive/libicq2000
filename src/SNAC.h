/*
 * SNACs 
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

#ifndef SNAC_H
#define SNAC_H

#include "buffer.h"

#include "SNAC-base.h"
#include "SNAC-BUD.h"
#include "SNAC-LOC.h"
#include "SNAC-GEN.h"
#include "SNAC-UIN.h"
#include "SNAC-MSG.h"
#include "SNAC-SRV.h"

namespace ICQ2000 {
  InSNAC* ParseSNAC(Buffer& b);
}

#endif

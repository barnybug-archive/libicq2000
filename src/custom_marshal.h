/*
 * Custom Marshals for Signal handling
 * - I'd swear marshal is actually spelt marshall
 *   maybe it's a weird Americanism :-)
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

#ifndef CUSTOM_MARSHAL_H
#define CUSTOM_MARSHAL_H

class StopOnTrueMarshal {
 public:

  typedef bool InType;
  typedef bool OutType;
  OutType  ret_val;
  
  StopOnTrueMarshal() : ret_val(false) {}

  OutType value() { return ret_val; }
  static OutType default_value() { return false; }
  bool marshal(const InType& val) { ret_val = val; return val; }
  
};

#endif



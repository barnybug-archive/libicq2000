/*
 * Capabilities
 * Some handling of the mysterious capabilities byte strings
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

#ifndef CAPABILITIES_H
#define CAPABILITIES_H

#include <libicq2000/buffer.h>

#include <vector>

namespace ICQ2000 {

  class Capabilities {
   private:
    std::string m_data;

   public:
    Capabilities();
    
    void default_icq2000_capabilities();
    void default_icq2002_capabilities();
    void clear();

    void Parse(Buffer& b, unsigned short len);
    void Output(Buffer& b) const;
    void OutputFirst16(Buffer& b) const;

    unsigned short get_length() const;

    bool get_accept_adv_msgs() const;
  };

}

#endif

/*
 * SNAC - Visible/invisible list management
 * Mitz Pettel, 2001
 *
 * based on: SNAC - Buddy (Contact) list management
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

#ifndef SNAC_BOS_H
#define SNAC_BOS_H

#include <string>
#include <list>

#include <libicq2000/SNAC-base.h>
#include <libicq2000/Contact.h>
#include <libicq2000/ContactList.h>
#include <libicq2000/UserInfoBlock.h>

namespace ICQ2000 {

  // List stuff (Family 0x0009)
  const unsigned short SNAC_BOS_Add_Visible = 0x0005;
  const unsigned short SNAC_BOS_Remove_Visible = 0x0006;
  const unsigned short SNAC_BOS_Add_Invisible = 0x0007;
  const unsigned short SNAC_BOS_Remove_Invisible = 0x0008;

  // ----------------- Visible/invisible List (Family 0x0009) SNACs -----------

  class BOSFamilySNAC : virtual public SNAC {
   public:
    unsigned short Family() const { return SNAC_FAM_BOS; }
  };

  class AddVisibleSNAC : public BOSFamilySNAC, public OutSNAC {
   private:
    list<string> m_buddy_list;
    
   protected:
    void OutputBody(Buffer& b) const;

   public:
    AddVisibleSNAC();
    AddVisibleSNAC(const ContactList& l);
    AddVisibleSNAC(const Contact& c);

    void addVisible(const Contact& c);

    unsigned short Subtype() const { return SNAC_BOS_Add_Visible; }
  };

  class RemoveVisibleSNAC : public BOSFamilySNAC, public OutSNAC {
   private:
    list<string> m_buddy_list;
    
   protected:
    void OutputBody(Buffer& b) const;

   public:
    RemoveVisibleSNAC();
    RemoveVisibleSNAC(const ContactList& l);
    RemoveVisibleSNAC(const string& s);

    void removeVisible(const Contact& c);

    unsigned short Subtype() const { return SNAC_BOS_Remove_Visible; }
  };
  
  class AddInvisibleSNAC : public BOSFamilySNAC, public OutSNAC {
   private:
    list<string> m_buddy_list;
    
   protected:
    void OutputBody(Buffer& b) const;

   public:
    AddInvisibleSNAC();
    AddInvisibleSNAC(const ContactList& l);
    AddInvisibleSNAC(const Contact& c);

    void addInvisible(const Contact& c);

    unsigned short Subtype() const { return SNAC_BOS_Add_Invisible; }
  };

  class RemoveInvisibleSNAC : public BOSFamilySNAC, public OutSNAC {
   private:
    list<string> m_buddy_list;
    
   protected:
    void OutputBody(Buffer& b) const;

   public:
    RemoveInvisibleSNAC();
    RemoveInvisibleSNAC(const ContactList& l);
    RemoveInvisibleSNAC(const string& s);

    void removeInvisible(const Contact& c);

    unsigned short Subtype() const { return SNAC_BOS_Remove_Invisible; }
  };
  
}

#endif

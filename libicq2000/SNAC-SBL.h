/*
 * SNAC - Server-based list management
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

#ifndef SNAC_SBL_H
#define SNAC_SBL_H

#include <string>
#include <list>

#include <libicq2000/SNAC-base.h>
#include <libicq2000/Contact.h>
#include <libicq2000/ContactList.h>
#include <libicq2000/UserInfoBlock.h>

namespace ICQ2000 {

  // Server-based list stuff (Family 0x0013)
  const unsigned short SNAC_SBL_Request_List = 0x0005;
  const unsigned short SNAC_SBL_List_From_Server = 0x0006;
  const unsigned short SNAC_SBL_Edit_Start = 0x0011;
  const unsigned short SNAC_SBL_Add_Item = 0x0008;
  const unsigned short SNAC_SBL_Remove_Item = 0x000a;
  const unsigned short SNAC_SBL_Edit_Finish = 0x0012;
  const unsigned short SNAC_SBL_Edit_Request_Access = 0x0002;
  const unsigned short SNAC_SBL_Edit_Access_Granted = 0x0003;
  const unsigned short SNAC_SBL_Update_Group_Header = 0x0009;
  const unsigned short SNAC_SBL_Modification_Ack = 0x000e;

  // ----------------- Server-based Lists (Family 0x0013) SNACs -----------

  class SBLFamilySNAC : virtual public SNAC {
   public:
    unsigned short Family() const { return SNAC_FAM_SBL; }
  };

  class RequestSBLSNAC : public SBLFamilySNAC, public OutSNAC {
    
   protected:
    void OutputBody(Buffer& b) const;

   public:
    RequestSBLSNAC();

    unsigned short Subtype() const { return SNAC_SBL_Request_List; }
  };
  
  class SBLListSNAC : public SBLFamilySNAC, public InSNAC {
   private:
    ContactList m_contacts;
     
   protected:
    void ParseBody(Buffer& b);

   public:
    SBLListSNAC();
    
    const ContactList& getContactList() const { return m_contacts; }
    unsigned short Subtype() const { return SNAC_SBL_List_From_Server; }
  };
  
  class EditStartSBLSNAC : public SBLFamilySNAC, public OutSNAC {
   protected:
    void OutputBody(Buffer& b) const;

   public:
    EditStartSBLSNAC();

    unsigned short Subtype() const { return SNAC_SBL_Edit_Start; }
  };

  class AddItemSBLSNAC : public SBLFamilySNAC, public OutSNAC {
   private:
    std::string m_group_name;
    unsigned short m_group_id;
    std::list<ContactRef> m_buddy_list;
    
   protected:
    void OutputBody(Buffer& b) const;

   public:
    AddItemSBLSNAC();
    AddItemSBLSNAC(const ContactList& l);
    AddItemSBLSNAC(const ContactRef& c);
    AddItemSBLSNAC(const std::string &group_name, unsigned short group_id);

    void addBuddy(const ContactRef& c);

    unsigned short Subtype() const { return SNAC_SBL_Add_Item; }
  };

  class RemoveItemSBLSNAC : public SBLFamilySNAC, public OutSNAC {
   private:
    std::string m_group_name;
    unsigned short m_group_id;
    std::list<ContactRef> m_buddy_list;
    
   protected:
    void OutputBody(Buffer& b) const;

   public:
    RemoveItemSBLSNAC();
    RemoveItemSBLSNAC(const ContactList& l);
    RemoveItemSBLSNAC(const ContactRef& c);
    RemoveItemSBLSNAC(const std::string &group_name, unsigned short group_id);

    unsigned short Subtype() const { return SNAC_SBL_Remove_Item; }
  };

  class EditFinishSBLSNAC : public SBLFamilySNAC, public OutSNAC {
   protected:
    void OutputBody(Buffer& b) const;

   public:
    EditFinishSBLSNAC();

    unsigned short Subtype() const { return SNAC_SBL_Edit_Finish; }
  };

  class EditReqAccessSBLSNAC : public SBLFamilySNAC, public OutSNAC {
   protected:
    void OutputBody(Buffer& b) const;

   public:
    EditReqAccessSBLSNAC();

    unsigned short Subtype() const { return SNAC_SBL_Edit_Request_Access; }
  };

  class ModificationAckSBLSNAC : public SBLFamilySNAC, public InSNAC {
   public:
     enum Result {
       Success,
       Failed,
       AuthRequired,
       AlreadyExists
     };

   protected:
    std::vector<Result> m_results;
    void ParseBody(Buffer& b);

   public:
    ModificationAckSBLSNAC();

    std::vector<Result> getResults() const { return m_results; }
    unsigned short Subtype() const { return SNAC_SBL_Modification_Ack; }
  };

  class EditReqAccessGrantedSBLSNAC : public SBLFamilySNAC, public InSNAC {
   protected:
    void ParseBody(Buffer& b);

   public:
    EditReqAccessGrantedSBLSNAC();

    unsigned short Subtype() const { return SNAC_SBL_Edit_Access_Granted; }
  };

  class UpdateGroupSBLSNAC : public SBLFamilySNAC, public OutSNAC {
   private:
     std::string m_group_name;
     unsigned short m_group_id;
     std::vector<unsigned short> m_ids;

   protected:
    void OutputBody(Buffer& b) const;

   public:
    UpdateGroupSBLSNAC(const std::string &group_name,
      unsigned short group_id, const std::vector<unsigned short> &ids);

    unsigned short Subtype() const { return SNAC_SBL_Update_Group_Header; }
  };
  
}

#endif

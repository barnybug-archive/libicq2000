/*
 * SNAC - Server-based lists
 * Mitz Pettel, 2001
 *
 * based on: SNAC - Buddy List
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

#include <libicq2000/SNAC-SBL.h>
#include <libicq2000/TLV.h>

using std::string;

namespace ICQ2000 {

  // --------------- Server-based Lists (Family 0x0013) SNACs --------------

  RequestSBLSNAC::RequestSBLSNAC() { }

  void RequestSBLSNAC::OutputBody(Buffer& b) const {
      b << (unsigned int)(0);
      b << (unsigned short)(1);
    }

  SBLListSNAC::SBLListSNAC() { }
  
  void SBLListSNAC::ParseBody(Buffer& b) {
        
        b.advance(1);			// 00
	unsigned short entityCount, group_id, tag_id;
        b >> entityCount;		// number of entities?
        while (b.pos()<=b.size()-10)
        {
            unsigned short nameLength;
            b >> nameLength;
            string name;
            b.Unpack(name, nameLength);
	    b >> group_id;
	    b >> tag_id;
	    b.advance(2);
            unsigned short dataLength;
            b >> dataLength;
            while (dataLength>=2)
            {
                unsigned short infoType;
                unsigned short infoLength;
                b >> infoType;
                dataLength -= 2;
                b >> infoLength;
                dataLength -= 2;
                if (infoType==0x0131)	// UIN
                {
                    ContactRef c(new Contact(Contact::StringtoUIN(name)));
                    string nickname;
                    b.Unpack(nickname, infoLength);
                    dataLength -= infoLength;
                    c->setAlias(nickname);
		    c->setServerSideInfo(group_id, tag_id);
		    c->setServerBased(true);
                    m_contacts.add(c);
                    break;
                }
                else			// other stuff we don't understand
                {
                    b.advance(infoLength);
                    dataLength -= infoLength;
                }
            }
            b.advance(dataLength);
        }
        b.advance(4);
    }

  EditStartSBLSNAC::EditStartSBLSNAC() { }

  void EditStartSBLSNAC::OutputBody(Buffer& b) const {
    // empty
    }

  EditFinishSBLSNAC::EditFinishSBLSNAC() { }

  void EditFinishSBLSNAC::OutputBody(Buffer& b) const {
    // empty
    }

  AddItemSBLSNAC::AddItemSBLSNAC() { }

  AddItemSBLSNAC::AddItemSBLSNAC(const ContactList& l)
    : m_buddy_list(), m_group_id(0) { 
    ContactList::const_iterator curr = l.begin();
    while (curr != l.end()) {
      if ((*curr)->isICQContact())
      if (!(*curr)->getServerBased()) m_buddy_list.push_back(*curr);
      ++curr;
    }
  }

  AddItemSBLSNAC::AddItemSBLSNAC(const ContactRef& c)
    : m_buddy_list(1, c), m_group_id(0) { }

  AddItemSBLSNAC::AddItemSBLSNAC(const std::string &group_name, unsigned short group_id)
    : m_group_name(group_name), m_group_id(group_id) { }

  void AddItemSBLSNAC::addBuddy(const ContactRef& c) {
    m_buddy_list.push_back(c);
    }

  void AddItemSBLSNAC::OutputBody(Buffer& b) const {
    if(m_group_id) {
      b << (unsigned short) m_group_name.size();
      b.Pack(m_group_name);
      b << m_group_id;
      b << (unsigned short) 0x0000;
      b << (unsigned short) 0x0001;
      b << (unsigned short) 0x0000;

    } else {
      std::list<ContactRef>::const_iterator curr = m_buddy_list.begin();
      while (curr != m_buddy_list.end()) {
	std::string suin = (*curr)->getStringUIN();
	std::string alias = (*curr)->getAlias();

	b << (unsigned short) suin.size();
	b.Pack(suin);
	b << (unsigned short) (*curr)->getServerSideGroupID();
	b << (unsigned short) (*curr)->getServerSideID();
	b << (unsigned short) 0x0000;

	int tlvlen = 4 + alias.size();
	if((*curr)->getAuthAwait()) tlvlen += 4;

	b << (unsigned short) tlvlen;

	b << TLV_ContactNickname;
	b << (unsigned short) alias.size();
	b.Pack(alias);

	if((*curr)->getAuthAwait()) {
	    b << TLV_AuthAwaited;
	    b << (unsigned short) 0x0000;
	}

	++curr;
      }

    }
  }

  RemoveItemSBLSNAC::RemoveItemSBLSNAC() { }

  RemoveItemSBLSNAC::RemoveItemSBLSNAC(const ContactList& l)
    : m_buddy_list(), m_group_id(0) { 
    ContactList::const_iterator curr = l.begin();
    while (curr != l.end()) {
      if ((*curr)->isICQContact()) m_buddy_list.push_back(*curr);
      ++curr;
    }
  }

  RemoveItemSBLSNAC::RemoveItemSBLSNAC(const ContactRef& c)
    : m_buddy_list(1, c), m_group_id(0) { }

  RemoveItemSBLSNAC::RemoveItemSBLSNAC(const std::string &group_name, unsigned short group_id)
    : m_group_name(group_name), m_group_id(group_id) { }

  void RemoveItemSBLSNAC::OutputBody(Buffer& b) const {
    if(m_group_id) {
      b << (unsigned short) m_group_name.size();
      b.Pack(m_group_name);
      b << m_group_id;
      b << (unsigned short) 0x0000;
      b << (unsigned short) 0x0001;
      b << (unsigned short) 0x0000;

    } else {
      std::list<ContactRef>::const_iterator curr = m_buddy_list.begin();
      while (curr != m_buddy_list.end()) {
	std::string suin = (*curr)->getStringUIN();
	std::string alias = (*curr)->getAlias();

	b << (unsigned short) suin.size();
	b.Pack(suin);
	b << (unsigned short) (*curr)->getServerSideGroupID();
	b << (unsigned short) (*curr)->getServerSideID();
	b << (unsigned short) 0x0000;

	int tlvlen = 4 + alias.size();
	if((*curr)->getAuthAwait()) tlvlen += 4;

	b << (unsigned short) tlvlen;

	b << TLV_ContactNickname;
	b << (unsigned short) alias.size();
	b.Pack(alias);

	if((*curr)->getAuthAwait()) {
	    b << TLV_AuthAwaited;
	    b << (unsigned short) 0x0000;
	}

	++curr;
      }

    }
  }

  EditReqAccessSBLSNAC::EditReqAccessSBLSNAC() { }

  void EditReqAccessSBLSNAC::OutputBody(Buffer& b) const {
    // empty
    }

  EditReqAccessGrantedSBLSNAC::EditReqAccessGrantedSBLSNAC() { }
  
  void EditReqAccessGrantedSBLSNAC::ParseBody(Buffer& b) {
    // some mess is inside, isn't worth parsing
    }

  ModificationAckSBLSNAC::ModificationAckSBLSNAC() { }

  void ModificationAckSBLSNAC::ParseBody(Buffer& b) {
    unsigned short errcode;

    while(b.remains() >= 2) {
      b >> errcode;

      switch(errcode) {
        case 0x0000: m_results.push_back(Success); break;
        case 0x0003: m_results.push_back(AlreadyExists); break;
        case 0x000a: m_results.push_back(Failed); break;
        case 0x000e: m_results.push_back(AuthRequired); break;
      }
    }
  }

  UpdateGroupSBLSNAC::UpdateGroupSBLSNAC(const std::string &group_name,
  unsigned short group_id, const std::vector<unsigned short> &ids)
    : m_group_name(group_name), m_group_id(group_id), m_ids(ids) { }

  void UpdateGroupSBLSNAC::OutputBody(Buffer& b) const {
    b << (unsigned short) m_group_name.size();
    b.Pack(m_group_name);
    b << m_group_id;
    b << (unsigned short) 0x0000;
    b << (unsigned short) 0x0001;

    if(m_ids.empty()) {
      b << (unsigned short) 0x0000;

    } else {
      b << (unsigned short) (4 + m_ids.size()*2);
      b << (unsigned short) 0x00c8;
      b << (unsigned short) (m_ids.size()*2);
	// raw-coded TLV, found no suitable data structures in TLV.h

      std::vector<unsigned short>::const_iterator curr = m_ids.begin();
      while (curr != m_ids.end()) {
	b << (unsigned short) *curr;
	++curr;
      }

    }
  }

}

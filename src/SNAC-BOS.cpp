/*
 * SNAC - Visible/invisible lists
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

#include <libicq2000/SNAC-BOS.h>

#include <libicq2000/TLV.h>

namespace ICQ2000 {

  // --------------- Visible/invisible List (Family 0x0009) SNACs --------------

  AddVisibleSNAC::AddVisibleSNAC() { }

  AddVisibleSNAC::AddVisibleSNAC(const ContactList& l)
    : m_buddy_list() { 
    ContactList::const_iterator curr = l.begin();
    while (curr != l.end()) {
      if ((*curr).isICQContact()) m_buddy_list.push_back((*curr).getStringUIN());
      ++curr;
    }
    
  }

  AddVisibleSNAC::AddVisibleSNAC(const Contact& c)
    : m_buddy_list(1, c.getStringUIN()) { }

  void AddVisibleSNAC::addVisible(const Contact& c) {
    m_buddy_list.push_back(c.getStringUIN());
  }

  void AddVisibleSNAC::OutputBody(Buffer& b) const {
    list<string>::const_iterator curr = m_buddy_list.begin();
    while (curr != m_buddy_list.end()) {
      b << (unsigned char)(*curr).size();
      b.Pack(*curr);
      curr++;
    }
  }

  RemoveVisibleSNAC::RemoveVisibleSNAC() { }

  RemoveVisibleSNAC::RemoveVisibleSNAC(const ContactList& l)
    : m_buddy_list() { 
    ContactList::const_iterator curr = l.begin();
    while (curr != l.end()) {
      if ((*curr).isICQContact()) m_buddy_list.push_back((*curr).getStringUIN());
      ++curr;
    }
    
  }

  RemoveVisibleSNAC::RemoveVisibleSNAC(const string& s)
    : m_buddy_list(1, s) { }

  void RemoveVisibleSNAC::removeVisible(const Contact& c) {
    m_buddy_list.push_back(c.getStringUIN());
  }

  void RemoveVisibleSNAC::OutputBody(Buffer& b) const {
    list<string>::const_iterator curr = m_buddy_list.begin();
    while (curr != m_buddy_list.end()) {
      b << (unsigned char)(*curr).size();
      b.Pack(*curr);
      curr++;
    }
  }

  AddInvisibleSNAC::AddInvisibleSNAC() { }

  AddInvisibleSNAC::AddInvisibleSNAC(const ContactList& l)
    : m_buddy_list() { 
    ContactList::const_iterator curr = l.begin();
    while (curr != l.end()) {
      if ((*curr).isICQContact()) m_buddy_list.push_back((*curr).getStringUIN());
      ++curr;
    }
    
  }

  AddInvisibleSNAC::AddInvisibleSNAC(const Contact& c)
    : m_buddy_list(1, c.getStringUIN()) { }

  void AddInvisibleSNAC::addInvisible(const Contact& c) {
    m_buddy_list.push_back(c.getStringUIN());
  }

  void AddInvisibleSNAC::OutputBody(Buffer& b) const {
    list<string>::const_iterator curr = m_buddy_list.begin();
    while (curr != m_buddy_list.end()) {
      b << (unsigned char)(*curr).size();
      b.Pack(*curr);
      curr++;
    }
  }

  RemoveInvisibleSNAC::RemoveInvisibleSNAC() { }

  RemoveInvisibleSNAC::RemoveInvisibleSNAC(const ContactList& l)
    : m_buddy_list() { 
    ContactList::const_iterator curr = l.begin();
    while (curr != l.end()) {
      if ((*curr).isICQContact()) m_buddy_list.push_back((*curr).getStringUIN());
      ++curr;
    }
    
  }

  RemoveInvisibleSNAC::RemoveInvisibleSNAC(const string& s)
    : m_buddy_list(1, s) { }

  void RemoveInvisibleSNAC::removeInvisible(const Contact& c) {
    m_buddy_list.push_back(c.getStringUIN());
  }

  void RemoveInvisibleSNAC::OutputBody(Buffer& b) const {
    list<string>::const_iterator curr = m_buddy_list.begin();
    while (curr != m_buddy_list.end()) {
      b << (unsigned char)(*curr).size();
      b.Pack(*curr);
      curr++;
    }
  }

}

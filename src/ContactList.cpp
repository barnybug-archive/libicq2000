/*
 * ContactList
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

#include "ContactList.h"

namespace ICQ2000 {

  ContactList::ContactList() { }

  Contact& ContactList::operator[](unsigned int uin) {
    return m_cmap[uin];
  }

  Contact& ContactList::operator[](const string& m) {
    iterator curr = begin();
    while (curr != end()) {
      if ((*curr).getMobileNo() == m) return (*curr);
      ++curr;
    }

    // really shouldn't reach here
    Contact c(m,m);
    add(c);
    return m_cmap[c.getUIN()];
  }

  Contact& ContactList::add(const Contact& ct) {
    m_cmap.insert(std::pair<unsigned int,Contact>(ct.getUIN(),ct));
    return m_cmap[ct.getUIN()];
  }

  void ContactList::remove(unsigned int uin) {
    m_cmap.erase(uin);
  }

  bool ContactList::empty() {
    return m_cmap.empty();
  }

  bool ContactList::exists(unsigned int uin) {
    return (m_cmap.count(uin) != 0);
  }
  
  bool ContactList::exists(const string& m) {
    iterator curr = begin();
    while (curr != end()) {
      if ((*curr).getMobileNo() == m) return true;
      ++curr;
    }
    return false;
  }

  ContactList::iterator ContactList::begin() {
    return iterator(m_cmap.begin());
  }

  ContactList::const_iterator ContactList::begin() const {
    return const_iterator(m_cmap.begin());
  }

  ContactList::iterator ContactList::end() {
    return iterator(m_cmap.end());
  }

  ContactList::const_iterator ContactList::end() const {
    return const_iterator(m_cmap.end());
  }

}  

/*
 * ContactList
 *
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>
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

#include <libicq2000/ContactList.h>

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

  bool ContactList::empty() const {
    return m_cmap.empty();
  }

  unsigned int ContactList::size() const {
    return m_cmap.size();
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

/*
 * ContactList (model)
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

#ifndef CONTACTLIST_H
#define CONTACTLIST_H

#include <list>
#include <string>

#include <config.h>

#ifdef HAVE_EXT_HASH_MAP
# include <ext/hash_map>
#elif HAVE_HASH_MAP
# include <hash_map>
#else
# error "hash_map not defined"
#endif

#include "Contact.h"

using std::list;
using std::string;
using std::hash_map;

namespace ICQ2000 {

  class _ContactList_iterator {
  private:
    hash_map<unsigned int,Contact>::iterator iter;
  
  public:
    _ContactList_iterator(hash_map<unsigned int,Contact>::iterator i)
      : iter(i) { }
  
    _ContactList_iterator& operator++() { ++iter; return *this; }
    _ContactList_iterator operator++(int) { return _ContactList_iterator(iter++); }
    bool operator==(const _ContactList_iterator& x) const { return iter == x.iter; }
    bool operator!=(const _ContactList_iterator& x) const { return iter != x.iter; }
    Contact& operator*() const { return (*iter).second; }
  };

  class _ContactList_const_iterator {
  private:
    hash_map<unsigned int,Contact>::const_iterator iter;
  
  public:
    _ContactList_const_iterator(hash_map<unsigned int,Contact>::const_iterator i)
      : iter(i) { }
  
    _ContactList_const_iterator& operator++() { ++iter; return *this; }
    _ContactList_const_iterator operator++(int) { return _ContactList_const_iterator(iter++); }
    bool operator==(const _ContactList_const_iterator& x) const { return iter == x.iter; }
    bool operator!=(const _ContactList_const_iterator& x) const { return iter != x.iter; }
    const Contact& operator*() const { return (*iter).second; }
  };

  class ContactList {
  private:
    hash_map<unsigned int,Contact> m_cmap;

    /* Mobile contacts are implemented as
     * Contact's and should still have UINs.
     * Purely Mobile contacts will have imaginary UINs
     * (ones counting down from -1), this is the best I could
     * do to keep this consistent across board.
     *
     * It would be nice to have a hash off mobile# to contact
     * but this was proving tricky to ensure consistency.
     */


  public:
    typedef _ContactList_iterator iterator;
    typedef _ContactList_const_iterator const_iterator;

    ContactList();

    Contact& operator[](unsigned int uin);
    Contact& operator[](const string& m);
    Contact& add(const Contact& ct);
    void remove(unsigned int uin);

    bool empty();

    bool exists(unsigned int uin);
    bool exists(const string& m);

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
  };

}

#endif

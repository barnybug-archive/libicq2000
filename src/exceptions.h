/*
 * Exceptions
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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>
#include <exception>

using std::string;
using std::exception;

namespace ICQ2000 {

  class ParseException : exception {
   private:
    string m_errortext;
    
   public:
    ParseException(const string& text);
    ~ParseException() throw() { }

    const char* what() const throw();
  };
  
}

#endif

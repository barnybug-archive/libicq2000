/*
 * Translate class
 *
 * Many parts of this source code were 'inspired' by the ircII4.4 translat.c source.
 * RIPPED FROM KVirc: http://www.kvirc.org
 * Original by Szymon Stefanek (kvirc@tin.it).
 * Modified by Andrew Frolov (dron@linuxer.net)
 * Further modified by Graham Roff
 *
 * 'Borrowed' from licq - thanks Barnaby
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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <string>
#include <exception>

using std::exception;
using std::string;

namespace ICQ2000 {
  class TranslatorException : exception {
   private:
    string m_errortext;
    
   public:
    TranslatorException(const string& text);
    ~TranslatorException() throw() { }
    
    const char* what() const throw();
  };

  class Translator{
   public:
    Translator();
    void setDefaultTranslationMap();
    void setTranslationMap(const string& szMapFileName);
    void ServerToClient(string& szString);
    void ClientToServer(string& szString);
    void ServerToClient(char &_cChar);
    void ClientToServer(char &_cChar);
    static void CRLFtoLF(string& s);
    static void LFtoCRLF(string& s);
    bool usingDefaultMap()  { return m_bDefault; }
    const string& getMapFileName() { return m_szMapFileName; }
    const string& getMapName() { return m_szMapName; }

   protected:
    unsigned char serverToClientTab[256];
    unsigned char clientToServerTab[256];
    string m_szMapFileName, m_szMapName;
    bool m_bDefault;
  };
}

#endif

/*
 * This file is one big nasty kludge because the older
 * libstdc++ libraries (before v3) have the old strstream
 * but I want to use the new, and much improved sstream.
 * - Barnaby
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_SSTREAM
# include <sstream>
#elif HAVE_STRSTREAM
# define USE_STRSTREAM_WRAPPERS
#else
# error "No sstream/strstream implementation"
#endif

#ifdef USE_STRSTREAM_WRAPPERS

#include <string>
#include <strstream>

namespace std
{

  /*
   * Only limited functionality from ostringstream
   * is implemented
   */
  class ostringstream : public ostrstream {
   public:
    string str() {
      char *cstr = ostrstream::str();
      freeze(false);
      if (cstr == 0) return string();
      return string(cstr,pcount());
    }
  };

  /*
   * Only limited functionality from istringstream
   * is implemented
   */
  class istringstream : public istrstream {
   public:
    istringstream(const string& str)
      : istrstream(str.c_str()) { }
  };

}

#endif

##
## Process with automake to produce Makefile.in
##

INCLUDES = @SIGC_CFLAGS@
LDFLAGS = @SIGC_LIBS@

lib_LTLIBRARIES = libicq2000.la

libicq2000_la_SOURCES = Client.cpp buffer.cpp socket.cpp TLV.cpp \
    SNAC.cpp SNAC-base.cpp SNAC-BUD.cpp SNAC-LOC.cpp SNAC-GEN.cpp SNAC-UIN.cpp SNAC-MSG.cpp SNAC-SRV.cpp \
    events.cpp Xml.cpp Contact.cpp ContactList.cpp exceptions.cpp \
    DirectClient.cpp ICQ.cpp UserInfoBlock.cpp ICBMCookie.cpp Translator.cpp

pkginclude_HEADERS = Client.h buffer.h socket.h TLV.h constants.h \
    SNAC.h SNAC-base.h SNAC-BUD.h SNAC-LOC.h SNAC-GEN.h SNAC-UIN.h SNAC-MSG.h SNAC-SRV.h \
    events.h Xml.h Contact.h ContactList.h exceptions.h custom_marshal.h \
    DirectClient.h ICQ.h UserInfoBlock.h ICBMCookie.h userinfoconstants.h \
    Translator.h Cache.h RequestIDCache.h ICBMCookieCache.h \
    SeqNumCache.h DCCache.h sstream_fix.h

libicq2000_la_LDFLAGS = -version-info 2:0:0


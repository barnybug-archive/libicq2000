##
## Process with automake to produce Makefile.in
##

INCLUDES = @SIGC_CFLAGS@
LDFLAGS = @SIGC_LIBS@

lib_LTLIBRARIES = libicq2000.la

libicq2000_la_SOURCES = Client.cpp buffer.cpp socket.cpp TLV.cpp	\
SNAC.cpp SNAC-base.cpp SNAC-BUD.cpp SNAC-LOC.cpp SNAC-GEN.cpp		\
SNAC-UIN.cpp SNAC-MSG.cpp SNAC-SRV.cpp SNAC-SBL.cpp SNAC-BOS.cpp	\
version.cpp events.cpp Xml.cpp Contact.cpp ContactList.cpp		\
exceptions.cpp DirectClient.cpp ICQ.cpp UserInfoBlock.cpp		\
ICBMCookie.cpp Translator.cpp userinfohelpers.cpp SMTPClient.cpp	\
SocketClient.cpp time_extra.cpp

noinst_HEADERS = sstream_fix.h

libicq2000_la_LDFLAGS = -version-info @LIBICQ2000_SO_VERSION@


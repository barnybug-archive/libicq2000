##
## Process with automake to produce Makefile.in
##

# Headers are installed in ${pkgincludedir}/libicq2000, this will allow
# multiple versions of the library to be installed in future without
# their headers overlapping (by varying the pkgincludedir with
# versioning information). Also it means a client compiling with the
# library will have to explicitly use the -I flag for the library
# headers it needs, guaranteeing it knows what it's getting, typically
# this will be got from the libicq2000-config script.

headersincludedir = $(pkgincludedir)/libicq2000

headersinclude_HEADERS = Client.h buffer.h socket.h TLV.h constants.h				\
    SNAC.h SNAC-base.h SNAC-BUD.h SNAC-LOC.h SNAC-GEN.h SNAC-UIN.h SNAC-MSG.h SNAC-SRV.h	\
    SNAC-SBL.h SNAC-BOS.h                                                                       \
    events.h Xml.h Contact.h ContactList.h exceptions.h custom_marshal.h                        \
    DirectClient.h ICQ.h UserInfoBlock.h ICBMCookie.h userinfoconstants.h                       \
    Translator.h Cache.h RequestIDCache.h ICBMCookieCache.h                                     \
    SeqNumCache.h DCCache.h

!	Make WorldWideWeb line mode hypertext browser under VMS
!       =======================================================
!
! History:
!  14 Aug 91 (TBL)	Reconstituted
!  25 Jun 92 (JFG)	Added TCP socket emulation over DECnet
!
! Bugs:
!	The dependencies are anything but complete - they were
!	just enough to allow the files to be compiled.
!
! Instructions:
!	Use the correct command line for your TCP/IP implementation:
!
!	$ MMS/MACRO=(MULTINET=1)	for Multinet
!	$ MMS/MACRO=(WIN_TCP=1)		for Wollongong TCP/IP
!	$ MMS/MACRO=(UCX=1)		for DEC/UCX
!	$ MMS/MACRO=(DECNET=1)		for socket emulation over DECnet
!
! If you are on HEP net and want to build using the really latest sources on
! PRIAM:: then define an extra macro U=PRIAM::, e.g.
!
!	$ MMS/MACRO=(MULTINET=1, U=PRIAM::)	for Multinet
!
! This will copy the sources from PRIAM as necessary. You can also try
!
!	$ MMS/MACRO=(U=PRIAM::) descrip.mms
!
! to update this file.
!
.IFDEF UCX
LIBS = sys$library:ucx$ipc/lib		! For UCX
OPTION_FILE = 
CFLAGS = /DEBUG/DEFINE=DEBUG
TCP=UCX
.ENDIF
.IFDEF MULTINET
LIBS = multinet.opt/opt			! For Multinet
OPTION_FILE = multinet.opt
CFLAGS = /DEFINE=(DEBUG,MULTINET)
TCP=MULTINET
.ENDIF
.IFDEF WIN_TCP
LIBS = win_tcp.opt/opt			! For Wollongong TCP
OPTION_FILE = win_tcp.opt
CFLAGS = /DEFINE=(DEBUG,WIN_TCP)
TCP=WIN_TCP
.ENDIF
.IFDEF DECNET
LIBS =  disk$c3:[hemmer.unix.usr.lib]libc.opt/opt	! TCP socket library over DECnet
OPTION_FILE = disk$c3:[hemmer.unix.usr.lib]libc.opt
CFLAGS = /DEFINE=(DEBUG,DECNET)
TCP=DECNET
.ENDIF

.IFDEF LIBS
.ELSE
LIBS = multinet.opt/opt			! (Default to multinet)
OPTION_FILE = multinet.opt
CFLAGS = /DEFINE=(DEBUG,MULTINET)
.ENDIF

.INCLUDE Version.make

.IFDEF DECNET  ! Strip FTP, Gopher and News
OBJECTS = HTAtom.obj, HTChunk.obj, HTList.obj, HTString.obj, -
	HTAccess.obj, HTAnchor.obj, HTFile.obj, HTFormat.obj, -
	HTHistory.obj, HTML.obj, -
	HTParse.obj, HTStyle.obj, HTTCP.obj, HTTP.obj, SGML.obj

MODULES = HTAtom, HTChunk, HTList, HTString, -
	HTAccess, HTAnchor, HTFile, HTFormat, -
	HTHistory, HTML, -
	HTParse, HTStyle, HTTCP, HTTP, SGML

OBJECTS_D = HTAtom_d.obj, HTChunk_d.obj, HTList_d.obj, HTString_d.obj, -
	HTAccess_d.obj, HTAnchor_d.obj, HTFile_d.obj, HTFormat_d.obj, -
	HTHistory_d.obj, HTML_d.obj, -
	HTParse_d.obj, HTStyle_d.obj, HTTCP_d.obj, HTTP_d.obj, SGML_d.obj
.ELSE
OBJECTS = HTAtom.obj, HTChunk.obj, HTList.obj, HTString.obj, -
	HTAccess.obj, HTAnchor.obj, HTFile.obj, HTFormat.obj, HTFTP.obj, -
	HTGopher.obj, HTHistory.obj, HTNews.obj, HTML.obj, -
	HTParse.obj, HTStyle.obj, HTTCP.obj, HTTP.obj, SGML.obj

MODULES = HTAtom, HTChunk, HTList, HTString, -
	HTAccess, HTAnchor, HTFile, HTFormat, -
	HTHistory, HTML, -
	HTParse, HTStyle, HTTCP, HTTP, SGML, HTFTP, HTGopher, HTNews

OBJECTS_D = HTAtom_d.obj, HTChunk_d.obj, HTList_d.obj, HTString_d.obj, -
	HTAccess_d.obj, HTAnchor_d.obj, HTFile_d.obj, HTFormat_d.obj, HTFTP_d.obj, -
	HTGopher_d.obj, HTHistory_d.obj, HTNews_d.obj, HTML_d.obj, -
	HTParse_d.obj, HTStyle_d.obj, HTTCP_d.obj, HTTP_d.obj, SGML_d.obj, -
	HTBrowse_d.obj, GridText_d.obj, DefaultStyles_d.obj
.ENDIF

!___________________________________________________________________
! WWW Library

lib : wwwlib_$(TCP)($(MODULES)) build_$(TCP).com

build_$(TCP).com : descrip.mms
	mms/from_sources/out=build_$(TCP).com/macro=($(tcp)=1) 
!___________________________________________________________________
! BASIC modules

!_____________________________	HTAtom

HTAtom.obj   : HTAtom.c HTAtom.h HTUtils.h HTString.h
        cc $(CFLAGS)/obj=$*.obj HTAtom.c
HTAtom_d.obj : HTAtom.c HTAtom.h HTUtils.h
	cc/debug $(CFLAGS)/obj=$*.obj HTAtom.c
.IFDEF U
HTAtom.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAtom.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAtom.c" - 
             HTAtom.c
HTAtom.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAtom.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAtom.h" -
             HTAtom.h
.ENDIF
!_____________________________	HTChunk

HTChunk.obj   : HTChunk.c HTChunk.h HTUtils.h
        cc $(CFLAGS)/obj=$*.obj HTChunk.c
HTChunk_d.obj : HTChunk.c HTChunk.h
	cc/debug $(CFLAGS)/obj=$*.obj HTChunk.c
.IFDEF U
HTChunk.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTChunk.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTChunk.c" - 
             HTChunk.c
HTChunk.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTChunk.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTChunk.h" -
             HTChunk.h
.ENDIF
!_____________________________	HTList

HTList.obj   : HTList.c HTList.h
        cc $(CFLAGS)/obj=$*.obj HTList.c
HTList_d.obj : HTList.c HTList.h
	cc/debug $(CFLAGS)/obj=$*.obj HTList.c
.IFDEF U
HTList.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTList.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTList.c" - 
             HTList.c
HTList.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTList.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTList.h" -
             HTList.h
.ENDIF
!_____________________________	HTString

HTString.obj   : HTString.c HTString.h tcp.h Version.make
        cc $(CFLAGS)/obj=$*.obj /define=(VC="""$(VC)""") HTString.c
HTString_d.obj : HTString.c HTString.h tcp.h
	cc/debug $(CFLAGS)/obj=$*.obj/define=(VC="""$(VC)""") HTString.c
.IFDEF U
HTString.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTString.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTString.c" - 
             HTString.c
HTString.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTString.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTString.h" -
             HTString.h
.ENDIF

!    C O M M O N	M O D U L E S

!_____________________________	HTAccess

HTAccess.obj   : HTAccess.c HTAccess.h HTParse.h HTUtils.h WWW.h -
	HTFile.h HTNews.h HTGopher.h HTTP.h HTFTP.h -
	HTAnchor.h  HTFormat.h HTStyle.h HText.h
        cc $(CFLAGS)/obj=$*.obj HTAccess.c
HTAccess_d.obj : HTAccess.c HTAccess.h HTParse.h HTUtils.h WWW.h -
	HTFile.h HTNews.h HTGopher.h HTTP.h HTFTP.h -
	HTAnchor.h  HTFormat.h HTStyle.h HText.h
	cc/debug $(CFLAGS)/obj=$*.obj HTAccess.c
.IFDEF U
HTAccess.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAccess.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAccess.c" - 
             HTAccess.c
HTAccess.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAccess.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAccess.h" -
             HTAccess.h
.ENDIF
!_____________________________	HTAnchor

HTAnchor.obj   : HTAnchor.c HTAnchor.h HTUtils.h
        cc $(CFLAGS)/obj=$*.obj HTAnchor.c
HTAnchor_d.obj : HTAnchor.c HTAnchor.h HTUtils.h
	cc/debug $(CFLAGS)/obj=$*.obj HTAnchor.c
.IFDEF U
HTAnchor.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAnchor.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAnchor.c" - 
             HTAnchor.c
HTAnchor.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAnchor.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTAnchor.h" -
             HTAnchor.h
.ENDIF

!_________________________________ HTFile

.IFDEF U
HTFile.c   : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFile.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFile.c" - 
             HTFile.c
HTFile.h   : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFile.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFile.h" -
             HTFile.h
.ENDIF
HTFile.obj   : HTFile.c HTFile.h HTUtils.h WWW.h HTParse.h tcp.h HTAnchor.h - 
               HTTCP.h HTFTP.h 
         cc $(CFLAGS)/obj=$*.obj HTFile.c
HTFile_d.obj : HTFile.c HTFile.h HTUtils.h WWW.h HTParse.h tcp.h -
               HTTCP.h HTFTP.h
         cc/debug $(CFLAGS)/obj=$*.obj HTFile.c

!_____________________________	HTFormat

HTFormat.obj   : HTFormat.c HTFormat.h HTUtils.h HTML.h SGML.h
        cc $(CFLAGS)/obj=$*.obj HTFormat.c
HTFormat_d.obj : HTFormat.c HTFormat.h HTUtils.h HTML.h SGML.h
	cc/debug $(CFLAGS)/obj=$*.obj HTFormat.c
.IFDEF U
HTFormat.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFormat.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFormat.c" - 
             HTFormat.c
HTFormat.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFormat.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFormat.h" -
             HTFormat.h
.ENDIF
!__________________________________ HTFTP
.IFDEF DECNET
.ELSE
HTFTP.obj   : HTFTP.c HTFTP.h HTParse.h HTUtils.h tcp.h HTTCP.h
        cc $(CFLAGS)/obj=$*.obj HTFTP.c
HTFTP_d.obj : HTFTP.c HTFTP.h HTParse.h HTUtils.h tcp.h HTTCP.h 
        cc/debug $(CFLAGS)/obj=$*.obj HTFTP.c
.IFDEF U
HTFTP.c    : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFTP.c"
             copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFTP.c" -
             HTFTP.c
HTFTP.h    : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFTP.h"
             copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTFTP.h" - 
             HTFTP.h
.ENDIF
.ENDIF

!_____________________________	HTGopher
.IFDEF DECNET
.ELSE
HTGopher.obj   : HTGopher.c HTGopher.h HTUtils.h
        cc $(CFLAGS)/obj=$*.obj HTGopher.c
HTGopher_d.obj : HTGopher.c HTGopher.h HTUtils.h
	cc/debug $(CFLAGS)/obj=$*.obj HTGopher.c
.IFDEF U
HTGopher.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTGopher.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTGopher.c" - 
             HTGopher.c
HTGopher.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTGopher.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTGopher.h" -
             HTGopher.h
.ENDIF
.ENDIF
!_____________________________	HTHistory

HTHistory.obj   : HTHistory.c HTHistory.h HTUtils.h
        cc $(CFLAGS)/obj=$*.obj HTHistory.c
HTHistory_d.obj : HTHistory.c HTHistory.h HTUtils.h
	cc/debug $(CFLAGS)/obj=$*.obj HTHistory.c
.IFDEF U
HTHistory.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTHistory.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTHistory.c" - 
             HTHistory.c
HTHistory.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTHistory.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTHistory.h" -
             HTHistory.h
.ENDIF
!_____________________________	HTNews
.IFDEF DECNET
.ELSE
HTNews.obj   : HTNews.c HTNews.h HTUtils.h
        cc $(CFLAGS)/obj=$*.obj HTNews.c
HTNews_d.obj : HTNews.c HTNews.h HTUtils.h
	cc/debug $(CFLAGS)/obj=$*.obj HTNews.c
.IFDEF U
HTNews.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTNews.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTNews.c" - 
             HTNews.c
HTNews.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTNews.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTNews.h" -
             HTNews.h
.ENDIF
.ENDIF
!_____________________________	HTML

HTML.obj   : HTML.c HTML.h HTUtils.h SGML.h
        cc $(CFLAGS)/obj=$*.obj HTML.c
HTML_d.obj : HTML.c HTML.h HTUtils.h SGML.h
	cc/debug $(CFLAGS)/obj=$*.obj HTML.c
.IFDEF U
HTML.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTML.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTML.c" - 
             HTML.c
HTML.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTML.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTML.h" -
             HTML.h
.ENDIF
!________________________________ HTParse

.IFDEF U
HTParse.c  : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTParse.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTParse.c" - 
             HTParse.c
HTParse.h  : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTParse.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTParse.h" -
             HTParse.h
.ENDIF
HTParse.obj   : HTParse.c HTParse.h HTUtils.h tcp.h
        cc $(CFLAGS)/obj=$*.obj HTParse.c
HTParse_d.obj : HTParse.c HTParse.h HTUtils.h tcp.h
	cc/debug $(CFLAGS)/obj=$*.obj HTParse.c

!_____________________________	HTStyle

HTStyle.obj   : HTStyle.c HTStyle.h HTUtils.h 
        cc $(CFLAGS)/obj=$*.obj HTStyle.c
HTStyle_d.obj : HTStyle.c HTStyle.h HTUtils.h 
	cc/debug $(CFLAGS)/obj=$*.obj HTStyle.c
.IFDEF U
HTStyle.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTStyle.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTStyle.c" - 
             HTStyle.c
HTStyle.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTStyle.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTStyle.h" -
             HTStyle.h
.ENDIF
! _________________________________ HTTCP

HTTCP.obj : HTTCP.c HTTCP.h HTUtils.h tcp.h
         cc $(CFLAGS)/obj=$*.obj HTTCP.c
HTTCP_d.obj : HTTCP.c HTTCP.h HTUtils.h tcp.h
         cc/debug $(CFLAGS)/obj=$*.obj HTTCP.c
.IFDEF U
HTTCP.c    : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTTCP.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTTCP.c" - 
             HTTCP.c
HTTCP.h    : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTTCP.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTTCP.h" - 
             HTTCP.h
.ENDIF
!________________________________ HTTP

HTTP.obj   : HTTP.c HTTP.h HTParse.h HTUtils.h tcp.h
        cc $(CFLAGS)/obj=$*.obj HTTP.c
HTTP_d.obj : HTTP.c HTTP.h HTParse.h HTUtils.h tcp.h
	cc/debug $(CFLAGS)/obj=$*.obj HTTP.c
.IFDEF U
HTTP.c     : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTTP.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTTP.c" -
             HTTP.c
HTTP.h     : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTTP.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTTP.h" -
             HTTP.h 
.ENDIF
!_____________________________	SGML

SGML.obj   : SGML.c SGML.h HTUtils.h
        cc $(CFLAGS)/obj=$*.obj SGML.c
SGML_d.obj : SGML.c SGML.h HTUtils.h
	cc/debug $(CFLAGS)/obj=$*.obj SGML.c
.IFDEF U
SGML.c : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/SGML.c"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/SGML.c" - 
             SGML.c
SGML.h : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/SGML.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/SGML.h" -
             SGML.h
.ENDIF
!_________________________________ include files only:

.IFDEF U
HTUtils.h  : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTUtils.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HTUtils.h" -
             HTUtils.h
tcp.h      : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/tcp.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/tcp.h" - 
             tcp.h
HText.h      : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HText.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/HText.h" - 
             HText.h
README      : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/README"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/README" - 
             README
WWW.h      : $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/WWW.h"
	     copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/WWW.h" -
             WWW.h
.ENDIF
 
! ______________________________  The version file

.IFDEF U
Version.make :  $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/Version.make"
	copy $(U)"/userd/tbl/hypertext/WWW/Library/Implementation/Version.make" - 
             Version.make
	write sys$output: "Please rebuild with new Version file"
	exit 2	! Error
.ENDIF

! _____________________________VMS SPECIAL FILES:
! latest version of this one:

.IFDEF U
descrip.mms : $(U)"/userd/tbl/hypertext/WWW/Library/vms/descrip.mms"
	copy $(U)"/userd/tbl/hypertext/WWW/Library/vms/descrip.mms" -
	descrip.mms
	write sys$output: "Please rebuild with new MMS file"
	exit 2	! Error

.ENDIF



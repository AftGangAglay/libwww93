/*                                                      System dependencies in the W3 library
                                   SYSTEM DEPENDENCIES
                                             
   System-system differences for TCP include files and macros. This file includes for each
   system the files necessary for network and file I/O.
   
  AUTHORS
  
  TBL                     Tim Berners-Lee, W3 project, CERN, <timbl@info.cern.ch>
                         
  EvA                     Eelco van Asperen <evas@cs.few.eur.nl>
                         
  MA                      Marc Andreesen NCSA
                         
  AT                      Aleksandar Totic <atotic@ncsa.uiuc.edu>
                         
  SCW                     Susan C. Weber <sweber@kyle.eitech.com>
                         
  HISTORY:
  
  22 Feb 91               Written (TBL) as part of the WWW library.
                         
  16 Jan 92               PC code from EvA
                         
  22 Apr 93               Merged diffs bits from xmosaic release
                         
  29 Apr 93               Windows/NT code from SCW
                         
 */

#ifndef TCP_H
#define TCP_H

/*

Default values

   These values may be reset and altered by system-specific sections later on.  there are
   also a bunch of defaults at the end .
   
 */
/* Default values of those: */
#define NETCLOSE close      /* Routine to close a TCP-IP socket         */
#define NETREAD  read       /* Routine to read from a TCP-IP socket     */
#define NETWRITE write      /* Routine to write to a TCP-IP socket      */

/* Unless stated otherwise, */
#define SELECT                  /* Can handle >1 channel.               */

#ifdef unix
#define GOT_PIPE
#endif

typedef struct sockaddr_in SockA;  /* See netinet/in.h */

#include <string.h>             /* For bzero etc - not  VM */

/*

IBM-PC running Windows NT

   These parameters provided by  Susan C. Weber <sweber@kyle.eitech.com>.
   
 */
#ifdef _WIN32

#include "fcntl.h"                      /* For HTFile.c */
#include "sys/types.h"                  /* For HTFile.c */
#include "sys/stat.h"                   /* For HTFile.c */

#undef NETREAD
#undef NETWRITE
#undef NETCLOSE
#define NETREAD(s, b, l)  ((s)>10 ? recv((s),(b),(l),0) : read((s),(b),(l)))
#define NETWRITE(s, b, l) ((s)>10 ? send((s),(b),(l),0) : write((s),(b),(l)))
#define NETCLOSE(s)     ((s)>10 ? closesocket(s) : close(s))

#include <io.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <direct.h>
#include <stdio.h>
#include <winsock.h>

#define INCLUDES_DONE
#define TCP_INCLUDES_DONE
#endif  /* WINDOWS */

/*

Regular BSD unix versions

   These are a default unix where not already defined specifically.
   
 */
#ifndef INCLUDES_DONE
#include <sys/types.h>
/* #include <streams/streams.h>                 not ultrix */
#include <string.h>

#include <errno.h>          /* independent */
#include <sys/stat.h>
#ifndef _MSC_VER
#include <sys/time.h>       /* independent */
#include <sys/param.h>
#include <sys/file.h>       /* For open() etc */
#endif
#define INCLUDES_DONE
#endif  /* Normal includes */

/*                      Directory reading stuff - BSD or SYS V
*/
#ifdef unix                    /* if this is to compile on a UNIX machine */
#define GOT_READ_DIR 1    /* if directory reading functions are available */
#ifdef USE_DIRENT             /* sys v version */
#include <dirent.h>
#define direct dirent
#else
#include <sys/dir.h>
#endif
#if defined(sun) && defined(__svr4__)
#include <sys/fcntl.h>
#include <limits.h>
#endif
#endif

/*

Defaults

  INCLUDE FILES FOR TCP
  
 */
#ifndef TCP_INCLUDES_DONE
#include <sys/socket.h>
#include <netinet/in.h>
#ifndef __hpux /* this may or may not be good -marc */
#include <arpa/inet.h>      /* Must be after netinet/in.h */
#endif
#include <netdb.h>
#endif  /* TCP includes */

/*

  CACHE FILE PREFIX
  
   This is something onto which we tag something meaningful to make a cache file name.
   used in HTWSRC.c at least. If it is nor defined at all, caching is turned off.
   
 */
#ifndef CACHE_FILE_PREFIX
#ifdef unix
#define CACHE_FILE_PREFIX  "/usr/wsrc/"
#endif
#endif

#endif /* TCP_H */



/*

   end of system-specific file  */

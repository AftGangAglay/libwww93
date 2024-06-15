/*
 * System dependencies in the W3 library
 * SYSTEM DEPENDENCIES
 *
 * System-system differences for TCP include files and macros. This file includes for each
 * system the files necessary for network and file I/O.
 *
 * AUTHORS:
 * 		TBL 	Tim Berners-Lee, W3 project, CERN, <timbl@info.cern.ch>
 * 		EvA		Eelco van Asperen <evas@cs.few.eur.nl>
 * 		MA		Marc Andreesen NCSA
 * 		AT		Aleksandar Totic <atotic@ncsa.uiuc.edu>
 * 		SCW		Susan C. Weber <sweber@kyle.eitech.com>
 *
 * HISTORY:
 * 		22 Feb 91	Written (TBL) as part of the WWW library.
 * 		16 Jan 92	PC code from EvA
 * 		22 Apr 93	Merged diffs bits from xmosaic release
 * 		29 Apr 93	Windows/NT code from SCW
 */

#ifndef HT_TCP_H
#define HT_TCP_H

#ifdef _WIN32
# ifndef _CRT_SECURE_NO_WARNINGS
#  define _CRT_SECURE_NO_WARNINGS
# endif
# ifndef _CRT_NONSTDC_NO_WARNINGS
#  define _CRT_NONSTDC_NO_WARNINGS
# endif
#endif

#ifndef _SVID_SOURCE
/*
 * NOTE: This is to circumvent the glibc warning, we don't really want
 * 		 Everything this claims to expose.
 */
# define _DEFAULT_SOURCE
# define _SVID_SOURCE
#endif

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4668) /* Symbol not defined as macro. */
#endif

#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _WIN32
/*
 * IBM-PC running Windows NT
 * These parameters provided by  Susan C. Weber <sweber@kyle.eitech.com>.
 */

# include <io.h>
# include <process.h>
# include <direct.h>
# include <winsock.h>
#else
/*
 * Regular BSD unix versions
 * These are a default unix where not already defined specifically.
 */

# include <unistd.h>
# include <sys/time.h>
# include <sys/param.h>
# include <sys/file.h>

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
#endif

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#ifndef _MSC_VER
# undef _DEFAULT_SOURCE
# undef _SVID_SOURCE
#endif

#ifdef _WIN32
# undef _CRT_SECURE_NO_WARNINGS
# undef _CRT_NONSTDC_NO_WARNINGS
#endif

#endif

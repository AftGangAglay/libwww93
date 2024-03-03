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

#ifndef TCP_H
#define TCP_H

#ifndef _SVID_SOURCE
/*
 * NOTE: This is to circumvent the glibc warning, we don't really want
 * 		 Everything this claims to expose.
 */
# define _DEFAULT_SOURCE
# define _SVID_SOURCE
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

#endif

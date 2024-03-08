/*		Access Manager					HTAccess.c
**		==============
**
** Authors
**	TBL	Tim Berners-Lee timbl@info.cern.ch
**	JFG	Jean-Francois Groff jfg@dxcern.cern.ch
**	DD	Denis DeLaRoca (310) 825-4580  <CSP1DWD@mvs.oac.ucla.edu>
** History
**       8 Jun 92 Telnet hopping prohibited as telnet is not secure TBL
**	26 Jun 92 When over DECnet, suppressed FTP, Gopher and News. JFG
**	 6 Oct 92 Moved HTClientHost and logfile into here. TBL
**	17 Dec 92 Tn3270 added, bug fix. DD
**	 4 Feb 93 Access registration, Search escapes bad chars TBL
**		  PARAMETERS TO HTSEARCH AND HTLOADRELATIVE CHANGED
**	28 May 93 WAIS gateway explicit if no WAIS library linked in.
**
** Bugs
**	This module assumes that that the graphic object is hypertext, as it
**	needs to select it when it has been loaded. A superclass needs to be
**	defined which accepts select and select_anchor.
*/

#ifndef DEFAULT_WAIS_GATEWAY
#define DEFAULT_WAIS_GATEWAY "http://info.cern.ch:8001/"
#endif

/* Implements:
*/
#include "HTAccess.h"

/* Uses:
*/

#include "HTParse.h"
#include "HTUtils.h"
#include "HTML.h"        /* SCW */

#ifndef NO_RULES

#include "HTRules.h"

#endif

#include "HTList.h"
#include "HText.h"    /* See bugs above */
#include "HTAlert.h"

#include "tcp.h"

/*	These flags may be set to modify the operation of this module
*/
char* HTClientHost = 0;    /* Name of remote login host if any */
FILE* logfile = 0;    /* File to which to output one-liners */
HTBool HTSecure = HT_FALSE;    /* Disable access for telnet users? */

/*	To generate other things, play with these:
*/

HTFormat HTOutputFormat = NULL;
HTStream* HTOutputStream = NULL;    /* For non-interactive, set this */

static HTList* protocols = NULL;   /* List of registered protocol descriptors */


/*	Register a Protocol				HTRegisterProtocol
**	-------------------
*/

HTBool HTRegisterProtocol(HTProtocol* protocol) {
	if(!protocols) protocols = HTList_new();
	HTList_addObject(protocols, protocol);
	return HT_TRUE;
}


/*	Register all known protocols
**	----------------------------
**
**	Add to or sub from this list if you add or remove protocol modules.
**	This routine is called the first time the protocol list is needed,
**	unless any protocols are already registered, in which case it is not called.
**	Therefore the application can override this list.
**
**	Compiling with NO_INIT prevents all known protocols from being forced
**	in at link time.
*/
#ifndef NO_INIT

static void HTAccessInit(void)            /* Call me once */
{
	extern HTProtocol HTTP, HTFile, HTTelnet, HTTn3270, HTRlogin;
#ifndef DECNET
	extern HTProtocol HTFTP, HTNews, HTGopher;
#ifdef DIRECT_WAIS
	extern HTProtocol HTWAIS;
#endif
	HTRegisterProtocol(&HTFTP);
	HTRegisterProtocol(&HTNews);
	HTRegisterProtocol(&HTGopher);
#ifdef DIRECT_WAIS
	HTRegisterProtocol(&HTWAIS);
#endif
#endif

	HTRegisterProtocol(&HTTP);
	HTRegisterProtocol(&HTFile);
	HTRegisterProtocol(&HTTelnet);
	HTRegisterProtocol(&HTTn3270);
	HTRegisterProtocol(&HTRlogin);
}

#endif


/*		Find physical name and access protocol
**		--------------------------------------
**
**
** On entry,
**	addr		must point to the fully qualified hypertext reference.
**	anchor		a pareent anchor with whose address is addr
**
** On exit,
**	returns		HT_NO_ACCESS		Error has occured.
**			HT_OK			Success
**
*/
static int get_physical(const char* addr, HTParentAnchor* anchor) {
	char* access = 0;    /* Name of access method */
	char* physical = 0;

#ifndef NO_RULES
	physical = HTTranslate(addr);
	if(!physical) {
		return HT_FORBIDDEN;
	}
	HTAnchor_setPhysical(anchor, physical);
	free(physical);            /* free our copy */
#else
	HTAnchor_setPhysical(anchor, addr);
#endif

	access = HTParse(
			HTAnchor_physical(anchor), "file:", HT_PARSE_ACCESS);

/*	Check whether gateway access has been set up for this
**
**	This function can be replaced by the rule system above.
*/
#define USE_GATEWAYS
#ifdef USE_GATEWAYS
	{
		char* gateway_parameter, * gateway;
		gateway_parameter = malloc(strlen(access) + 20);
		if(gateway_parameter == NULL) outofmem(__FILE__, "HTLoad");
		strcpy(gateway_parameter, "WWW_");
		strcat(gateway_parameter, access);
		strcat(gateway_parameter, "_GATEWAY");
		gateway = getenv(gateway_parameter); /* coerce for decstation */
		free(gateway_parameter);

#ifndef DIRECT_WAIS
		if(!gateway && 0 == strcmp(access, "wais")) {
			gateway = DEFAULT_WAIS_GATEWAY;
		}
#endif
		if(gateway) {
			char* path = HTParse(
					addr, "",
					HT_PARSE_HOST + HT_PARSE_PATH + HT_PARSE_PUNCTUATION);
			/* Chop leading / off to make host into part of path */
			char* gatewayed = HTParse(path + 1, gateway, HT_PARSE_ALL);
			free(path);
			HTAnchor_setPhysical(anchor, gatewayed);
			free(gatewayed);
			free(access);

			access = HTParse(
					HTAnchor_physical(anchor), "http:", HT_PARSE_ACCESS);
		}
	}
#endif



/*	Search registered protocols to find suitable one
*/
	{
		int i, n;
#ifndef NO_INIT
		if(!protocols) HTAccessInit();
#endif
		n = HTList_count(protocols);
		for(i = 0; i < n; i++) {
			HTProtocol* p = HTList_objectAt(protocols, i);
			if(strcmp(p->name, access) == 0) {
				HTAnchor_setProtocol(anchor, p);
				free(access);
				return (HT_OK);
			}
		}
	}

	free(access);
	return HT_NO_ACCESS;
}


/*		Load a document
**		---------------
**
**	This is an internal routine, which has an address AND a matching
**	anchor. (The public routines are called with one OR the other.)
**
** On entry,
**	addr		must point to the fully qualified hypertext reference.
**	anchor		a pareent anchor with whose address is addr
**
** On exit,
**	returns		<0		Error has occured.
**			HT_LOADED	Success
**			HT_NO_DATA	Success, but no document loaded.
**					(telnet sesssion started etc)
**
*/
static int HTLoad(
		const char* addr, HTParentAnchor* anchor, HTFormat format_out,
		HTStream* sink) {
	HTProtocol* p;
	int status = get_physical(addr, anchor);
	if(status == HT_FORBIDDEN) {
		return HTLoadError(sink, 500, "Access forbidden by rule");
	}
	if(status < 0) return status;    /* Can't resolve or forbidden */

	p = HTAnchor_protocol(anchor);
	return (*(p->load))(
			HTAnchor_physical(anchor), anchor, format_out, sink);
}


/*		Get a save stream for a document
**		--------------------------------
*/
HTStream* HTSaveStream(HTParentAnchor* anchor) {
	HTProtocol* p = HTAnchor_protocol(anchor);
	if(!p) return NULL;

	return (*p->saveStream)(anchor);

}


/*		Load a document - with logging etc
**		----------------------------------
**
**	- Checks or documents already loaded
**	- Logs the access
**	- Allows stdin filter option
**	- Trace ouput and error messages
**
**    On Entry,
**	  anchor	    is the node_anchor for the document
**        full_address      The address of the document to be accessed.
**        filter            if HT_TRUE, treat stdin as HTML
**
**    On Exit,
**        returns    HT_TRUE     Success in opening document
**                   HT_FALSE      Failure
**
*/

static HTBool HTLoadDocument(
		const char* full_address, HTParentAnchor* anchor, HTFormat format_out,
		HTStream* sink) {
	int status;
	HText* text;

	if(TRACE) {
		fprintf(
				stderr, "HTAccess: loading document %s\n", full_address);
	}

	if((text = (HText*) HTAnchor_document(anchor))) {    /* Already loaded */
		if(TRACE) fprintf(stderr, "HTAccess: Document already in memory.\n");
		HText_select(text);
		return HT_TRUE;
	}

	status = HTLoad(full_address, anchor, format_out, sink);


/*	Log the access if necessary
*/
	if(logfile) {
		time_t theTime;
		time(&theTime);
		fprintf(
				logfile, "%24.24s %s %s %s\n", ctime(&theTime),
				HTClientHost ? HTClientHost : "local",
				status < 0 ? "FAIL" : "GET", full_address);
		fflush(logfile);    /* Actually update it on disk */
		if(TRACE) {
			fprintf(
					stderr, "Log: %24.24s %s %s %s\n", ctime(&theTime),
					HTClientHost ? HTClientHost : "local",
					status < 0 ? "FAIL" : "GET", full_address);
		}
	}


	if(status == HT_LOADED) {
		if(TRACE) {
			fprintf(
					stderr, "HTAccess: `%s' has been accessed.\n",
					full_address);
		}
		return HT_TRUE;
	}

	if(status == HT_NO_DATA) {
		if(TRACE) {
			fprintf(
					stderr, "HTAccess: `%s' has been accessed, No data left.\n",
					full_address);
		}
		return HT_FALSE;
	}

	if(status < 0) {              /* Failure in accessing a document */
		if(TRACE) {
			fprintf(
					stderr, "HTAccess: Can't access `%s'\n", full_address);
		}
		HTLoadError(sink, 500, "Unable to access document.");
		return HT_FALSE;
	}

	/* If you get this, then please find which routine is returning
	   a positive unrecognised error code! */

	fprintf(
			stderr,
			"**** HTAccess: socket or file number returned by obsolete load routine!\n");
	fprintf(
			stderr,
			"**** HTAccess: Internal software error. Please mail www-bug@info.cern.ch!\n");
	exit(-6996);

} /* HTLoadDocument */



/*		Load a document from absolute name
**		---------------
**
**    On Entry,
**        addr     The absolute address of the document to be accessed.
**        filter   if HT_TRUE, treat document as HTML
**
**    On Exit,
**        returns    HT_TRUE     Success in opening document
**                   HT_FALSE      Failure
**
**
*/

HTBool HTLoadAbsolute(const char* addr) {
	return HTLoadDocument(
			addr, HTAnchor_parent(HTAnchor_findAddress(addr)),
			HTOutputFormat ? HTOutputFormat : WWW_PRESENT, HTOutputStream);
}


/*		Load a document from absolute name to stream
**		--------------------------------------------
**
**    On Entry,
**        addr     The absolute address of the document to be accessed.
**        sink     if non-NULL, send data down this stream
**
**    On Exit,
**        returns    HT_TRUE     Success in opening document
**                   HT_FALSE      Failure
**
**
*/

HTBool HTLoadToStream(const char* addr, HTBool filter, HTStream* sink) {
	(void) filter;
	return HTLoadDocument(
			addr, HTAnchor_parent(HTAnchor_findAddress(addr)),
			HTOutputFormat ? HTOutputFormat : WWW_PRESENT, sink);
}


/*		Load a document from relative name
**		---------------
**
**    On Entry,
**        relative_name     The relative address of the document
**	  		    to be accessed.
**
**    On Exit,
**        returns    HT_TRUE     Success in opening document
**                   HT_FALSE      Failure
**
**
*/

HTBool HTLoadRelative(const char* relative_name, HTParentAnchor* here) {
	char* full_address = 0;
	HTBool result;
	char* mycopy = 0;
	char* stripped = 0;
	char* current_address = HTAnchor_address((HTAnchor*) here);

	StrAllocCopy(mycopy, relative_name);

	stripped = HTStrip(mycopy);
	full_address = HTParse(
			stripped, current_address,
			HT_PARSE_ACCESS | HT_PARSE_HOST | HT_PARSE_PATH |
			HT_PARSE_PUNCTUATION);
	result = HTLoadAbsolute(full_address);
	free(full_address);
	free(current_address);
	free(mycopy);  /* Memory leak fixed 10/7/92 -- JFG */
	return result;
}


/*		Load if necessary, and select an anchor
**		--------------------------------------
**
**    On Entry,
**        destination      	    The child or parenet anchor to be loaded.
**
**    On Exit,
**        returns    HT_TRUE     Success
**                   HT_FALSE      Failure
**
*/

HTBool HTLoadAnchor(HTAnchor* destination) {
	HTParentAnchor* parent;
	HTBool loaded = HT_FALSE;
	if(!destination) return HT_FALSE;    /* No link */

	parent = HTAnchor_parent(destination);

	if(HTAnchor_document(parent) == NULL) {    /* If not alread loaded */
		/* TBL 921202 */

		HTBool result;
		char* address = HTAnchor_address((HTAnchor*) parent);
		result = HTLoadDocument(
				address, parent, HTOutputFormat ? HTOutputFormat : WWW_PRESENT,
				HTOutputStream);
		free(address);
		if(!result) return HT_FALSE;
		loaded = HT_TRUE;
	}

	{
		HText* text = (HText*) HTAnchor_document(parent);
		if(destination != (HTAnchor*) parent) {  /* If child anchor */
			HText_selectAnchor(
					text,
					(HTChildAnchor*) destination); /* Double display? @@ */
		}
		else {
			if(!loaded) HText_select(text);
		}
	}
	return HT_TRUE;

} /* HTLoadAnchor */


/*		Search
**		------
**  Performs a keyword search on word given by the user. Adds the keyword to 
**  the end of the current address and attempts to open the new address.
**
**  On Entry,
**       *keywords  	space-separated keyword list or similar search list
**	here		is anchor search is to be done on.
*/

static char hex(int i) {
	char* hexchars = "0123456789ABCDEF";
	return hexchars[i];
}

HTBool HTSearch(const char* keywords, HTParentAnchor* here) {

#define acceptable \
"1234567890abcdefghijlkmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.-_"

	char* q, * u;
	const char* p, * s, * e;        /* Pointers into keywords */
	char* address = HTAnchor_address((HTAnchor*) here);
	HTBool result;
	char* escaped = malloc(strlen(keywords) * 3 + 1);

	static const HTBool isAcceptable[96] =

			/*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
			{
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1,
					0,    /* 2x   !"#$%&'()*+,-./	 */
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
					0,    /* 3x  0123456789:;<=>?	 */
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1,    /* 4x  @ABCDEFGHIJKLMNO  */
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
					1,    /* 5X  PQRSTUVWXYZ[\]^_	 */
					0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
					1,    /* 6x  `abcdefghijklmno	 */
					1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
					0 };    /* 7X  pqrstuvwxyz{\}~	DEL */

	if(escaped == NULL) outofmem(__FILE__, "HTSearch");


/*	Convert spaces to + and hex escape unacceptable characters
*/
	for(s = keywords; *s && HT_WHITE(*s);
			s++) { /*scan */ }    /* Skip white space */
	for(e = s + strlen(s); e > s && HT_WHITE(*(e - 1));
			e--) { } /* Skip trailers */
	for(q = escaped, p = s; p < e; p++) {            /* scan stripped field */
		int c = (int) (*p);
		if(HT_WHITE(*p)) {
			*q++ = '+';
		}
		else if(c >= 32 && c <= (char) 127 && isAcceptable[c - 32]) {
			*q++ = *p;            /* 930706 TBL for MVS bug */
		}
		else {
			*q++ = '%';
			*q++ = hex(c / 16);
			*q++ = hex(c % 16);
		}
	} /* Loop over string */

	*q = 0;
	/* terminate escaped sctring */
	u = strchr(address, '?');        /* Find old search string */
	if(u) *u = 0;                    /* Chop old search off */

	StrAllocCat(address, "?");
	StrAllocCat(address, escaped);
	free(escaped);
	result = HTLoadRelative(address, here);
	free(address);

	return result;
}


/*		Search Given Indexname
**		------
**  Performs a keyword search on word given by the user. Adds the keyword to 
**  the end of the current address and attempts to open the new address.
**
**  On Entry,
**       *keywords  	space-separated keyword list or similar search list
**	*addres		is name of object search is to be done on.
*/

HTBool HTSearchAbsolute(const char* keywords, const char* indexname) {
	HTParentAnchor* anchor = (HTParentAnchor*) HTAnchor_findAddress(indexname);
	return HTSearch(keywords, anchor);
}


/*		Generate the anchor for the home page
**		-------------------------------------
**
**	As it involves file access, this should only be done once
**	when the program first runs.
**	This is a default algorithm -- browser don't HAVE to use this.
**	But consistency betwen browsers is STRONGLY recommended!
**
**	Priority order is:
**
**		1	WWW_HOME environment variable (logical name, etc)
**		2	~/WWW/default.html
**		3	/usr/local/bin/default.html
**		4	http://info.cern.ch/default.html
**
*/
HTParentAnchor* HTHomeAnchor(void) {
	char* my_home_document = NULL;
	char* home = getenv(LOGICAL_DEFAULT);
	char* ref;
	HTParentAnchor* anchor;

	if(home) {
		StrAllocCopy(my_home_document, home);

/* 	Someone telnets in, they get a special home.
*/
#define MAX_FILE_NAME 1024                    /* @@@ */
	}
	else if(HTClientHost) {            /* Telnet server */
		FILE* fp = fopen(REMOTE_POINTER, "r");
		char* status;
		if(fp) {
			my_home_document = malloc(MAX_FILE_NAME);
			status = fgets(my_home_document, MAX_FILE_NAME, fp);
			if(!status) {
				free(my_home_document);
				my_home_document = NULL;
			}
			fclose(fp);
		}
		if(!my_home_document) StrAllocCopy(my_home_document, REMOTE_ADDRESS);
	}


#ifdef unix

	if (!my_home_document) {
	FILE * fp = NULL;
	const char * home =  (const char*)getenv("HOME");
	if (home) { 
		my_home_document = (char *)malloc(
		strlen(home)+1+ strlen(PERSONAL_DEFAULT)+1);
		if (my_home_document == NULL) outofmem(__FILE__, "HTLocalName");
		sprintf(my_home_document, "%s/%s", home, PERSONAL_DEFAULT);
		fp = fopen(my_home_document, "r");
	}

	if (!fp) {
		StrAllocCopy(my_home_document, LOCAL_DEFAULT_FILE);
		fp = fopen(my_home_document, "r");
	}
	if (fp) {
		fclose(fp);
	} else {
	if (TRACE) fprintf(stderr,
		"HTBrowse: No local home document ~/%s or %s\n",
		PERSONAL_DEFAULT, LOCAL_DEFAULT_FILE);
		free(my_home_document);
		my_home_document = NULL;
	}
	}
#endif
	ref = HTParse(
			my_home_document ? my_home_document : HTClientHost ? REMOTE_ADDRESS
															   : LAST_RESORT,
			"file:", HT_PARSE_ACCESS | HT_PARSE_HOST | HT_PARSE_PATH |
					 HT_PARSE_PUNCTUATION);
	if(my_home_document) {
		if(TRACE) {
			fprintf(
					stderr,
					"HTAccess: Using custom home page %s i.e. address %s\n",
					my_home_document, ref);
		}
		free(my_home_document);
	}
	anchor = (HTParentAnchor*) HTAnchor_findAddress(ref);
	free(ref);
	return anchor;
}



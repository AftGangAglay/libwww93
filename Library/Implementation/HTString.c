/*		Case-independent string comparison		HTString.c
**
**	Original version came with listserv implementation.
**	Version TBL Oct 91 replaces one which modified the strings.
**	02-Dec-91 (JFG) Added stralloccopy and stralloccat
**	23 Jan 92 (TBL) Changed strallocc* to 8 char HTSAC* for VM and suchlike
**	 6 Oct 92 (TBL) Moved WWW_TraceFlag in here to be in library
*/
#include <ctype.h>
#include "HTUtils.h"
#include "tcp.h"

int WWW_TraceFlag = 0;    /* Global trace flag for ALL W3 code */

#ifndef VC
#define VC "unknown"
#endif

const char* HTLibraryVersion = VC; /* String for help screen etc */

#ifndef VM        /* VM has these already it seems */

/*	Strings of any length
**	---------------------
*/
int strcasecomp(const char* a, const char* b) {
	const char* p = a;
	const char* q = b;
	for(p = a, q = b; *p && *q; p++, q++) {
		int diff = tolower(*p) - tolower(*q);
		if(diff) return diff;
	}
	if(*p) return 1;    /* p was longer than q */
	if(*q) return -1;    /* p was shorter than q */
	return 0;        /* Exact match */
}


/*	With count limit
**	----------------
*/
int strncasecomp(const char* a, const char* b, int n) {
	const char* p = a;
	const char* q = b;

	for(p = a, q = b;; p++, q++) {
		int diff;
		if(p == a + n) return 0;    /*   Match up to n characters */
		if(!(*p && *q)) return *p - *q;
		diff = tolower(*p) - tolower(*q);
		if(diff) return diff;
	}
	/*NOTREACHED*/
}

#endif

/*	Allocate a new copy of a string, and returns it
*/
char* HTSACopy(char** dest, const char* src) {
	if(*dest) free(*dest);
	if(!src) {
		*dest = NULL;
	}
	else {
		*dest = malloc(strlen(src) + 1);
		if(*dest == NULL) outofmem(__FILE__, "HTSACopy");
		strcpy(*dest, src);
	}
	return *dest;
}

/*	String Allocate and Concatenate
*/
char* HTSACat(char** dest, const char* src) {
	if(src && *src) {
		if(*dest) {
			int length = strlen(*dest);
			*dest = realloc(*dest, length + strlen(src) + 1);
			if(*dest == NULL) outofmem(__FILE__, "HTSACat");
			strcpy(*dest + length, src);
		}
		else {
			*dest = malloc(strlen(src) + 1);
			if(*dest == NULL) outofmem(__FILE__, "HTSACat");
			strcpy(*dest, src);
		}
	}
	return *dest;
}


/*	Find next Field
**	---------------
**
** On entry,
**	*pstr	points to a string containig white space separated
**		field, optionlly quoted.
**
** On exit,
**	*pstr	has been moved to the first delimiter past the
**		field
**		THE STRING HAS BEEN MUTILATED by a 0 terminator
**
**	returns	a pointer to the first field
*/
char* HTNextField(char** pstr) {
	char* p = *pstr;
	char* start;            /* start of field */

	while(*p && HT_WHITE(*p)) p++;        /* Strip white space */
	if(!*p) {
		*pstr = p;
		return NULL;        /* No first field */
	}
	if(*p == '"') {            /* quoted field */
		p++;
		start = p;
		for(; *p && *p != '"'; p++) {
			if(*p == '\\' && p[1]) p++;    /* Skip escaped chars */
		}
	}
	else {
		start = p;
		while(*p && !HT_WHITE(*p)) p++;    /* Skip first field */
	}
	if(*p) *p++ = 0;
	*pstr = p;
	return start;
}

/*
 * String handling for libwww
 * STRINGS
 *
 * Case-independent string comparison and allocations with copies etc
 */
#ifndef HTSTRING_H
#define HTSTRING_H

#include <HTUtils.h>

extern const char* HTLibraryVersion;   /* String for help screen etc */

/*
 * Case-insensitive string comparison
 * The usual routines (comp instead of cmp) had some problem.
 */
int strcasecomp(const char* a, const char* b);
int strncasecomp(const char* a, const char* b, int n);

/*
 * Malloced string manipulation
 */
#define StrAllocCopy(dest, src) HTSACopy (&(dest), src)
#define StrAllocCat(dest, src)  HTSACat  (&(dest), src)

char* HTSACopy(char** dest, const char* src);

char* HTSACat(char** dest, const char* src);

/*
 * Next word or quoted string
 */
char* HTNextField(char** pstr);

#endif

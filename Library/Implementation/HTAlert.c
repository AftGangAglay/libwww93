/*	Displaying messages and getting input for LineMode Browser
**	==========================================================
**
**	REPLACE THIS MODULE with a GUI version in a GUI environment!
**
** History:
**	   Jun 92 Created May 1992 By C.T. Barker
**	   Feb 93 Simplified, portablised TBL
**
*/


#include "HTAlert.h"

#include "tcp.h"        /* for toupper */
#include <ctype.h>        /* for toupper - should be in tcp.h */

void HTAlert(const char* Msg) {
#ifdef NeXTStep
	NXRunAlertPanel(NULL, "%s", NULL, NULL, NULL, Msg);
#else
	fprintf(stderr, "WWW Alert:  %s\n", Msg);
#endif
}


void HTProgress(const char* Msg) {
	fprintf(stderr, "   %s ...\n", Msg);
}


HTBool HTConfirm(const char* Msg) {
	char Reply[4];
	char* URep;

	fprintf(stderr, "WWW: %s (y/n) ", Msg);
	fprintf(stderr, "(y/n) ");

	scanf("%3s", Reply); /* get reply, max 3 characters */
	URep = Reply;
	while(*URep) {
		URep++;
		*URep = (char) toupper(*URep);
	}

	if((strcmp(Reply, "HT_TRUE") == 0) || (strcmp(Reply, "Y") == 0)) {
		return (HT_TRUE);
	}
	else {
		return (HT_FALSE);
	}
}

/*	Prompt for answer and get text back
*/
char* HTPrompt(const char* Msg, const char* deflt) {
	char Tmp[200];
	char* rep = 0;
	fprintf(stderr, "WWW: %s", Msg);
	if(deflt) fprintf(stderr, "\n (RETURN for %s)\n", deflt);

	fgets(Tmp, 199, stdin);

	StrAllocCopy(rep, *Tmp ? Tmp : deflt);
	return rep;
}

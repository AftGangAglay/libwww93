/*		FILE WRITER				HTFWrite.h
**		===========
**
**	This version of the stream object just writes to a C file.
**	The file is assumed open and left open.
**
**	Bugs:
**		strings written must be less than buffer size.
*/

#include "HTFWriter.h"

#include "HTFormat.h"
#include "HTAlert.h"
#include "HTFile.h"

/*		Stream Object
**		------------
*/

struct _HTStream {
	const HTStreamClass* isa;

	FILE* fp;
	char* end_command;
	char* remove_command;
	HTBool announce;
};


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
**  Bug:
**	All errors are ignored.
*/

/*	Character handling
**	------------------
*/

static void HTFWriter_put_character (HTStream * me, char c) {
	putc(c, me->fp);
}



/*	String handling
**	---------------
**
**	Strings must be smaller than this buffer size.
*/
static void HTFWriter_put_string (HTStream * me, const char* s) {
	fputs(s, me->fp);
}


/*	Buffer write. Buffers can (and should!) be big.
**	------------
*/
static void HTFWriter_write (HTStream * me, const char* s, int l) {
	fwrite(s, 1, l, me->fp);
}




/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created
**	object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
static void HTFWriter_free (HTStream * me) {
	fflush(me->fp);
	if(me->end_command) {        /* Temp file */
		fclose(me->fp);
		HTProgress(me->end_command);    /* Tell user what's happening */
		system(me->end_command);
		free(me->end_command);
		if(me->remove_command) {
			system(me->remove_command);
			free(me->remove_command);
		}
	}

	free(me);
}

/*	End writing
*/

static void HTFWriter_abort (HTStream * me, HTError e) {
	(void) e;

	fflush(me->fp);
	if(me->end_command) {        /* Temp file */
		fclose(me->fp);
		if(TRACE) {
			fprintf(
					stderr, "HTFWriter: Aborting: file not executed.\n");
		}
		free(me->end_command);
		if(me->remove_command) {
			system(me->remove_command);
			free(me->remove_command);
		}
	}

	free(me);
}



/*	Structured Object Class
**	-----------------------
*/
static const HTStreamClass HTFWriter = /* As opposed to print etc */
		{
				"FileWriter", HTFWriter_free, HTFWriter_abort,
				HTFWriter_put_character, HTFWriter_put_string,
				HTFWriter_write };


/*	Subclass-specific Methods
**	-------------------------
*/

HTStream* HTFWriter_new (FILE * fp) {
	HTStream* me;

	if(!fp) return NULL;

	me = malloc(sizeof(*me));
	if(me == NULL) outofmem(__FILE__, "HTML_new");
	me->isa = &HTFWriter;

	me->fp = fp;
	me->end_command = NULL;
	me->remove_command = NULL;
	me->announce = HT_FALSE;

	return me;
}

/*	Make system command from template
**	---------------------------------
**
**	See mailcap spec for description of template.
*/
/* @@ to be written. sprintfs will do for now. */


/*	Take action using a system command
**	----------------------------------
**
**	originally from Ghostview handling by Marc Andreseen.
**	Creates temporary file, writes to it, executes system command
**	on end-document. The suffix of the temp file can be given
**	in case the application is fussy, or so that a generic opener can
**	be used.
*/
HTStream*
HTSaveAndExecute (HTPresentation * pres, HTParentAnchor *
					   anchor,    /* Not used */
					   HTStream * sink)    /* Not used */

#ifdef unix
#define REMOVE_COMMAND "/bin/rm -f %s\n"
#endif
#ifdef VMS
#define REMOVE_COMMAND "delete/noconfirm/nolog %s.."
#endif

#ifdef REMOVE_COMMAND
{
	char *fnam;
	const char * suffix;

	HTStream* me;

	me = (HTStream*)malloc(sizeof(*me));
	if (me == NULL) outofmem(__FILE__, "Save and execute");
	me->isa = &HTFWriter;

	/* Save the file under a suitably suffixed name */

	suffix = HTFileSuffix(pres->rep);

	fnam = (char *)malloc (L_tmpnam + 16 + strlen(suffix));
	tmpnam (fnam);
	if (suffix) strcat(fnam, suffix);

	me->fp = fopen (fnam, "w");
	if (!me->fp) {
	HTAlert("Can't open temporary file!");
		free(fnam);
	free(me);
	return NULL;
	}

/*	Make command to process file
*/
	me->end_command = (char *)malloc (
				(strlen (pres->command) + 10+ 3*strlen(fnam))
				 * sizeof (char));
	if (me == NULL) outofmem(__FILE__, "SaveAndExecute");

	sprintf (me->end_command, pres->command, fnam, fnam, fnam);

	me->remove_command = NULL;	/* If needed, put into end_command */
#ifdef NOPE
/*	Make command to delete file
*/
	me->remove_command = (char *)malloc (
				(strlen (REMOVE_COMMAND) + 10+ strlen(fnam))
				 * sizeof (char));
	if (me == NULL) outofmem(__FILE__, "SaveAndExecute");

	sprintf (me->remove_command, REMOVE_COMMAND, fnam);
#endif

	me->announce = HT_FALSE;
	free (fnam);
	return me;
}

#else	/* can do remove */
{
	(void) pres;
	(void) anchor;
	(void) sink;

	return NULL;
}

#endif


/*	Save Locally
**	------------
**
**  Bugs:
**	GUI Apps should open local Save panel here really.
**
*/
HTStream* HTSaveLocally (HTPresentation * pres, HTParentAnchor *
									 anchor,    /* Not used */
									 HTStream * sink)    /* Not used */

{
	char* fnam;
	char* answer;
	const char* suffix;

	HTStream* me;

	(void) anchor;
	(void) sink;

	me = malloc(sizeof(*me));
	if(me == NULL) outofmem(__FILE__, "SaveLocally");
	me->isa = &HTFWriter;
	me->end_command = NULL;
	me->remove_command = NULL;    /* If needed, put into end_command */
	me->announce = HT_TRUE;

	/* Save the file under a suitably suffixed name */

	suffix = HTFileSuffix(pres->rep);

	fnam = malloc(L_tmpnam + 16 + strlen(suffix));
	tmpnam(fnam);
	if(suffix) strcat(fnam, suffix);

	/*	Save Panel */
	answer = HTPrompt("Give name of file to save in", fnam);

	free(fnam);

	me->fp = fopen(answer, "w");
	if(!me->fp) {
		HTAlert("Can't open local file to write into.");
		free(answer);
		free(me);
		return NULL;
	}

	free(answer);
	return me;
}



/*	Format Converter using system command
**	-------------------------------------
*/

/*		Plain text object		HTWrite.c
**		=================
**
**	This version of the stream object just writes to a socket.
**	The socket is assumed open and left open.
**
**	Bugs:
**		strings written must be less than buffer size.
*/
#include <HTPlain.h>

#define BUFFER_SIZE 4096;    /* Tradeoff */

#include <HTUtils.h>
#include <HText.h>
#include <HTStyle.h>

extern HTStyleSheet* styleSheet;


/*		HTML Object
**		-----------
*/

struct _HTStream {
	const HTStreamClass* isa;

	HText* text;
};

/*	Write the buffer out to the socket
**	----------------------------------
*/


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/

static void HTPlain_put_character(HTStream* me, char c) {
	HText_appendCharacter(me->text, c);
}


/*	String handling
**	---------------
**
*/
static void HTPlain_put_string(HTStream* me, const char* s) {
	HText_appendText(me->text, s);
}


static void HTPlain_write(HTStream* me, const char* s, int l) {
	const char* p;
	const char* e = s + l;
	for(p = s; p < e; p++) HText_appendCharacter(me->text, *p);
}


/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
static void HTPlain_free(HTStream* me) {
	free(me);
}

/*	End writing
*/

static void HTPlain_abort(HTStream* me, HTError e) {
	(void) e;
	HTPlain_free(me);
}


/*		Structured Object Class
**		-----------------------
*/
const HTStreamClass HTPlain = {
		"SocketWriter", HTPlain_free, HTPlain_abort, HTPlain_put_character,
		HTPlain_put_string, HTPlain_write, };


/*		New object
**		----------
*/
HTStream* HTPlainPresent(
		HTPresentation* pres, HTParentAnchor* anchor, HTStream* sink) {

	HTStream* me = malloc(sizeof(*me));

	(void) pres;
	(void) sink;

	if(me == NULL) HTOOM(__FILE__, "HTPlain_new");
	me->isa = &HTPlain;

	me->text = HText_new(anchor);
	HText_setStyle(me->text, HTStyleNamed(styleSheet, "Example"));
	HText_beginAppend(me->text);

	return (HTStream*) me;
}



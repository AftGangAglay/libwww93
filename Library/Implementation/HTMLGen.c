/*		HTML Generator
**		==============
**
**	This version of the HTML object sends HTML markup to the output stream.
**
** Bugs:	Line wrapping is not done at all.
**		All data handled as PCDATA.
**		Should convert old XMP, LISTING and PLAINTEXT to PRE.
**
**	It is not obvious to me right now whether the HEAD should be generated
**	from the incomming data or the anchor. Currently itis from the former
**	which is cleanest.
*/

/* Implements:
*/
#include <HTMLGen.h>

#include <HTMLDTD.h>
#include <HTStream.h>
#include <SGML.h>
#include <HTFormat.h>

#define PUTC(c) (*me->targetClass.put_character)(me->target, c)
#define PUTS(s) (*me->targetClass.put_string)(me->target, s)
#define PUTB(s, l) (*me->targetClass.put_block)(me->target, s, l)

/*		HTML Object
**		-----------
*/

struct _HTStream {
	const HTStreamClass* isa;
	HTStream* target;
	HTStreamClass targetClass;    /* COPY for speed */
};

struct _HTStructured {
	const HTStructuredClass* isa;
	HTStream* target;
	HTStreamClass targetClass;    /* COPY for speed */
};


/*	Character handling
**	------------------
*/
static void HTMLGen_put_character(HTStructured* me, char c) {
	PUTC(c);
}


/*	String handling
**	---------------
*/
static void HTMLGen_put_string(HTStructured* me, const char* s) {
	PUTS(s);
}

static void HTMLGen_write(HTStructured* me, const char* s, unsigned l) {
	PUTB(s, l);
}


/*	Start Element
**	-------------
*/
static void HTMLGen_start_element(
		HTStructured* me, int element_number, const HTBool* present,
		const char** value) {
	int i;

	HTTag* tag = &HTML_dtd.tags[element_number];
	PUTC('<');
	PUTS(tag->name);
	if(present) {
		for(i = 0; i < tag->number_of_attributes; i++) {
			if(present[i]) {
				PUTC(' ');
				PUTS(tag->attributes[i].name);
				if(value[i]) {
					PUTS("=\"");
					PUTS(value[i]);
					PUTC('"');
				}
			}
		}
	}
	PUTC('>');
}


/*		End Element
**		-----------
**
*/
/*	When we end an element, the style must be returned to that
**	in effect before that element. Note that anchors (etc?)
**	don't have an associated style, so that we must scan down the
**	stack for an element with a defined style. (In fact, the styles
**	should be linked to the whole stack not just the top one.)
**	TBL 921119
*/
static void HTMLGen_end_element(HTStructured* me, int element_number) {
	PUTS("</");
	PUTS(HTML_dtd.tags[element_number].name);
	PUTC('>');
}


/*		Expanding entities
**		------------------
**
*/

static void HTMLGen_put_entity(HTStructured* me, int entity_number) {
	PUTC('&');
	PUTS(HTML_dtd.entity_names[entity_number]);
	PUTC(';');
}


/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
static void HTMLGen_free(HTStructured* me) {
	(*me->targetClass.free)(me->target);    /* ripple through */
	free(me);
}


static void HTMLGen_abort(HTStructured* me, HTError e) {
	(void) e;

	HTMLGen_free(me);
}


static void PlainToHTML_abort(HTStructured* me, HTError e) {
	(void) e;

	HTMLGen_free(me);
}


/*	Structured Object Class
**	-----------------------
*/
static const HTStructuredClass HTMLGeneration = /* As opposed to print etc */
		{
				"text/html", HTMLGen_free, HTMLGen_abort, HTMLGen_put_character,
				HTMLGen_put_string, HTMLGen_write, HTMLGen_start_element,
				HTMLGen_end_element, HTMLGen_put_entity };


/*	Subclass-specific Methods
**	-------------------------
*/

HTStructured* HTMLGenerator(HTStream* output) {
	HTStructured* me = malloc(sizeof(*me));
	if(me == NULL) outofmem(__FILE__, "HTMLGenerator");
	me->isa = &HTMLGeneration;

	me->target = output;
	me->targetClass = *me->target->isa; /* Copy pointers to routines for speed*/

	return me;
}

/*	Stream Object Class
**	-------------------
**
**	This object just converts a plain text stream into HTML
**	It is officially a structured strem but only the stream bits exist.
**	This is just the easiest way of typecasting all the routines.
*/
static const HTStructuredClass PlainToHTMLConversion = {
		"plaintexttoHTML", HTMLGen_free, PlainToHTML_abort,
		HTMLGen_put_character, HTMLGen_put_string, HTMLGen_write,
		NULL,        /* Structured stuff */
		NULL, NULL };


/*	HTConverter from plain text to HTML Stream
**	------------------------------------------
*/

HTStream* HTPlainToHTML(
		HTPresentation* pres, HTParentAnchor* anchor, HTStream* sink) {
	HTStream* me = malloc(sizeof(*me));

	(void) pres;
	(void) anchor;

	if(me == NULL) outofmem(__FILE__, "PlainToHTML");
	me->isa = (HTStreamClass*) &PlainToHTMLConversion;

	me->target = sink;
	me->targetClass = *me->target->isa;
	/* Copy pointers to routines for speed*/

	PUTS("<BODY>\n<PRE>\n");
	return me;
}



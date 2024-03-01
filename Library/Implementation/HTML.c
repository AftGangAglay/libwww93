/*		Structured stream to Rich hypertext converter
**		============================================
**
**	This generates of a hypertext object.  It converts from the
**	structured stream interface fro HTMl events into the style-
**	oriented iunterface of the HText.h interface.  This module is
**	only used in clients and shouldnot be linked into servers.
**
**	Override this module if making a new GUI browser.
**
*/
#include "HTML.h"

/* #define CAREFUL		 Check nesting here notreally necessary */

#include <ctype.h>
#include <stdio.h>

#include "HTAtom.h"
#include "HTChunk.h"
#include "HText.h"
#include "HTStyle.h"

#include "HTAlert.h"
#include "HTMLGen.h"
#include "HTParse.h"

extern HTStyleSheet* styleSheet;    /* Application-wide */

/*	Module-wide style cache
*/
static int got_styles = 0;
static HTStyle* styles[HTML_ELEMENTS];
static HTStyle* default_style;


/*		HTML Object
**		-----------
*/
#define MAX_NESTING 20        /* Should be checked by parser */

typedef struct _stack_element {
	HTStyle* style;
	int tag_number;
} stack_element;

struct _HTStructured {
	const HTStructuredClass* isa;
	HTParentAnchor* node_anchor;
	HText* text;

	HTStream* target;            /* Output stream */
	HTStreamClass targetClass;        /* Output routines */

	HTChunk title;        /* Grow by 128 */

	char* comment_start;    /* for literate programming */
	char* comment_end;

	HTTag* current_tag;
	HTBool style_change;
	HTStyle* new_style;
	HTStyle* old_style;
	HTBool in_word;  /* Have just had a non-white char */
	stack_element stack[MAX_NESTING];
	stack_element* sp;        /* Style stack pointer */
};

struct _HTStream {
	const HTStreamClass* isa;
	/* .... */
};

/*		Forward declarations of routines
*/
static void get_styles (void);


static void actually_set_style (HTStructured * me);

static void
change_paragraph_style (HTStructured * me, HTStyle * style);

/*	Style buffering avoids dummy paragraph begin/ends.
*/
#define UPDATE_STYLE if (me->style_change) { actually_set_style(me); }



/* 	Entity values -- for ISO Latin 1 local representation
**
**	This MUST match exactly the table referred to in the DTD!
*/
static char* ISO_Latin1[] = {
		"\306",    /* capital AE diphthong (ligature) */
		"\301",    /* capital A, acute accent */
		"\302",    /* capital A, circumflex accent */
		"\300",    /* capital A, grave accent */
		"\305",    /* capital A, ring */
		"\303",    /* capital A, tilde */
		"\304",    /* capital A, dieresis or umlaut mark */
		"\307",    /* capital C, cedilla */
		"\320",    /* capital Eth, Icelandic */
		"\311",    /* capital E, acute accent */
		"\312",    /* capital E, circumflex accent */
		"\310",    /* capital E, grave accent */
		"\313",    /* capital E, dieresis or umlaut mark */
		"\315",    /* capital I, acute accent */
		"\316",    /* capital I, circumflex accent */
		"\314",    /* capital I, grave accent */
		"\317",    /* capital I, dieresis or umlaut mark */
		"\321",    /* capital N, tilde */
		"\323",    /* capital O, acute accent */
		"\324",    /* capital O, circumflex accent */
		"\322",    /* capital O, grave accent */
		"\330",    /* capital O, slash */
		"\325",    /* capital O, tilde */
		"\326",    /* capital O, dieresis or umlaut mark */
		"\336",    /* capital THORN, Icelandic */
		"\332",    /* capital U, acute accent */
		"\333",    /* capital U, circumflex accent */
		"\331",    /* capital U, grave accent */
		"\334",    /* capital U, dieresis or umlaut mark */
		"\335",    /* capital Y, acute accent */
		"\341",    /* small a, acute accent */
		"\342",    /* small a, circumflex accent */
		"\346",    /* small ae diphthong (ligature) */
		"\340",    /* small a, grave accent */
		"\046",    /* ampersand */
		"\345",    /* small a, ring */
		"\343",    /* small a, tilde */
		"\344",    /* small a, dieresis or umlaut mark */
		"\347",    /* small c, cedilla */
		"\351",    /* small e, acute accent */
		"\352",    /* small e, circumflex accent */
		"\350",    /* small e, grave accent */
		"\360",    /* small eth, Icelandic */
		"\353",    /* small e, dieresis or umlaut mark */
		"\076",    /* greater than */
		"\355",    /* small i, acute accent */
		"\356",    /* small i, circumflex accent */
		"\354",    /* small i, grave accent */
		"\357",    /* small i, dieresis or umlaut mark */
		"\074",    /* less than */
		"\361",    /* small n, tilde */
		"\363",    /* small o, acute accent */
		"\364",    /* small o, circumflex accent */
		"\362",    /* small o, grave accent */
		"\370",    /* small o, slash */
		"\365",    /* small o, tilde */
		"\366",    /* small o, dieresis or umlaut mark */
		"\337",    /* small sharp s, German (sz ligature) */
		"\376",    /* small thorn, Icelandic */
		"\372",    /* small u, acute accent */
		"\373",    /* small u, circumflex accent */
		"\371",    /* small u, grave accent */
		"\374",    /* small u, dieresis or umlaut mark */
		"\375",    /* small y, acute accent */
		"\377",    /* small y, dieresis or umlaut mark */
};

/* 	Entity values -- for IBM/PC Code Page 850 (International)
**
**	This MUST match exactly the table referred to in the DTD!
**
*/
/* @@@@@@@@@@@@@@@@@ TBD */



/*		Set character set
**		----------------
*/


/*		Flattening the style structure
**		------------------------------
**
On the NeXT, and on any read-only browser, it is simpler for the text to have
a sequence of styles, rather than a nested tree of styles. In this
case we have to flatten the structure as it arrives from SGML tags into
a sequence of styles.
*/

/*		If style really needs to be set, call this
*/
static void actually_set_style (HTStructured * me) {
	if(!me->text) {            /* First time through */
		me->text = HText_new2(me->node_anchor, me->target);
		HText_beginAppend(me->text);
		HText_setStyle(me->text, me->new_style);
		me->in_word = HT_FALSE;
	}
	else {
		HText_setStyle(me->text, me->new_style);
	}
	me->old_style = me->new_style;
	me->style_change = HT_FALSE;
}

/*      If you THINK you need to change style, call this
*/

static void
change_paragraph_style (HTStructured * me, HTStyle * style) {
	if(me->new_style != style) {
		me->style_change = HT_TRUE;
		me->new_style = style;
	}
	me->in_word = HT_FALSE;
}

/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/
static void HTML_put_character (HTStructured * me, char c) {

	switch(me->sp[0].tag_number) {
		case HTML_COMMENT: break;                    /* Do Nothing */

		case HTML_TITLE: HTChunkPutc(&me->title, c);
			break;


		case HTML_LISTING:                /* Literal text */
		case HTML_XMP:
		case HTML_PLAINTEXT:
		case HTML_PRE:
/*	We guarrantee that the style is up-to-date in begin_literal
*/
			HText_appendCharacter(me->text, c);
			break;

		default:                    /* Free format text */
			if(me->style_change) {
				if((c == '\n') || (c == ' ')) return;    /* Ignore it */
				UPDATE_STYLE;
			}
			if(c == '\n') {
				if(me->in_word) {
					HText_appendCharacter(me->text, ' ');
					me->in_word = HT_FALSE;
				}
			}
			else {
				HText_appendCharacter(me->text, c);
				me->in_word = HT_TRUE;
			}
	} /* end switch */
}



/*	String handling
**	---------------
**
**	This is written separately from put_character becuase the loop can
**	in some cases be promoted to a higher function call level for speed.
*/
static void HTML_put_string (HTStructured * me, const char* s) {

	switch(me->sp[0].tag_number) {
		case HTML_COMMENT: break;                    /* Do Nothing */

		case HTML_TITLE: HTChunkPuts(&me->title, s);
			break;


		case HTML_LISTING:                /* Literal text */
		case HTML_XMP:
		case HTML_PLAINTEXT:
		case HTML_PRE:

/*	We guarrantee that the style is up-to-date in begin_literal
*/
			HText_appendText(me->text, s);
			break;

		default:                    /* Free format text */
		{
			const char* p = s;
			if(me->style_change) {
				for(; *p && ((*p == '\n') || (*p == ' '));
						p++) { }  /* Ignore leaders */
				if(!*p) return;
				UPDATE_STYLE;
			}
			for(; *p; p++) {
				if(me->style_change) {
					if((*p == '\n') || (*p == ' ')) continue;  /* Ignore it */
					UPDATE_STYLE;
				}
				if(*p == '\n') {
					if(me->in_word) {
						HText_appendCharacter(me->text, ' ');
						me->in_word = HT_FALSE;
					}
				}
				else {
					HText_appendCharacter(me->text, *p);
					me->in_word = HT_TRUE;
				}
			} /* for */
		}
	} /* end switch */
}


/*	Buffer write
**	------------
*/
static void HTML_write (HTStructured * me, const char* s, int l) {
	const char* p;
	const char* e = s + l;
	for(p = s; s < e; p++) HTML_put_character(me, *p);
}


/*	Start Element
**	-------------
*/
static void
HTML_start_element (HTStructured * me, int element_number, const HTBool*
						 present, const char ** value) {
	switch(element_number) {
		case HTML_A: {
			HTChildAnchor* source;
			char* href = NULL;
			if(present[HTML_A_HREF]) {
				StrAllocCopy(href, value[HTML_A_HREF]);
				HTSimplify(href);
			}
			source = HTAnchor_findChildAndLink(
					me->node_anchor,                /* parent */
					present[HTML_A_NAME] ? value[HTML_A_NAME] : 0,    /* Tag */
					present[HTML_A_HREF] ? href : 0,        /* Addresss */
					present[HTML_A_TYPE] && value[HTML_A_TYPE]
					? (HTLinkType*) HTAtom_for(value[HTML_A_TYPE]) : 0);

			if(present[HTML_A_TITLE] && value[HTML_A_TITLE]) {
				HTParentAnchor* dest = HTAnchor_parent(
						HTAnchor_followMainLink((HTAnchor*) source));
				if(!HTAnchor_title(dest)) {
					HTAnchor_setTitle(dest, value[HTML_A_TITLE]);
				}
			}
			UPDATE_STYLE;
			HText_beginAnchor(me->text, source);
		}
			break;

		case HTML_TITLE:HTChunkClear(&me->title);
			break;

		case HTML_NEXTID:
			/* if (present[NEXTID_N] && value[NEXTID_N])
			HText_setNextId(me->text, atoi(value[NEXTID_N])); */
			break;

		case HTML_ISINDEX: HTAnchor_setIndex(me->node_anchor);
			break;

		case HTML_P: UPDATE_STYLE;
			HText_appendParagraph(me->text);
			me->in_word = HT_FALSE;
			break;

		case HTML_DL:
			change_paragraph_style(
					me, present && present[DL_COMPACT] ? styles[HTML_DLC]
													   : styles[HTML_DL]);
			break;

		case HTML_DT:
			if(!me->style_change) {
				HText_appendParagraph(me->text);
				me->in_word = HT_FALSE;
			}
			break;

		case HTML_DD:UPDATE_STYLE;
			HTML_put_character(me, '\t');    /* Just tab out one stop */
			me->in_word = HT_FALSE;
			break;

		case HTML_UL:
		case HTML_OL:
		case HTML_MENU:
		case HTML_DIR: change_paragraph_style(me, styles[element_number]);
			break;

		case HTML_LI:UPDATE_STYLE;
			if(me->sp[0].tag_number != HTML_DIR) {
				HText_appendParagraph(me->text);
			}
			else {
				HText_appendCharacter(
						me->text, '\t');
			}    /* Tab @@ nl for UL? */
			me->in_word = HT_FALSE;
			break;

		case HTML_LISTING:                /* Literal text */
		case HTML_XMP:
		case HTML_PLAINTEXT:
		case HTML_PRE: change_paragraph_style(me, styles[element_number]);
			UPDATE_STYLE;
			if(me->comment_end) {
				HText_appendText(me->text, me->comment_end);
			}
			break;

		case HTML_HTML:            /* Ignore these altogether */
		case HTML_HEAD:
		case HTML_BODY:

		case HTML_IMG:            /* Images -- ignore */

		case HTML_TT:            /* Physical character highlighting */
		case HTML_B:            /* Currently ignored */
		case HTML_I:
		case HTML_U:

		case HTML_EM:            /* Logical character highlighting */
		case HTML_STRONG:            /* Currently ignored */
		case HTML_CODE:
		case HTML_SAMP:
		case HTML_KBD:
		case HTML_VAR:
		case HTML_DFN:
		case HTML_CITE: break;

		case HTML_H1:            /* paragraph styles */
		case HTML_H2:
		case HTML_H3:
		case HTML_H4:
		case HTML_H5:
		case HTML_H6:
		case HTML_H7:
		case HTML_ADDRESS:
		case HTML_BLOCKQUOTE:
			change_paragraph_style(
					me, styles[element_number]);    /* May be postponed */
			break;

	} /* end switch */

	if(HTML_dtd.tags[element_number].contents != SGML_EMPTY) {
		if(me->sp == me->stack) {
			fprintf(
					stderr, "HTML: ****** Maximum nesting of %d exceded!\n",
					MAX_NESTING);
			return;
		}
		--(me->sp);
		me->sp[0].style = me->new_style;    /* Stack new style */
		me->sp[0].tag_number = element_number;
	}
}


/*		End Element
**		-----------
**
*/
/*	When we end an element, the style must be returned to that
**	in effect before that element.  Note that anchors (etc?)
**	don't have an associated style, so that we must scan down the
**	stack for an element with a defined style. (In fact, the styles
**	should be linked to the whole stack not just the top one.)
**	TBL 921119
**
**	We don't turn on "CAREFUL" check because the parser produces
**	(internal code errors apart) good nesting. The parser checks
**	incoming code errors, not this module.
*/
static void HTML_end_element (HTStructured * me, int element_number) {
#ifdef CAREFUL            /* parser assumed to produce good nesting */
	if (element_number != me->sp[0].tag_number) {
		fprintf(stderr, "HTMLText: end of element %s when expecting end of %s\n",
		HTML_dtd.tags[element_number].name,
		HTML_dtd.tags[me->sp->tag_number].name);
		/* panic */
	}
#endif

	me->sp++;                /* Pop state off stack */

	switch(element_number) {

		case HTML_A: UPDATE_STYLE;
			HText_endAnchor(me->text);
			break;

		case HTML_TITLE:HTChunkTerminate(&me->title);
			HTAnchor_setTitle(me->node_anchor, me->title.data);
			break;

		case HTML_LISTING:                /* Literal text */
		case HTML_XMP:
		case HTML_PLAINTEXT:
		case HTML_PRE:
			if(me->comment_start) {
				HText_appendText(me->text, me->comment_start);
			}
			/* Fall through */

		default:

			change_paragraph_style(
					me, me->sp->style);    /* Often won't really change */
			break;

	} /* switch */
}


/*		Expanding entities
**		------------------
*/
/*	(In fact, they all shrink!)
*/

static void HTML_put_entity (HTStructured * me, int entity_number) {
	HTML_put_string(
			me, ISO_Latin1[entity_number]);    /* @@ Other representations */
}



/*	Free an HTML object
**	-------------------
**
** If the document is empty, the text object will not yet exist.
   So we could in fact abandon creating the document and return
   an error code.  In fact an empty document is an important type
   of document, so we don't.
**
**	If non-interactive, everything is freed off.   No: crashes -listrefs
**	Otherwise, the interactive object is left.	
*/
void HTML_free (HTStructured * me) {
	UPDATE_STYLE;        /* Creates empty document here! */
	if(me->comment_end) {
		HTML_put_string(me, me->comment_end);
	}
	HText_endAppend(me->text);

	if(me->target) {
		(*me->targetClass.free)(me->target);
	}
	free(me);
}


static void HTML_abort (HTStructured * me, HTError e) {
	if(me->target) {
		(*me->targetClass.abort)(me->target, e);
	}
	free(me);

}


/*	Get Styles from style sheet
**	---------------------------
*/
static void get_styles (void) {
	got_styles = HT_TRUE;

	default_style = HTStyleNamed(styleSheet, "Normal");

	styles[HTML_H1] = HTStyleNamed(styleSheet, "Heading1");
	styles[HTML_H2] = HTStyleNamed(styleSheet, "Heading2");
	styles[HTML_H3] = HTStyleNamed(styleSheet, "Heading3");
	styles[HTML_H4] = HTStyleNamed(styleSheet, "Heading4");
	styles[HTML_H5] = HTStyleNamed(styleSheet, "Heading5");
	styles[HTML_H6] = HTStyleNamed(styleSheet, "Heading6");
	styles[HTML_H7] = HTStyleNamed(styleSheet, "Heading7");

	styles[HTML_DL] = HTStyleNamed(styleSheet, "Glossary");
	styles[HTML_UL] = styles[HTML_OL] = HTStyleNamed(styleSheet, "List");
	styles[HTML_MENU] = HTStyleNamed(styleSheet, "Menu");
	styles[HTML_DIR] = HTStyleNamed(styleSheet, "Dir");
	styles[HTML_DLC] = HTStyleNamed(styleSheet, "GlossaryCompact");
	styles[HTML_ADDRESS] = HTStyleNamed(styleSheet, "Address");
	styles[HTML_BLOCKQUOTE] = HTStyleNamed(styleSheet, "BlockQuote");
	styles[HTML_PLAINTEXT] = styles[HTML_XMP] = HTStyleNamed(
			styleSheet, "Example");
	styles[HTML_PRE] = HTStyleNamed(styleSheet, "Preformatted");
	styles[HTML_LISTING] = HTStyleNamed(styleSheet, "Listing");
}
/*				P U B L I C
*/

/*	Structured Object Class
**	-----------------------
*/
const HTStructuredClass HTMLPresentation = /* As opposed to print etc */
		{
				"text/html", HTML_free, HTML_abort, HTML_put_character,
				HTML_put_string, HTML_write, HTML_start_element,
				HTML_end_element, HTML_put_entity };


/*		New Structured Text object
**		--------------------------
**
**	The strutcured stream can generate either presentation,
**	or plain text, or HTML.
*/
HTStructured*
HTML_new (HTParentAnchor * anchor, HTFormat format_out, HTStream*
			   stream) {

	HTStructured* me;

	if(format_out != WWW_PLAINTEXT && format_out != WWW_PRESENT) {
		HTStream* intermediate = HTStreamStack(
				WWW_HTML, format_out, stream, anchor);
		if(intermediate) return HTMLGenerator(intermediate);
		fprintf(
				stderr, "** Internal error: can't parse HTML to %s\n",
				HTAtom_name(format_out));
		exit(-99);
	}

	me = malloc(sizeof(*me));
	if(me == NULL) outofmem(__FILE__, "HTML_new");

	if(!got_styles) get_styles();

	me->isa = &HTMLPresentation;
	me->node_anchor = anchor;
	me->title.size = 0;
	me->title.growby = 128;
	me->title.allocated = 0;
	me->title.data = 0;
	me->text = 0;
	me->style_change = HT_TRUE; /* Force check leading to text creation */
	me->new_style = default_style;
	me->old_style = 0;
	me->sp = me->stack + MAX_NESTING - 1;
	me->sp->tag_number = -1;                /* INVALID */
	me->sp->style = default_style;            /* INVALID */

	me->comment_start = NULL;
	me->comment_end = NULL;
	me->target = stream;
	if(stream) me->targetClass = *stream->isa;    /* Copy pointers */

	return (HTStructured*) me;
}


/*	HTConverter for HTML to plain text
**	----------------------------------
**
**	This will convert from HTML to presentation or plain text.
*/
HTStream*
HTMLToPlain (HTPresentation * pres, HTParentAnchor * anchor, HTStream *
				  sink) {
	return SGML_new(&HTML_dtd, HTML_new(anchor, pres->rep_out, sink));
}


/*	HTConverter for HTML to C code
**	------------------------------
**
**	C copde is like plain text but all non-preformatted code
**	is commented out.
**	This will convert from HTML to presentation or plain text.
*/
HTStream*
HTMLToC (HTPresentation * pres, HTParentAnchor * anchor, HTStream *
			  sink) {

	HTStructured* html;

	(void) pres;

	(*sink->isa->put_string)(sink, "/* ");    /* Before even title */
	html = HTML_new(anchor, WWW_PLAINTEXT, sink);
	html->comment_start = "/* ";
	html->comment_end = " */\n";    /* Must start in col 1 for cpp */
/*    HTML_put_string(html,html->comment_start); */
	return SGML_new(&HTML_dtd, html);
}


/*	Presenter for HTML
**	------------------
**
**	This will convert from HTML to presentation or plain text.
**
**	Override this if you have a windows version
*/
#ifndef GUI
HTStream*
HTMLPresent (HTPresentation * pres, HTParentAnchor * anchor, HTStream *
				  sink) {
	(void) pres;

	return SGML_new(&HTML_dtd, HTML_new(anchor, WWW_PRESENT, sink));
}

#endif


/*	Record error message as a hypertext object
**	------------------------------------------
**
**	The error message should be marked as an error so that
**	it can be reloaded later.
**	This implementation just throws up an error message
**	and leaves the document unloaded.
**	A smarter implementation would load an error document,
**	marking at such so that it is retried on reload.
**
** On entry,
**	sink 	is a stream to the output device if any
**	number	is the HTTP error number
**	message	is the human readable message.
**
** On exit,
**	returns	a negative number to indicate lack of success in the load.
*/

int
HTLoadError (HTStream * sink, int number, const char* message) {
	(void) sink;

	HTAlert(message);        /* @@@@@@@@@@@@@@@@@@@ */
	return -number;
} 


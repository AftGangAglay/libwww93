/*			General SGML Parser code		SGML.c
**			========================
**
**	This module implements an HTStream object. To parse an
**	SGML file, create this object which is a parser. The object
**	is (currently) created by being passed a DTD structure,
**	and a target HTStructured oject at which to throw the parsed stuff.
**	
**	 6 Feb 93  Binary seraches used. Intreface modified.
*/

#include <SGML.h>

#include <HTUtils.h>
#include <HTChunk.h>
#include <HTSTD.h>

#define INVALID (-1)

/*	The State (context) of the parser
**
**	This is passed with each call to make the parser reentrant
**
*/

#define MAX_ATTRIBUTES 20    /* Max number of attributes per element */


/*		Element Stack
**		-------------
**	This allows us to return down the stack reselcting styles.
**	As we return, attribute values will be garbage in general.
*/
typedef struct _HTElement HTElement;
struct _HTElement {
	HTElement* next;    /* Previously nested element or 0 */
	HTTag* tag;    /* The tag at this level  */
};


/*	Internal Context Data Structure
**	-------------------------------
*/
struct _HTStream {

	const HTStreamClass* isa;        /* inherited from HTStream */

	const SGML_dtd* dtd;
	HTStructuredClass* actions;    /* target class  */
	HTStructured* target;    /* target object */

	HTTag* current_tag;
	int current_attribute_number;
	HTChunk* string;
	HTElement* element_stack;
	enum sgml_state {
		S_text,
		S_literal,
		S_tag,
		S_tag_gap,
		S_attr,
		S_attr_gap,
		S_equals,
		S_value,
		S_ero,
		S_cro,
		S_squoted,
		S_dquoted,
		S_end,
		S_entity,
		S_junk_tag
	} state;
#ifdef CALLERDATA
	void *		callerData;
#endif
	HTBool present[MAX_ATTRIBUTES];    /* Flags: attribute is present? */
	char* value[MAX_ATTRIBUTES];    /* malloc'd strings or NULL if none */
};


#define PUTC(ch) ((*context->actions->put_character)(context->target, ch))



/*	Handle Attribute
**	----------------
*/
/* const char * SGML_default = "";   ?? */

static void handle_attribute_name(HTStream* context, const char* s) {

	HTTag* tag = context->current_tag;
	attr* attributes = tag->attributes;

	int high, low, i, diff;        /* Binary search for attribute name */
	for(low = 0, high = tag->number_of_attributes; high > low;
			diff < 0 ? (low = i + 1) : (high = i)) {
		i = (low + (high - low) / 2);
		diff = strcasecomp(attributes[i].name, s);
		if(diff == 0) {            /* success: found it */
			context->current_attribute_number = i;
			context->present[i] = HT_TRUE;
			if(context->value[i]) {
				free(context->value[i]);
				context->value[i] = NULL;
			}
			return;
		} /* if */

	} /* for */

	if(TRACE) {
		fprintf(
				stderr, "SGML: Unknown attribute %s for tag %s\n", s,
				context->current_tag->name);
	}
	context->current_attribute_number = INVALID;    /* Invalid */
}


/*	Handle attribute value
**	----------------------
*/
static void handle_attribute_value(HTStream* context, const char* s) {
	if(context->current_attribute_number != INVALID) {
		StrAllocCopy(context->value[context->current_attribute_number], s);
	}
	else {
		if(TRACE) fprintf(stderr, "SGML: Attribute value %s ignored\n", s);
	}
	context->current_attribute_number = INVALID; /* can't have two assignments! */
}


/*	Handle entity
**	-------------
**
** On entry,
**	s	contains the entity name zero terminated
** Bugs:
**	If the entity name is unknown, the terminator is treated as
**	a printable non-special character in all cases, even if it is '<'
*/

static void handle_entity(HTStream* context, char term) {

	const char** entities = context->dtd->entity_names;
	const char* s = context->string->data;

	int high, low, i, diff;
	for(low = 0, high = context->dtd->number_of_entities; high > low;
			diff < 0 ? (low = i + 1) : (high = i)) {  /* Binary serach */
		i = (low + (high - low) / 2);
		diff = strcmp(entities[i], s);    /* Csse sensitive! */
		if(diff == 0) {            /* success: found it */
			(*context->actions->put_entity)(context->target, i);
			return;
		}
	}
	/* If entity string not found, display as text */
	if(TRACE) {
		fprintf(stderr, "SGML: Unknown entity %s\n", s);
	}
	PUTC('&');
	{
		const char* p;
		for(p = s; *p; p++) {
			PUTC(*p);
		}
	}
	PUTC(term);
}


/*	End element
**	-----------
*/
static void end_element(HTStream* context, HTTag* old_tag) {
	if(TRACE) fprintf(stderr, "SGML: End   </%s>\n", old_tag->name);
	if(old_tag->contents == SGML_EMPTY) {
		if(TRACE) {
			fprintf(
					stderr, "SGML: Illegal end tag </%s> found.\n",
					old_tag->name);
		}
		return;
	}
	while(context->element_stack) {/* Loop is error path only */
		HTElement* N = context->element_stack;
		HTTag* t = N->tag;

		if(old_tag != t) {        /* Mismatch: syntax error */
			if(context->element_stack
					->next) {    /* This is not the last level */
				if(TRACE) {
					fprintf(
							stderr,
							"SGML: Found </%s> when expecting </%s>. </%s> assumed.\n",
							old_tag->name, t->name, t->name);
				}
			}
			else {            /* last level */
				if(TRACE) {
					fprintf(
							stderr,
							"SGML: Found </%s> when expecting </%s>. </%s> Ignored.\n",
							old_tag->name, t->name, old_tag->name);
				}
				return;            /* Ignore */
			}
		}

		context->element_stack = N->next;        /* Remove from stack */
		free(N);
		(*context->actions->end_element)(
				context->target, t - context->dtd->tags);
		if(old_tag == t) return;  /* Correct sequence */

		/* Syntax error path only */

	}
	if(TRACE) {
		fprintf(
				stderr, "SGML: Extra end tag </%s> found and ignored.\n",
				old_tag->name);
	}
}


/*	Start a element
*/
static void start_element(HTStream* context) {
	HTTag* new_tag = context->current_tag;

	if(TRACE) fprintf(stderr, "SGML: Start <%s>\n", new_tag->name);
	(*context->actions->start_element)(
			context->target, new_tag - context->dtd->tags, context->present,
			(const char**) context->value);  /* coerce type for think c */
	if(new_tag->contents != SGML_EMPTY) {        /* i.e. tag not empty */
		HTElement* N = malloc(sizeof(HTElement));
		if(N == NULL) HTOOM(__FILE__, "start_element");
		N->next = context->element_stack;
		N->tag = new_tag;
		context->element_stack = N;
	}
}


/*		Find Tag in DTD tag list
**		------------------------
**
** On entry,
**	dtd	points to dtd structire including valid tag list
**	string	points to name of tag in question
**
** On exit,
**	returns:
**		NULL		tag not found
**		else		address of tag structure in dtd
*/
static HTTag* find_tag(const SGML_dtd* dtd, char* string) {
	int high, low, i, diff;
	for(low = 0, high = dtd->number_of_tags; high > low;
			diff < 0 ? (low = i + 1) : (high = i)) {  /* Binary serach */
		i = (low + (high - low) / 2);
		diff = strcasecomp(dtd->tags[i].name, string);    /* Case insensitive */
		if(diff == 0) {            /* success: found it */
			return &dtd->tags[i];
		}
	}
	return 0;
}

/*________________________________________________________________________
**			Public Methods
*/


/*	Could check that we are back to bottom of stack! @@  */

void SGML_free(HTStream* context) {
	int i;
	(*context->actions->free)(context->target);
	HTChunkFree(context->string);
	for(i = 0; i < MAX_ATTRIBUTES; ++i) free(context->value[i]);
	free(context);
}

void SGML_abort(HTStream* context, HTError e) {
	(*context->actions->abort)(context->target, e);
	HTChunkFree(context->string);
	free(context);
}


/*	Read and write user callback handle
**	-----------------------------------
**
**   The callbacks from the SGML parser have an SGML context parameter.
**   These calls allow the caller to associate his own context with a
**   particular SGML context.
*/

#ifdef CALLERDATA
void* SGML_callerData ARGS1(HTStream * context)
{
	return context->callerData;
}

void SGML_setCallerData ARGS2(HTStream * context, void* data)
{
	context->callerData = data;
}
#endif

void SGML_character(HTStream* context, char c) {
	const SGML_dtd* dtd = context->dtd;
	HTChunk* string = context->string;

	switch(context->state) {
		case S_text:
			if(c == '&' && (!context->element_stack ||
							(context->element_stack->tag &&
							 (context->element_stack->tag->contents ==
							  SGML_MIXED ||
							  context->element_stack->tag->contents ==
							  SGML_RCDATA)))) {
				string->size = 0;
				context->state = S_ero;

			}
			else if(c == '<') {
				string->size = 0;
				context->state = (context->element_stack &&
								  context->element_stack->tag &&
								  context->element_stack->tag->contents ==
								  SGML_LITERAL) ? S_literal : S_tag;
			}
			else
				PUTC(c);
			break;

/*	In literal mode, waits only for specific end tag!
**	Only for compatibility with old servers.
*/
		case S_literal : HTChunkPutc(string, c);
			if(toupper(c) !=
			   ((string->size == 1) ? '/' : context->element_stack->tag->name[
					   string->size - 2])) {
				int i;

				/*	If complete match, end literal */
				if((c == '>') &&
				   (!context->element_stack->tag->name[string->size - 2])) {
					end_element(context, context->element_stack->tag);
					string->size = 0;
					context->current_attribute_number = INVALID;
					context->state = S_text;
					break;
				}        /* If Mismatch: recover string. */
				PUTC('<');
				for(i = 0; i < string->size; i++)    /* recover */
					PUTC(string->data[i]);
				context->state = S_text;
			}

			break;

/*	Character reference or Entity
*/
		case S_ero:
			if(c == '#') {
				context->state = S_cro;  /*   &# is Char Ref Open */
				break;
			}
			context->state = S_entity;    /* Fall through! */

/*	Handle Entities
*/
		case S_entity:
			if(isalnum(c)) {
				HTChunkPutc(string, c);
			}
			else {
				HTChunkTerminate(string);
				handle_entity(context, c);
				context->state = S_text;
			}
			break;

/*	Character reference
*/
		case S_cro:
			if(isalnum(c)) {
				HTChunkPutc(string, c);    /* accumulate a character NUMBER */
			}
			else {
				int value;
				HTChunkTerminate(string);
				if(sscanf(string->data, "%d", &value) == 1)
					PUTC(((char) value));
				context->state = S_text;
			}
			break;

/*		Tag
*/
		case S_tag:                /* new tag */
			if(isalnum(c)) {
				HTChunkPutc(string, c);
			}
			else {                /* End of tag name */
				HTTag* t;
				if(c == '/') {
					if(TRACE) {
						if(string->size != 0) {
							fprintf(
									stderr, "SGML:  `<%s/' found!\n",
									string->data);
						}
					}
					context->state = S_end;
					break;
				}
				HTChunkTerminate(string);

				t = find_tag(dtd, string->data);
				if(!t) {
					if(TRACE) {
						fprintf(
								stderr, "SGML: *** Unknown element %s\n",
								string->data);
					}
					context->state = (c == '>') ? S_text : S_junk_tag;
					break;
				}
				context->current_tag = t;

				/*  Clear out attributes
				*/

				{
					int i;
					for(i = 0; i < context->current_tag->number_of_attributes;
							i++) {
						context->present[i] = HT_FALSE;
					}
				}
				string->size = 0;
				context->current_attribute_number = INVALID;

				if(c == '>') {
					if(context->current_tag->name) start_element(context);
					context->state = S_text;
				}
				else {
					context->state = S_tag_gap;
				}
			}
			break;


		case S_tag_gap:        /* Expecting attribute or > */
			if(HT_WHITE(c)) break;    /* Gap between attributes */
			if(c == '>') {        /* End of tag */
				if(context->current_tag->name) start_element(context);
				context->state = S_text;
				break;
			}
			HTChunkPutc(string, c);
			context->state = S_attr;        /* Get attribute */
			break;

			/* accumulating value */
		case S_attr:
			if(HT_WHITE(c) || (c == '>') ||
			   (c == '=')) {        /* End of word */
				HTChunkTerminate(string);
				handle_attribute_name(context, string->data);
				string->size = 0;
				if(c == '>') {        /* End of tag */
					if(context->current_tag->name) start_element(context);
					context->state = S_text;
					break;
				}
				context->state = (c == '=' ? S_equals : S_attr_gap);
			}
			else {
				HTChunkPutc(string, c);
			}
			break;

		case S_attr_gap:        /* Expecting attribute or = or > */
			if(HT_WHITE(c)) break;    /* Gap after attribute */
			if(c == '>') {        /* End of tag */
				if(context->current_tag->name) start_element(context);
				context->state = S_text;
				break;
			}
			else if(c == '=') {
				context->state = S_equals;
				break;
			}
			HTChunkPutc(string, c);
			context->state = S_attr;        /* Get next attribute */
			break;

		case S_equals:            /* After attr = */
			if(HT_WHITE(c)) break;    /* Before attribute value */
			if(c == '>') {        /* End of tag */
				if(TRACE) fprintf(stderr, "SGML: found = but no value\n");
				if(context->current_tag->name) start_element(context);
				context->state = S_text;
				break;

			}
			else if(c == '\'') {
				context->state = S_squoted;
				break;

			}
			else if(c == '"') {
				context->state = S_dquoted;
				break;
			}
			HTChunkPutc(string, c);
			context->state = S_value;
			break;

		case S_value:
			if(HT_WHITE(c) || (c == '>')) {        /* End of word */
				HTChunkTerminate(string);
				handle_attribute_value(context, string->data);
				string->size = 0;
				if(c == '>') {        /* End of tag */
					if(context->current_tag->name) start_element(context);
					context->state = S_text;
					break;
				}
				else { context->state = S_tag_gap; }
			}
			else {
				HTChunkPutc(string, c);
			}
			break;

		case S_squoted:        /* Quoted attribute value */
			if(c == '\'') {        /* End of attribute value */
				HTChunkTerminate(string);
				handle_attribute_value(context, string->data);
				string->size = 0;
				context->state = S_tag_gap;
			}
			else {
				HTChunkPutc(string, c);
			}
			break;

		case S_dquoted:        /* Quoted attribute value */
			if(c == '"') {        /* End of attribute value */
				HTChunkTerminate(string);
				handle_attribute_value(context, string->data);
				string->size = 0;
				context->state = S_tag_gap;
			}
			else {
				HTChunkPutc(string, c);
			}
			break;

		case S_end:                    /* </ */
			if(isalnum(c)) {
				HTChunkPutc(string, c);
			}
			else {                /* End of end tag name */
				HTTag* t;
				HTChunkTerminate(string);
				if(!*string->data) {    /* Empty end tag */
					t = context->element_stack->tag;
				}
				else {
					t = find_tag(dtd, string->data);
				}
				if(!t) {
					if(TRACE) {
						fprintf(
								stderr, "Unknown end tag </%s>\n",
								string->data);
					}
				}
				else {
					context->current_tag = t;
					end_element(context, context->current_tag);
				}

				string->size = 0;
				context->current_attribute_number = INVALID;
				if(c != '>') {
					if(TRACE && !HT_WHITE(c)) {
						fprintf(
								stderr, "SGML:  `</%s%c' found!\n",
								string->data, c);
					}
					context->state = S_junk_tag;
				}
				else {
					context->state = S_text;
				}
			}
			break;


		case S_junk_tag:
			if(c == '>') {
				context->state = S_text;
			}

	} /* switch on context->state */

}  /* SGML_character */


void SGML_string(HTStream* context, const char* str) {
	const char* p;
	for(p = str; *p; p++) {
		SGML_character(context, *p);
	}
}


void SGML_write(HTStream* context, const char* str, int l) {
	const char* p;
	const char* e = str + l;
	for(p = str; p < e; p++) {
		SGML_character(context, *p);
	}
}

/*_______________________________________________________________________
*/

/*	Structured Object Class
**	-----------------------
*/
const HTStreamClass SGMLParser = {
		"SGMLParser", SGML_free, SGML_abort, SGML_character, SGML_string,
		SGML_write, };

/*	Create SGML Engine
**	------------------
**
** On entry,
**	dtd		represents the DTD, along with
**	actions		is the sink for the data as a set of routines.
**
*/

HTStream* SGML_new(const SGML_dtd* dtd, HTStructured* target) {
	int i;
	HTStream* context = malloc(sizeof(*context));
	if(!context) HTOOM(__FILE__, "SGML_begin");

	context->isa = &SGMLParser;
	context->string = HTChunkCreate(128);    /* Grow by this much */
	context->dtd = dtd;
	context->target = target;
	context->actions = (HTStructuredClass*) (((HTStream*) target)->isa);
	/* Ugh: no OO */
	context->state = S_text;
	context->element_stack = 0;            /* empty */
#ifdef CALLERDATA
	context->callerData = (void*) callerData;
#endif
	for(i = 0; i < MAX_ATTRIBUTES; i++) context->value[i] = 0;

	return context;
}


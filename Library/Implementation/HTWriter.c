/*		FILE WRITER			HTWrite.c
**		===========
**
*/
#include <HTWriter.h>

#define BUFFER_SIZE 4096    /* Tradeoff */

#include <HTUtils.h>
#include <HTSTD.h>


/*		HTML Object
**		-----------
*/

struct _HTStream {
	const HTStreamClass* isa;

	int soc;
	char* write_pointer;
	char buffer[BUFFER_SIZE];
#ifdef NOT_ASCII
	HTBool	make_ascii;	/* Are we writing to the net? */
#endif
};


/*	Write the buffer out to the socket
**	----------------------------------
*/

static void flush(HTStream* me) {
	char* read_pointer = me->buffer;
	char* write_pointer = me->write_pointer;

#ifdef NOT_ASCCII
	if (me->make_ascii) {
		char * p;
	for(p = me->buffer; p < me->write_pointer; p++)
		*p = (*p);
	}
#endif
	while(read_pointer < write_pointer) {
		int status;

		status = write(
				me->soc, me->buffer,
				(unsigned) (write_pointer - read_pointer));

		if(status < 0) {
			if(TRACE) {
				fprintf(
						stderr,
						"HTWrite: Error: write() on socket returns %d !!!\n",
						status);
			}
			return;
		}
		read_pointer = read_pointer + status;
	}
	me->write_pointer = me->buffer;
}


/*_________________________________________________________________________
**
**			A C T I O N 	R O U T I N E S
*/

/*	Character handling
**	------------------
*/

static void HTWriter_put_character(HTStream* me, char c) {
	if(me->write_pointer == &me->buffer[BUFFER_SIZE]) flush(me);
	*me->write_pointer++ = c;
}


/*	String handling
**	---------------
**
**	Strings must be smaller than this buffer size.
*/
static void HTWriter_put_string(HTStream* me, const char* s) {
	int l = (int) strlen(s);
	if(me->write_pointer + l > &me->buffer[BUFFER_SIZE]) flush(me);
	strcpy(me->write_pointer, s);
	me->write_pointer = me->write_pointer + l;
}


/*	Buffer write. Buffers can (and should!) be big.
**	------------
*/
static void HTWriter_write(HTStream* me, const char* s, int l) {

	const char* read_pointer = s;
	const char* write_pointer = s + l;

	flush(me);        /* First get rid of our buffer */

	while(read_pointer < write_pointer) {
		int status = write(me->soc, read_pointer,
						   (unsigned) (write_pointer - read_pointer));
		if(status < 0) {
			if(TRACE) {
				fprintf(
						stderr,
						"HTWriter_write: Error on socket output stream!!!\n");
			}
			return;
		}
		read_pointer = read_pointer + status;
	}
}


/*	Free an HTML object
**	-------------------
**
**	Note that the SGML parsing context is freed, but the created object is not,
**	as it takes on an existence of its own unless explicitly freed.
*/
static void HTWriter_free(HTStream* me) {
	flush(me);
	close(me->soc);
	free(me);
}

static void HTWriter_abort(HTStream* me, HTError e) {
	(void) e;

	HTWriter_free(me);
}


/*	Structured Object Class
**	-----------------------
*/
static const HTStreamClass HTWriter = /* As opposed to print etc */
		{
				"SocketWriter", HTWriter_free, HTWriter_abort,
				HTWriter_put_character, HTWriter_put_string, HTWriter_write };


/*	Subclass-specific Methods
**	-------------------------
*/

HTStream* HTWriter_new(int soc) {
	HTStream* me = malloc(sizeof(*me));
	if(me == NULL) HTOOM(__FILE__, "HTML_new");
	me->isa = &HTWriter;

#ifdef NOT_ASCII
	me->make_ascii = HT_FALSE;
#endif
	me->soc = soc;
	me->write_pointer = me->buffer;
	return me;
}

/*	Subclass-specific Methods
**	-------------------------
*/

HTStream* HTASCIIWriter(int soc) {
	HTStream* me = malloc(sizeof(*me));
	if(me == NULL) HTOOM(__FILE__, "HTML_new");
	me->isa = &HTWriter;

#ifdef NOT_ASCII
	me->make_ascii = HT_TRUE;
#endif
	me->soc = soc;
	me->write_pointer = me->buffer;
	return me;
}


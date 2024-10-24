/*                                                      The Stream class definition -- libwww
                                 STREAM OBJECT DEFINITION
                                             
   A Stream object is something which accepts a stream of text.
   
   The creation methods will vary on the type of Stream Object.   All creation methods
   return a pointer to the stream type below.
   
   As you can see, but the methods used to write to the stream and close it are pointed to
   be the object itself.
   
 */
#ifndef HTSTREAM_H
#define HTSTREAM_H

#include <HTUtils.h>

typedef struct _HTStream HTStream;

/*

   These are the common methods of all streams. They should be self-explanatory, except
   for end_document which must be called before free. It should be merged with free in
   fact:  it should be dummy for new streams.
   
   The put_block method was write, but this upset systems whiuch had macros for write().
   
 */
typedef struct _HTStreamClass {

	char* name;                            /* Just for diagnostics */

	void (* free)(HTStream* me);

	void (* abort)(HTStream* me, HTError e);

	void (* put_character)(HTStream* me, char ch);

	void (* put_string)(HTStream* me, const char* str);

	void (* put_block)(
			HTStream* me, const char* str, int len);


} HTStreamClass;

#endif /* HTSTREAM_H */

/*

   end of HTStream.h */

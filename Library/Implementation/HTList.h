/*  */

/*              List object
**
**      The list object is a generic container for storing collections
**      of things in order.
*/
#ifndef HTLIST_H
#define HTLIST_H

#include "HTUtils.h"  /* for HTBool type and PARAMS and ARGS*/

typedef struct _HTList HTList;

struct _HTList {
	void* object;
	HTList* next;
};


HTList* HTList_new (void);

void HTList_delete (HTList * me);

/*      Add object to START of list
*/
void HTList_addObject (HTList * me, void* newObject);


HTBool HTList_removeObject (HTList * me, void* oldObject);

void* HTList_removeLastObject (HTList * me);

void* HTList_removeFirstObject (HTList * me);

#define         HTList_isEmpty(me) (me ? me->next == NULL : HT_TRUE)

int HTList_count (HTList * me);

int HTList_indexOf (HTList * me, void* object);

#define         HTList_lastObject(me) \
  (me && me->next ? me->next->object : NULL)

void* HTList_objectAt (HTList * me, int position);

/* Fast macro to traverse the list. Call it first with copy of list header :
   it returns the first object and increments the passed list pointer.
   Call it with the same variable until it returns NULL. */
#define HTList_nextObject(me) \
  (me && (me = me->next) ? me->object : NULL)

#endif /* HTLIST_H */
/*

    */

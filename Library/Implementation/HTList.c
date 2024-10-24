/*	A small List class					      HTList.c
**	==================
**
**	A list is represented as a sequence of linked nodes of type HTList.
**	The first node is a header which contains no object.
**	New nodes are inserted between the header and the rest of the list.
*/

#include <HTList.h>
#include <HTSTD.h>

HTList* HTList_new(void) {
	HTList* newList = malloc(sizeof(HTList));
	if(newList == NULL) HTOOM(__FILE__, "HTList_new");
	newList->object = NULL;
	newList->next = NULL;
	return newList;
}

void HTList_delete(HTList* me) {
	HTList* current;
	while((current = me)) {
		me = me->next;
		free(current);
	}
}

void HTList_addObject(HTList* me, void* newObject) {
	if(me) {
		HTList* newNode = malloc(sizeof(HTList));
		if(newNode == NULL) HTOOM(__FILE__, "HTList_addObject");
		newNode->object = newObject;
		newNode->next = me->next;
		me->next = newNode;
	}
	else if(TRACE) {
		fprintf(
				stderr,
				"HTList: Trying to add object %p to a nonexisting list\n",
				newObject);
	}
}

HTBool HTList_removeObject(HTList* me, void* oldObject) {
	if(me) {
		HTList* previous;
		while(me->next) {
			previous = me;
			me = me->next;
			if(me->object == oldObject) {
				previous->next = me->next;
				free(me);
				return HT_TRUE;  /* Success */
			}
		}
	}
	return HT_FALSE;  /* object not found or NULL list */
}

void* HTList_removeLastObject(HTList* me) {
	if(me && me->next) {
		HTList* lastNode = me->next;
		void* lastObject = lastNode->object;
		me->next = lastNode->next;
		free(lastNode);
		return lastObject;
	}
	else {  /* Empty list */
		return 0;
	}
}

void* HTList_removeFirstObject(HTList* me) {
	if(me && me->next) {
		HTList* prevNode = 0;
		void* firstObject;
		while(me->next) {
			prevNode = me;
			me = me->next;
		}
		firstObject = me->object;
		prevNode->next = NULL;
		free(me);
		return firstObject;
	}
	else {  /* Empty list */
		return 0;
	}
}

int HTList_count(HTList* me) {
	int count = 0;
	if(me) {
		while((me = me->next))
			count++;
	}
	return count;
}

int HTList_indexOf(HTList* me, void* object) {
	if(me) {
		int position = 0;
		while((me = me->next)) {
			if(me->object == object) {
				return position;
			}
			position++;
		}
	}
	return -1;  /* Object not in the list */
}

void* HTList_objectAt(HTList* me, int position) {
	if(position < 0) {
		return 0;
	}
	if(me) {
		while((me = me->next)) {
			if(position == 0) {
				return me->object;
			}
			position--;
		}
	}
	return 0;  /* Reached the end of the list */
}

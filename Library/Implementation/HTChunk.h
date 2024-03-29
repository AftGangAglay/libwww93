/*                  /Net/dxcern/userd/timbl/hypertext/WWW/Library/Implementation/HTChunk.html
                                     CHUNK HANDLING:
                                     FLEXIBLE ARRAYS
                                             
   This module implements a flexible array. It is a general utility module. A chunk is a
   structure which may be extended. These routines create and append data to chunks,
   automatically reallocating them as necessary.
   
 */
typedef struct {
	int size;           /* In bytes                     */
	int growby;         /* Allocation unit in bytes     */
	int allocated;      /* Current size of *data        */
	char* data;           /* Pointer to malloced area or 0 */
} HTChunk;


/*

Create new chunk

  ON ENTRY,
  
  growby                  The number of bytes to allocate at a time when the chunk is
                         later extended. Arbitrary but normally a trade-off time vs.
                         memory
                         
  ON EXIT,
  
  returns                 A chunk pointer to the new chunk,
                         
 */

HTChunk* HTChunkCreate(int growby);


/*

Free a chunk

  ON ENTRY,
  
  ch                      A valid chunk pointer made by HTChunkCreate()
                         
  ON EXIT,
  
  ch                      is invalid and may not be used.
                         
 */

void HTChunkFree(HTChunk* ch);


/*

Clear a chunk

  ON ENTRY,
  
  ch                      A valid chunk pointer made by HTChunkCreate()
                         
  ON EXIT,
  
  *ch                     The size of the chunk is zero.
                         
 */

void HTChunkClear(HTChunk* ch);


/*

Ensure a chunk has a certain space in

  ON ENTRY,
  
  ch                      A valid chunk pointer made by HTChunkCreate()
                         
  s                       The size required
                         
  ON EXIT,
  
  *ch                     Has size at least s
                         
 */

void HTChunkEnsure(HTChunk* ch, int s);


/*

Append a character to a  chunk

  ON ENTRY,
  
  ch                      A valid chunk pointer made by HTChunkCreate()
                         
  c                      The character to be appended
                         
  ON EXIT,
  
  *ch                    Is one character bigger
                         
 */
void HTChunkPutc(HTChunk* ch, char c);

/*

Append a string to a  chunk

  ON ENTRY,
  
  ch                      A valid chunk pointer made by HTChunkCreate()
                         
  str                     Tpoints to a zero-terminated string to be appended
                         
  ON EXIT,
  
  *ch                     Is bigger by strlen(str)
                         
 */


void HTChunkPuts(HTChunk* ch, const char* str);


/*

Append a zero character to a  chunk

 */

/*

  ON ENTRY,
  
  ch                      A valid chunk pointer made by HTChunkCreate()
                         
  ON EXIT,
  
  *ch                     Is one character bigger
                         
 */


void HTChunkTerminate(HTChunk* ch);

/*

   end  */

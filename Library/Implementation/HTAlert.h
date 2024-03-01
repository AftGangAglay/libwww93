/*                  /Net/dxcern/userd/timbl/hypertext/WWW/Library/Implementation/HTAlert.html
                  DISPLAYING MESSAGES AND GETTING INPUT FOR WWW LIBRARY
                                             
   This module may be overridden for GUI clients.    It allows progress indications and
   warning messages to be communicated to the user in a portable way. It should be used
   for this purpose throughout the library but isn't yet (July 93)
   
      May 92 Created By C.T. Barker
      
      Feb 93 Portablized etc TBL
      
 */
#include "HTUtils.h"
#include "tcp.h"

/*

HTPrompt: Display a message and get the input

  ON ENTRY,
  
  Msg                     String to be displayed.
                         
  ON EXIT,
  
  Return value            is malloc'd string which must be freed.
                         
 */

char* HTPrompt (const char* Msg, const char* deflt);


/*

Display a message, don't wait for input

  ON ENTRY,
  
  Msg                     String to be displayed.
                         
 */

void HTAlert (const char* Msg);


/*

Display a progress message for information (and diagnostics) only

  ON ENTRY,
  
   The input is a list of parameters for printf.
   
 */
void HTProgress (const char* Msg);


/*

Display a message, then wait for 'yes' or 'no'.

  ON ENTRY,
  
  Msg                     String to be displayed
                         
  ON EXIT,
  
  Returns                 If the user reacts in the affirmative, returns TRUE, returns
                         FALSE otherwise.
                         
 */

HTBool HTConfirm (const char* Msg);




/*

    */

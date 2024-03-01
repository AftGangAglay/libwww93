/*                                                     HTML to rich text converter for libwww
                             THE HTML TO RTF OBJECT CONVERTER
                                             
 */
#ifndef HTML_H
#define HTML_H

#include "HTUtils.h"
#include "HTAnchor.h"
#include "HTMLDTD.h"


extern const HTStructuredClass HTMLPresentation;

/*

HTConverter to present HTML

 */
HTStream* HTMLToPlain
		(HTPresentation * pres, HTParentAnchor * anchor, HTStream * sink);

HTStream* HTMLToC
		(HTPresentation * pres, HTParentAnchor * anchor, HTStream * sink);

HTStream* HTMLPresent
		(HTPresentation * pres, HTParentAnchor * anchor, HTStream * sink);

HTStructured* HTML_new(HTParentAnchor * anchor, HTFormat format_out, HTStream * target);

/*

Record error message as a hypertext object

   The error message should be marked as an error so that it can be reloaded later. This
   implementation just throws up an error message and leaves the document unloaded.
   
 */
/* On entry,
**      sink    is a stream to the output device if any
**      number  is the HTTP error number
**      message is the human readable message.
** On exit,
**      a retrun code like HT_LOADED if object exists else 60; 0
*/

int HTLoadError (HTStream * sink,
									  int number,
									  const char* message);

#endif

/*

    */

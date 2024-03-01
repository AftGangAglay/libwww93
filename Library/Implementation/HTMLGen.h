/*  */

/*              HTML generator
*/

#ifndef HTMLGEN_H
#define HTMLGEN_H

#include "HTML.h"
#include "HTStream.h"

/* Subclass:
*/
/* extern const HTStructuredClass HTMLGeneration; */

/* Special Creation:
*/
HTStructured* HTMLGenerator (HTStream * output);

HTStream* HTPlainToHTML
		(HTPresentation * pres, HTParentAnchor * anchor, HTStream * sink);


#endif
/*

    */

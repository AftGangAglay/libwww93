/*                                                           Rich Hypertext object for libWWW
                                  RICH HYPERTEXT OBJECT
                                             
 */

/*

   This is the C interface to the Objective-C (or whatever) HyperText class.
   
 */
#ifndef HTEXT_H
#define HTEXT_H

#include <HTAnchor.h>
#include <HTStyle.h>
#include <HTStream.h>

typedef struct _HText HText;    /* Normal Library */

extern HText* HTMainText;              /* Pointer to current main text */
extern HTParentAnchor* HTMainAnchor;   /* Pointer to current text's anchor */

/*                      Creation and deletion
**
**      Create hypertext object                                 HText_new
*/
HText* HText_new(HTParentAnchor* anchor);

HText* HText_new2(HTParentAnchor* anchor, HTStream* output_stream);

/*      Free hypertext object                                   HText_free
*/
void HText_free(HText* me);


/*                      Object Building methods
**                      -----------------------
**
**      These are used by a parser to build the text in an object
**      HText_beginAppend must be called, then any combination of other
**      append calls, then HText_endAppend. This allows optimised
**      handling using buffers and caches which are flushed at the end.
*/
void HText_beginAppend(HText* text);

void HText_endAppend(HText* text);

/*      Set the style for future text
*/
void HText_setStyle(HText* text, HTStyle* style);

/*      Add one character
*/
void HText_appendCharacter(HText* text, char ch);

/*      Add a zero-terminated string
*/
void HText_appendText(HText* text, const char* str);

/*      New Paragraph
*/
void HText_appendParagraph(HText* text);

/*      Start/end sensitive text
**
** The anchor object is created and passed to HText_beginAnchor.
** The senstive text is added to the text object, and then HText_endAnchor
** is called. Anchors may not be nested.
*/

void HText_beginAnchor(HText* text, HTChildAnchor* anc);

void HText_endAnchor(HText* text);


/*      Dump diagnostics to stderr
*/
void HText_dump(HText* me);

/*      Return the anchor associated with this node
*/
HTParentAnchor* HText_nodeAnchor(HText* me);


/*              Browsing functions
**              ------------------
*/

/*      Bring to front and highlight it
*/

HTBool HText_select(HText* text);

HTBool HText_selectAnchor(HText* text, HTChildAnchor* anchor);

/*              Editing functions
**              -----------------
**
**      These are called from the application. There are many more functions
**      not included here from the orginal text object. These functions
**      NEED NOT BE IMPLEMENTED in a browser which cannot edit.
*/

/*      Style handling:
*/
/*      Apply this style to the selection
*/
void HText_applyStyle(HText* me, HTStyle* style);

/*      Update all text with changed style.
*/
void HText_updateStyle(HText* me, HTStyle* style);

/*      Return style of  selection
*/
HTStyle* HText_selectionStyle(HText* me, HTStyleSheet* sheet);

/*      Paste in styled text
*/
void HText_replaceSel(
		HText* me, const char* aString, HTStyle* aStyle);

/*      Apply this style to the selection and all similarly formatted text
**      (style recovery only)
*/
void HTextApplyToSimilar(HText* me, HTStyle* style);

/*      Select the first unstyled run.
**      (style recovery only)
*/
void HTextSelectUnstyled(HText* me, HTStyleSheet* sheet);


/*      Anchor handling:
*/
void HText_unlinkSelection(HText* me);

HTAnchor* HText_referenceSelected(HText* me);

HTAnchor* HText_linkSelTo(HText* me, HTAnchor* anchor);


#endif /* HTEXT_H */

/*

   end  */

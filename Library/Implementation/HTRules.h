/*                                                           Configuration Manager for libwww
                                  CONFIGURATION MANAGER
                                             
   Author Tim Berners-Lee/CERN. Public domain. Please mail changes to timbl@info.cern.ch.
   
   The configuration information loaded includes tables (file suffixes, presentation
   methods) in other modules. The most likely routines needed by developers will be:
   
  HTSetConfiguration      to load configuration information.
                         
  HTLoadRules             to load a whole file of configuration information
                         
  HTTranslate             to translate a URL using the rule table.
                         
 */
#ifndef HTRULE_H
#define HTRULE_H

#include <HTUtils.h>

typedef enum _HTRuleOp {
	HT_Invalid, HT_Map, HT_Pass, HT_Fail
} HTRuleOp;

/*

HTAddRule:  Add rule to the list

  ON ENTRY,
  
  pattern                points to 0-terminated string containing a single "*"
                         
  equiv                  points to the equivalent string with * for the place where the
                         text matched by * goes.
                         
  ON EXIT,
  
  returns                0 if success, -1 if error.
                         
   Note that if BYTE_ADDRESSING is set, the three blocks required are allocated and
   deallocated as one. This will save time and storage, when malloc's allocation units are
   large.
   
 */
int HTAddRule(HTRuleOp op, const char* pattern, const char* equiv);


/*

HTClearRules: Clear all rules

  ON EXIT,
  
  Rule file               There are no rules
                         
  returns
                          0 if success, -1 if error.
                         
 */

int HTClearRules(void);


/*

HTTranslate: Translate by rules

 */

/*

  ON ENTRY,
  
  required                points to a string whose equivalent value is neeed
                         
  ON EXIT,
  
  returns                 the address of the equivalent string allocated from the heap
                         which the CALLER MUST FREE. If no translation occured, then it is
                         a copy of the original.
                         
 */
char* HTTranslate(const char* required);


/*

HTSetConfiguration:  Load one line of configuration information

  ON ENTRY,
  
  config                  is a string in the syntax of a rule file line.
                         
   This routine may be used for loading configuration information from sources other than
   the  rule file, for example INI files for X resources.
   
 */
int HTSetConfiguration(const char* config);


/*

HtLoadRules:  Load the rules from a file

  ON ENTRY,
  
  Rule table              Rules can be in any state
                         
  ON EXIT,
  
  Rule table              Any existing rules will have been kept. Any new rules will have
                         been loaded on top, so as to be tried first.
                         
  Returns                 0 if no error.
                         
 */

int HTLoadRules(const char* filename);


#endif /* HTUtils.h */

/*

   end  */

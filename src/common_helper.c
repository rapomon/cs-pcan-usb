/* Common helper functions used by both PCAN-Basic and N-API components

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/


#include "common_helper.h"


// ----------------------------------- // -----------------------------------
// Definitions




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables




// ----------------------------------- // -----------------------------------
// Local functions




// ----------------------------------- // -----------------------------------
// Public functions


const char *lookupString(const lookup_t *table, const char* defaultString, int value)
{
    int i;
    const char *returnString = 0;

    for (i = 0; table[i].string; i++)
    {
	if (table[i].value == value)
	{
	    returnString = table[i].string;
	    break;
	}
	else
	{
	    returnString = defaultString;
	}
	// If no match, set to default but continue iterating through table
    }

    // If value does not correspond to any known table element, default
    // string will be returned
    
    return returnString;
}

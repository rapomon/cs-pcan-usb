/* Common helper functions used by both PCAN-Basic and N-API components

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/

#ifndef _COMMON_HELPER_H_
#define _COMMON_HELPER_H_

// ----------------------------------- // -----------------------------------
// Definitions

// Integer constant + string pair for lookup tables
typedef struct
{
    const int value;
    const char* string;
} lookup_t;




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables




// ----------------------------------- // -----------------------------------
// Local functions




// ----------------------------------- // -----------------------------------
// Public functions

// Return pointer to string corresponding to a given integer value and
// strLookup_t table
const char *lookupString(const lookup_t *table, const char *defaultString, int value);




#endif // _COMMON_HELPER_H_


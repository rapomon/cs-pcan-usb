/* NodeJS N-API helper functions

   Functions for viewing N-API statuses, types, and error codes

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/


#ifndef _NAPI_HELPER_H_
#define _NAPI_HELPER_H_

#include <node_api.h> // provide N-API types


// ----------------------------------- // -----------------------------------
// Definitions

// Macro for concisely declaring array of methods passed to
// napi_define_properties
#define DECLARE_NAPI_METHOD(name, func) \
      { name, 0, func, 0, 0, 0, napi_default, 0 }




// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables




// ----------------------------------- // -----------------------------------
// Local functions




// ----------------------------------- // -----------------------------------
// Public functions

// Print properties of a given napi_value to stdout for debugging purposes
void napiDumpValue(napi_env env, napi_value value, char* name);


// Return pointer to string representation of N-API status code, given a status
// code defined in js_native_api_types.h
const char *napiStatusLookup(napi_status status);


// Return pointer to string representation of N-API value type, given a value
// type defined in js_native_api_types.h
const char *napiValuetypeLookup(napi_valuetype valuetype);

// Check for error given a napi_status code returned from an N-API call, printing
// detailed error info to stdout if available
// Returns true if status == napi_ok, and false otherwise
bool napiCheck(napi_env env, napi_status status);

// Print all struct members to stdout given an napi_extended_error_info struct
void napiDumpErrorInfo(const napi_extended_error_info *errorInfo);




#endif // _NAPI_HELPER_H_


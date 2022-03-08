/* NodeJS N-API helper functions

   Functions for viewing N-API statuses, types, and error codes

   Copyright (c) 2020 Control Solutions LLC. All rights reserved.
*/


#include <stdio.h>    // provide printf and snprintf
#include <node_api.h> // provide N-API constants, types, functions
#include <stdint.h>   // provide uintX_t
#include <string.h>   // provide strncpy

#include "common_helper.h" // provide lookupString and lookup_t
#include "napi_helper.h"


// ----------------------------------- // -----------------------------------
// Definitions

const lookup_t napiStatuses[] = {
    { napi_ok,                              "napi_ok" },
    { napi_invalid_arg,                     "napi_invalid_arg" },
    { napi_object_expected,                 "napi_object_expected" },
    { napi_string_expected,                 "napi_string_expected" },
    { napi_name_expected,                   "napi_name_expected" },
    { napi_function_expected,               "napi_function_expected" },
    { napi_number_expected,                 "napi_number_expected" },
    { napi_boolean_expected,                "napi_boolean_expected" },
    { napi_array_expected,                  "napi_array_expected" },
    { napi_generic_failure,                 "napi_generic_failure" },
    { napi_pending_exception,               "napi_pending_exception" },
    { napi_cancelled,                       "napi_cancelled" },
    { napi_escape_called_twice,             "napi_escape_called_twice" },
    { napi_handle_scope_mismatch,           "napi_handle_scope_mismatch" },
    { napi_callback_scope_mismatch,         "napi_callback_scope_mismatch" },
    { napi_queue_full,                      "napi_queue_full" },
    { napi_closing,                         "napi_closing" },
    { napi_bigint_expected,                 "napi_bigint_expected" },
    { napi_date_expected,                   "napi_date_expected" },
    { napi_arraybuffer_expected,            "napi_arraybuffer_expected" },
    { napi_detachable_arraybuffer_expected, "napi_detachable_arraybuffer_expected" },
    { 0, 0 }
};


const char napiStatusUnknown[] = "N-API status unknown";


const lookup_t napiValuetypes[] = {
    { napi_undefined, "napi_undefined" },
    { napi_null,      "napi_null" },
    { napi_boolean,   "napi_boolean" },
    { napi_number,    "napi_number" },
    { napi_string,    "napi_string" },
    { napi_symbol,    "napi_symbol" },
    { napi_object,    "napi_object" },
    { napi_function,  "napi_function" },
    { napi_external,  "napi_external" },
    { napi_bigint,    "napi_bigint" },
    { 0, 0 }
};


const char napiValuetypeUnknown[] = "N-API valuetype unknown";


// ----------------------------------- // -----------------------------------
// Global variables




// ----------------------------------- // -----------------------------------
// Local variables




// ----------------------------------- // -----------------------------------
// Local functions




// ----------------------------------- // -----------------------------------
// Public functions

void napiDumpValue(napi_env env, napi_value value, char* name)
{
    napi_status status = napi_generic_failure;

    // Get value type
    napi_valuetype vt;

    status = napi_typeof(env, value, &vt);
    if (status != napi_ok)
    {
        printf("napiDumpValue: Error at napi_typeof: %s",
               napiStatusLookup(status));
        return;
    }

    // Empty variables to be filled with napi_get_value_* results, depending
    // on the napi_value type
    bool napiValueBool = 0;
    int64_t napiValueInt64 = 0;
    char napiValueStringBuffer[128] = {0};
    size_t napiValueStringResult = 0;
    void* napiValueExternalPtr = 0;

    // Build value string in buffer
    char napiValueBuffer[512] = { 0 };
    
    switch (vt) {
    case napi_undefined:
        snprintf(napiValueBuffer, sizeof(napiValueBuffer), "undefined");
        break;
    case napi_null:
        snprintf(napiValueBuffer, sizeof(napiValueBuffer), "null");
        break;
    case napi_boolean:
        status = napi_get_value_bool(env, value, &napiValueBool);
        if (status != napi_ok)
        {
            printf("napiDumpValue: Error at napi_get_value_bool: %s",
                   napiStatusLookup(status));
        }
        snprintf(napiValueBuffer, sizeof(napiValueBuffer), "%s",
                 napiValueBool ? "true" : "false");
        break;
    case napi_number:
        // Assume int64
        status = napi_get_value_int64(env, value, &napiValueInt64);
        if (status != napi_ok)
        {
            printf("napiDumpValue: Error at napi_get_value_int64: %s",
                   napiStatusLookup(status));
        }
        snprintf(napiValueBuffer, sizeof(napiValueBuffer), "0x%llX = %lli",
                 (uint64_t)napiValueInt64, napiValueInt64);
        break;
    case napi_string:
        status = napi_get_value_string_utf8(env,
                                            value,
                                            napiValueStringBuffer,
                                            sizeof(napiValueStringBuffer),
                                            &napiValueStringResult);
        if (status != napi_ok)
        {
            printf("napiDumpValue: Error at napi_get_string_utf8: %s",
                   napiStatusLookup(status));
        }
        strncpy(napiValueBuffer, napiValueStringBuffer, sizeof(napiValueBuffer));
        break;
    case napi_symbol:
        snprintf(napiValueBuffer, sizeof(napiValueBuffer), "[symbol]");
        break;
    case napi_object:
        snprintf(napiValueBuffer, sizeof(napiValueBuffer), "[object]");
        break;
    case napi_function:
        snprintf(napiValueBuffer, sizeof(napiValueBuffer), "[function]");
        break;
    case napi_external:
        status = napi_get_value_external(env, value, &napiValueExternalPtr);
        if (status != napi_ok)
        {
            printf("napiDumpValue: Error at napi_get_external: %s",
                   napiStatusLookup(status));
        }
        snprintf(napiValueBuffer, sizeof(napiValueBuffer),
                 "%p", napiValueExternalPtr);
        break;
    }

    // Print result
    if (name != 0)
    {
        printf("napiDumpValue: (%s) %s = %s\n",
               napiValuetypeLookup(vt),
               name,
               napiValueBuffer);
    }
    else
    {
        printf("napiDumpValue: (%s) = %s\n",
               napiValuetypeLookup(vt),
               napiValueBuffer);
    }

    return;
}




const char *napiStatusLookup(napi_status status)
{
    const char *statusStr;

    statusStr = lookupString(napiStatuses, napiStatusUnknown, status);

    return statusStr;
}




const char *napiValuetypeLookup(napi_valuetype valuetype)
{
    const char *valuetypeStr;

    valuetypeStr = lookupString(napiValuetypes, napiValuetypeUnknown, valuetype);

    return valuetypeStr;
}




bool napiCheck(napi_env env, napi_status status)
{
    const napi_extended_error_info *errorInfo = NULL;
    bool exceptionPending = false;

    if (status != napi_ok)
    {
        napi_get_last_error_info(env, &errorInfo);
        napi_is_exception_pending(env, &exceptionPending);

        if (!exceptionPending)
        {
            napiDumpErrorInfo(errorInfo);
            napi_throw_error(env, NULL,
                             errorInfo->error_message);
        }
        return false;
    }

    return true;
}




void napiDumpErrorInfo(const napi_extended_error_info *errorInfo)
{
    printf("errorInfo {\n"
           "  error_message = \"%s\"\n"
           "  engine_reserved = %p\n"
           "  engine_error_code = 0x%02X\n"
           "  error_code = 0x%02X\n"
           "  }\n",
           errorInfo->error_message,
           errorInfo->engine_reserved,
           errorInfo->engine_error_code,
           errorInfo->error_code);

    return;
}



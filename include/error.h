#ifndef ERROR_H
#define ERROR_H
#include "common.h"
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    typedef enum
    {
        ERR_ALL_GOOD,
        ERR_INVALID,
        ERR_UNDEFINED,
        ERR_INFALLIBLE,
        ERR_UNEXPECTED,
        ERR_FORBIDDEN,
        ERR_OUT_OF_RANGE,
        ERR_PERMISSION_DENIED,
        ERR_INTERRUPTION,
        ERR_NULL,
        ERR_PARSE_STRING_TO_INT,
        ERR_PARSE_STRING_TO_LONG_INT,
        ERR_PARSE_STRING_TO_FLOAT,
        ERR_EMPTY_STRING,
        ERR_JSON_INVALID,
        ERR_JSON_MISSING_ENTRY,
        ERR_TYPE_MISMATCH,
        ERR_FS_INTERNAL,
        ERR_HTTP_INTERNAL,
        ERR_TCP_INTERNAL,
        ERR_NOT_FOUND,
        ERR_FATAL,
    } Error;

#define is_err(_expr) (_expr != ERR_ALL_GOOD)
#define is_ok(_expr) (_expr == ERR_ALL_GOOD)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ERROR_H */

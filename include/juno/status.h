/*
    MIT License

    Copyright (c) 2025 Robin A. Onsay

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
*/
#ifndef JUNO_STATUS_H
#define JUNO_STATUS_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef int32_t JUNO_STATUS_T;
#define JUNO_STATUS_SUCCESS             0
#define JUNO_STATUS_ERR                 1
#define JUNO_STATUS_NULLPTR_ERROR       2
#define JUNO_STATUS_MEMALLOC_ERROR      3
#define JUNO_STATUS_MEMFREE_ERROR       4
#define JUNO_STATUS_INVALID_TYPE_ERROR  5
#define JUNO_STATUS_INVALID_SIZE_ERROR  6
#define JUNO_STATUS_TABLE_FULL_ERROR    7
#define JUNO_STATUS_DNE_ERROR           8
#define JUNO_STATUS_FILE_ERROR          9
#define JUNO_STATUS_READ_ERROR          10
#define JUNO_STATUS_WRITE_ERROR         11
#define JUNO_STATUS_CRC_ERROR           12
#define JUNO_STATUS_INVALID_REF_ERROR   13
#define JUNO_STATUS_REF_IN_USE_ERROR    14
#define JUNO_STATUS_INVALID_DATA_ERROR  15
#define JUNO_STATUS_TIMEOUT_ERROR       16
#define JUNO_STATUS_OOB                 17

typedef void JUNO_USER_DATA_T;
typedef void (*JUNO_FAILURE_HANDLER_T)(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData);

#ifndef JUNO_FAIL_MESSAGE_LEN
#define JUNO_FAIL_MESSAGE_LEN    256
#endif
#define JUNO_FAIL(tStatus, pfcnFailureHandler, pvFailureUserData, pcMessage)\
if(pfcnFailureHandler){pfcnFailureHandler(tStatus, pcMessage, pvFailureUserData);}

#define JUNO_FAIL_MODULE(tStatus, ptMod, pcMessage)\
if(ptMod && ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER){ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER(tStatus, pcMessage, ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA);}

#define JUNO_FAIL_ROOT(tStatus, ptMod, pcMessage)\
if(ptMod && ptMod->JUNO_FAILURE_HANDLER){ptMod->JUNO_FAILURE_HANDLER(tStatus, pcMessage, ptMod->JUNO_FAILURE_USER_DATA);}

#ifdef __cplusplus
}
#endif
#endif

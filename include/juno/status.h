#ifndef JUNO_STATUS_H
#define JUNO_STATUS_H
#ifdef __cplusplus
extern "C" {
#endif
typedef enum JUNO_STATUS_TAG
{
    JUNO_STATUS_SUCCESS             = 0,
    JUNO_STATUS_ERR                 = 1,
    JUNO_STATUS_NULLPTR_ERROR       = 2,
    JUNO_STATUS_MEMALLOC_ERROR      = 3,
    JUNO_STATUS_MEMFREE_ERROR       = 4,
    JUNO_STATUS_INVALID_TYPE_ERROR  = 5,
    JUNO_STATUS_INVALID_SIZE_ERROR  = 6,
    JUNO_STATUS_TABLE_FULL_ERROR    = 7,
    JUNO_STATUS_DNE_ERROR           = 8,
    JUNO_STATUS_FILE_ERROR          = 9,
    JUNO_STATUS_READ_ERROR          = 10,
    JUNO_STATUS_WRITE_ERROR         = 11,
    JUNO_STATUS_CRC_ERROR           = 12,
    JUNO_STATUS_INVALID_REF_ERROR   = 13,
    JUNO_STATUS_REF_IN_USE_ERROR    = 14,
    JUNO_STATUS_INVALID_DATA_ERROR  = 15,
    JUNO_STATUS_TIMEOUT_ERROR       = 16,
} JUNO_STATUS_T;

typedef void JUNO_USER_DATA_T;
typedef void (*JUNO_FAILURE_HANDLER_T)(JUNO_STATUS_T tStatus, const char *pcCustomMessage, JUNO_USER_DATA_T *pvUserData);

#ifndef FAIL_MESSAGE_LEN
#define FAIL_MESSAGE_LEN    256
#endif
#define FAIL(tStatus, pfcnFailureHandler, pvFailureUserData, pcMessage)\
if(pfcnFailureHandler){pfcnFailureHandler(tStatus, pcMessage, pvFailureUserData);}

#define FAIL_MODULE(tStatus, ptMod, pcMessage)\
if(ptMod && ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER){ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_HANDLER(tStatus, pcMessage, ptMod->JUNO_MODULE_SUPER.JUNO_FAILURE_USER_DATA);}
#ifdef __cplusplus
}
#endif
#endif

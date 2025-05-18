#ifndef JUNO_UDP_API_H
#define JUNO_UDP_API_H

#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include "juno/ipc/udp/udp_types.h"
#include "juno/string/string_types.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
typedef struct JUNO_UDP_API_TAG JUNO_UDP_API_T;

/**
    The Juno UDP API
*/
struct JUNO_UDP_API_TAG
{
    /**
        Initialize the Juno UDP Socket
        @param ptUdp The UDP module to initialize
        @param ptOsData The OS Data structure for this module
        @param pcAddr The address of the UDP socket
        @param iPort The port of this UDP socket
        @param pfcnFailureHdlr The failure handler for this UDP module
        @param pvFailureUserData The user data to pass to the failure handler
        @return Returns `JUNO_STATUS_SUCCESS` if the operations was successful
    */
    JUNO_STATUS_T (*Init)(
        JUNO_UDP_T *ptUdp,
        JUNO_UDP_OS_DATA_T *ptOsData,
        JUNO_STRING_T pcIpAddr,
        JUNO_FAILURE_HANDLER_T pfcnFailureHdlr,
        JUNO_USER_DATA_T *pvFailureUserData
    );
    /**
        Send data via this UDP module. Assumes the module has been initialized
        @param ptUdp The UDP module to use
        @param pcData The data to send
        @param zDataSize The size of the data
        @return Returns `JUNO_STATUS_SUCCESS` if the operations was successful
    */
    JUNO_STATUS_T (*Send)(JUNO_UDP_T *ptUdp, const void *pcData, const size_t zDataSize);
    /**
        Attempt to send data using this module for a set period of time.
        If this timeout is `0` this will return immediatley if it can't send
        @param ptUdp The UDP module to use
        @param pcData The data to send
        @param zDataSize The size of the data
        @return Returns `JUNO_STATUS_SUCCESS` if the operations was successful
    */
    JUNO_STATUS_T (*TrySend)(JUNO_UDP_T *ptUdp, const void *pcData, const size_t zDataSize, const uint32_t iTimeoutMs);
    /**
        Receive data using this module and block until there is data.
        @param ptUdp The UDP module to use
        @param pcData The data to receive
        @param zDataSize The size of the data
        @return Returns `JUNO_STATUS_SUCCESS` if the operations was successful
    */
    JUNO_STATUS_T (*Recv)(JUNO_UDP_T *ptUdp, void *pcData, const size_t zDataSize);
    /**
        Attempt to receive data using this module for a set period of time.
        If this timeout is `0` this will return immediatley if there is no data
        @param ptUdp The UDP module to use
        @param pcData The data to receive
        @param zDataSize The size of the data
        @return Returns `JUNO_STATUS_SUCCESS` if the operations was successful
    */
    JUNO_STATUS_T (*TryRecv)(JUNO_UDP_T *ptUdp, void *pcData, const size_t zDataSize, const uint32_t iTimeoutMs);
    /**
        Free the resources for this module
        @param ptUdp The UDP module to use
        @return Returns `JUNO_STATUS_SUCCESS` if the operations was successful
    */
    JUNO_STATUS_T (*Free)(JUNO_UDP_T *ptUdp);
};

#ifdef __cplusplus
}
#endif
#endif

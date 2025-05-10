#ifndef JUNO_UDP_POSIX_H
#define JUNO_UDP_POSIX_H

#include "juno/ipc/udp/udp_types.h"
#ifdef __cplusplus
extern "C"
{
#endif

struct JUNO_UDP_OS_DATA_TAG
{
    int iSocket;
};

inline JUNO_UDP_OS_DATA_T Juno_UdpPosixCreateOsData()
{
    JUNO_UDP_OS_DATA_T tOsData = {.iSocket = -1};
    return tOsData;
}

#ifdef __cplusplus
}
#endif
#endif

#include "juno/ipc/udp/udp_api.h"
#include "juno/ipc/udp/udp_posix.h"
#include "juno/ipc/udp/udp_types.h"
#include "juno/macros.h"
#include "juno/memory/memory_types.h"
#include "juno/status.h"
#include "juno/string/string_types.h"
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

static inline JUNO_STATUS_T Validate(JUNO_UDP_T *ptUdp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    ASSERT_EXISTS((ptUdp->_ptOsData && ptUdp->_pcIpAddr.pcCStr && ptUdp->_pcIpAddr.zSize));
    return tStatus;
}

static inline JUNO_STATUS_T ParseIpAddr(JUNO_STRING_T pcIpAddr);
static inline JUNO_STATUS_T ParseIpv4Addr(JUNO_STRING_T pcIpAddr);
static inline JUNO_STATUS_T ParseIpv6Addr(JUNO_STRING_T pcIpAddr);

JUNO_STATUS_T Init(
    JUNO_UDP_T *ptUdp,
    JUNO_UDP_OS_DATA_T *ptOsData,
    JUNO_STRING_T pcIpAddr,
    JUNO_FAILURE_HANDLER_T pfcnFailureHdlr,
    JUNO_USER_DATA_T *pvFailureUserData
)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;
    ptUdp->_ptOsData = ptOsData;
    ptUdp->_pcIpAddr = pcIpAddr;
    ptUdp->_pfcnFailureHdlr = pfcnFailureHdlr;
    ptUdp->_pvFailureUserData = pvFailureUserData;
    tStatus = Validate(ptUdp);
    ASSERT_SUCCESS(tStatus,
    {
        FAIL(tStatus, pfcnFailureHdlr, pvFailureUserData,
            "UDP Module has missing dependencies"
        );
    });
    struct addrinfo ptHints = {0}, *ptRet;
    ptHints.ai_family   = AF_UNSPEC;     /* IPv4 or IPv6 */
    ptHints.ai_socktype = SOCK_DGRAM;    /* UDP */
    return tStatus;
}

JUNO_STATUS_T Send(JUNO_UDP_T *ptUdp, const void *pcData, const size_t zDataSize)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;

    return tStatus;
}

JUNO_STATUS_T TrySend(JUNO_UDP_T *ptUdp, const void *pcData, const size_t zDataSize, const uint32_t iTimeoutMs)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;

    return tStatus;
}

JUNO_STATUS_T Recv(JUNO_UDP_T *ptUdp, void *pcData, const size_t zDataSize)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;

    return tStatus;
}

JUNO_STATUS_T TryRecv(JUNO_UDP_T *ptUdp, void *pcData, const size_t zDataSize, const uint32_t iTimeoutMs)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;

    return tStatus;
}

JUNO_STATUS_T Free(JUNO_UDP_T *ptUdp)
{
    JUNO_STATUS_T tStatus = JUNO_STATUS_SUCCESS;

    return tStatus;
}


static const JUNO_UDP_API_T tUdpApi =
{
    .Init = Init,
    .Send = Send,
    .TrySend = TrySend,
    .Recv = Recv,
    .TryRecv = TryRecv,
    .Free = Free
};

const JUNO_UDP_API_T * Juno_UdpPosixApi(void)
{
    return &tUdpApi;
}

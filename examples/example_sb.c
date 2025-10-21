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

#include "juno/sb/broker_api.h"
#include "juno/status.h"
#include "juno/time/time_api.h"
#include <stdint.h>
#include <stdio.h>

/**DOC
# Software Bus Example Overview
In this example we will walkthrough the process of implementing a
LibJuno software bus. See the doxygen documentation for the broker_api
for detailed design documentation on the broker.

# Type Definitions
First we declate the message type definitions that are going to
be flowing over the software bus on this thread. In this example
we have are defining messages that will be sent to and from the engine
application. The engine can be commanded to an RPM and can telemeter
the current measured RPM.

We also set parameters like the Message Identifier (MID) and the
pipe depth, or the maximum number of messages the subscriber will
receive of that type. In this case, the engine application will
receive a maximum of 10 commands during an execution cycle, and
the car's main computer will recveive a maximum of 5 telemetry messages.
*/

typedef struct ENGINE_RPM_CMD_TAG
{
    float fRpm;
} ENGINE_RPM_CMD_T;
JUNO_SB_MID_T RpmCmdMid = 1;

typedef struct ENGINE_RPM_TLM_TAG
{
    float fRpm;
    JUNO_TIMESTAMP_T tTimestamp;
} ENGINE_RPM_TLM_T;
JUNO_SB_MID_T RpmTlmMid = 2;

int main(void)
{
    return 0;
}

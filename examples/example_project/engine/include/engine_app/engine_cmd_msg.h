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

#ifndef ENGINE_CMD_MSG_H
#define ENGINE_CMD_MSG_H
#include "juno/ds/array_api.h"
#include "juno/memory/pointer_api.h"
#include "juno/module.h"
#include "juno/sb/broker_api.h"
#include "juno/status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**DOC
This file is auto-generated from a LibJuno script (scripts/create_msg.py). Similarly the
engine_tlm_msg.h and engine_tlm_msg.c files are also auto-generated. We will only go over the
command file since it's identical to the telemetry file in terms of architecture.

### Engine Application Commands
In LibJuno the software bus is operated on a single thread. A single broker is implemented
for each thread of execution and the broker distributes messages to various queues that
are subscribed on the software bus. These queues are called pipes. Pipes are derived LibJuno
queues with a message ID subscription.

The message ID is a unique identifier for the type of message. Each message type will have its
own message ID. In this example the engine command MID is `0x1800`. This number can be arbitrary.
The only requirement is the MIDs are unique for every message type.
*/
#define ENGINE_CMD_MSG_MID            (0x1800)

/**DOC
The command message contains the RPM. Another application can send this command on the software bus
and tell the engine what RPM it should be set to. The engine app will then control the engine so it's
set to the new RPM.
*/
typedef struct ENGINE_CMD_MSG_TAG
{
    float fRpm;
} ENGINE_CMD_MSG_T;

/**DOC
### Engine Command Pipe
Below is the definition for the engine command pipe. This is derived from the `JUNO_SB_PIPE_T` using the 
`DERIVE_WITH_API` macro. This macro enables users to specify which API they are using.
The inheritance for a pipe is as follows:
```
JUNO_DS_ARRAY_ROOT_T -> JUNO_DS_QUEUE_ROOT_T -> JUNO_SB_PIPE_T -> Custom Pipe Type
```
Because this is a longer chain of inheritance, it's convenient to specify the API for ease-of-use.
In this case we are specifying the QUEUE api.

The pipe holds a pointer to a command buffer, which is specific to the command message type defined 
earlier. This enables the derived pipe to have specific type information (vs using a void pointer in the pipe).
*/
typedef struct ENGINE_CMD_MSG_ARRAY_TAG JUNO_MODULE_DERIVE(JUNO_DS_ARRAY_ROOT_T,
    ENGINE_CMD_MSG_T *ptArrEngineCmdMsgBuffer;
) ENGINE_CMD_MSG_ARRAY_T;

/**DOC
The queue api relies on LibJuno pointers. This enables the modules to write generic code safely without
specific type information. As a result, we need to specify a pointer API implementation for this command type.
*/
extern const JUNO_POINTER_API_T gtEngineCmdMsgPointerApi;

/**DOC
We also define convenience macros for initializing and verifying this type of pointer. This makes working
with LibJuno pointers easy.
*/
#define EngineCmdMsg_PointerInit(addr) JunoMemory_PointerInit(&gtEngineCmdMsgPointerApi, ENGINE_CMD_MSG_T, addr)
#define EngineCmdMsg_PointerVerify(tPointer) JunoMemory_PointerVerifyType(tPointer, ENGINE_CMD_MSG_T, gtEngineCmdMsgPointerApi)

/**DOC
Finally we define a Pipe Init function for this pipe type. This function will
initialize the pipe with the message buffer and capacity.
*/
JUNO_STATUS_T EngineCmdMsg_ArrayInit(ENGINE_CMD_MSG_ARRAY_T *ptEngineCmdMsgPipe, ENGINE_CMD_MSG_T *ptArrEngineCmdMsgBuffer, size_t iCapacity, JUNO_FAILURE_HANDLER_T pfcnFailureHdlr, JUNO_USER_DATA_T *pvUserData);

/**END*/
#ifdef __cplusplus
}
#endif
#endif


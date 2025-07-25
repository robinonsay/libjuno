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

/**
    This API has been generated by LibJuno:
    https://www.robinonsay.com/libjuno/
*/

/**
    This header contains the juno_publisher library API
    @author
*/
#ifndef JUNO_PUBLISHER_API_H
#define JUNO_PUBLISHER_API_H
#include "juno/sb/msg_api.h"
#include "juno/status.h"
#include "juno/module.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct JUNO_PUBLISHER_API_TAG JUNO_PUBLISHER_API_T;
typedef struct JUNO_PUBLISHER_ID_TAG JUNO_PUBLISHER_ID_T;

typedef union JUNO_PUBLISHER_TAG JUNO_PUBLISHER_T;
typedef struct JUNO_PUBLISHER_ROOT_TAG JUNO_PUBLISHER_ROOT_T;

struct JUNO_PUBLISHER_ROOT_TAG JUNO_MODULE_ROOT(JUNO_PUBLISHER_API_T,
    JUNO_PUBLISHER_ID_T *ptPublisherId;
);

struct JUNO_PUBLISHER_API_TAG
{
    /// Publish a sb messaage
    JUNO_STATUS_T (*Publish)(JUNO_PUBLISHER_T *ptJunoPublisher, const JUNO_MSG_T *ptMsg);
    /// Get the publisher ID
    JUNO_STATUS_T (*GetPublisherId)(JUNO_PUBLISHER_T *ptJunoPublisher, JUNO_PUBLISHER_ID_T *ptRetPublisherId);
};

#ifdef __cplusplus
}
#endif
#endif // JUNO_PUBLISHER_API_H

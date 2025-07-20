#ifndef JUNO_TYPES_H
#define JUNO_TYPES_H
#include "module.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

JUNO_MODULE_RESULT(JUNO_RESULT_BOOL_T, bool);
JUNO_MODULE_RESULT(JUNO_RESULT_UINT32_T, uint32_t);

#ifdef __cplusplus
}
#endif
#endif
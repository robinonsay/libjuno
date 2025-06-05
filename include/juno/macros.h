/**
   Copyright 2025 Robin A. Onsay

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef JUNO_MACROS_H
#define JUNO_MACROS_H

#include "juno/status.h"
#include <stdint.h>

#define ASSERT_EXISTS(ptr) \
if(!(ptr)) \
{ \
    return JUNO_STATUS_NULLPTR_ERROR; \
}

#define ASSERT_EXISTS_MODULE(ptr, ptMod, str) if(!(ptr)) \
{ \
    FAIL_MODULE(JUNO_STATUS_NULLPTR_ERROR, ptMod, str); \
    return JUNO_STATUS_NULLPTR_ERROR; \
}

#define ASSERT_SUCCESS(tStatus, failOp) if(tStatus != JUNO_STATUS_SUCCESS) \
{ \
    failOp; \
}


#endif // JUNO_MACROS_H

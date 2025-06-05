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

/**
    This header contains the template library API
    @author
*/
#ifndef TEMPLATE_API_H
#define TEMPLATE_API_H
#include "juno/status.h"
#include "juno/module.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef struct TEMPLATE_API_TAG TEMPLATE_API_T;

JUNO_MODULE_DECLARE(TEMPLATE_T);
JUNO_MODULE_BASE_DECLARE(TEMPLATE_BASE_T);

JUNO_MODULE_BASE(TEMPLATE_BASE_T, TEMPLATE_API_T,
    /*
    
        TODO: Add implementation independent member variables here
    
    */
);

struct TEMPLATE_API_TAG
{
    /// Initializes the module and resources for template
    JUNO_STATUS_T (*Init)(
        TEMPLATE_T *ptTemplate
        /* TODO: Insert initialization arguments for module members here*/
    );
    /// Frees resources allocated by template
    JUNO_STATUS_T (*Free)(TEMPLATE_T *ptTemplate);
};

#ifdef __cplusplus
}
#endif
#endif // TEMPLATE_API_H

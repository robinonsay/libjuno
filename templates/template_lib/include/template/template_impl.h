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
    This header contains the template impl implementation
    @author
*/
#ifndef TEMPLATE_IMPL_H
#define TEMPLATE_IMPL_H
#include "juno/module.h"
#include "juno/status.h"
#include "template_api.h"
#ifdef __cplusplus
extern "C"
{
#endif


JUNO_MODULE_DERIVE_DECLARE(TEMPLATE_IMPL_T);

JUNO_MODULE_DERIVE(TEMPLATE_IMPL_T, TEMPLATE_BASE_T,
    /*
    
    TODO: Include implementation specific members here
    
    */
);

#define TEMPLATE_IMPL   TEMPLATE_IMPL_T tTemplateImpl

#ifndef TEMPLATE_DERIVED
/**
    This is the default implementation for `TEMPLATE_T`.
    If you want to derive new implementations for `TEMPLATE_T`
    use `#define TEMPLATE_DERIVED` prior to including
    `#include "template_impl.h"`

    Note: If you are implementing a derived module you will need
    to implement `TEMPLATE_IMPL`.
*/
JUNO_MODULE(TEMPLATE_T, TEMPLATE_BASE_T,
    TEMPLATE_IMPL;
);
#endif

/* TODO: Insert initialization arguments for module members here*/
JUNO_STATUS_T Template_ImplApi(TEMPLATE_T *ptTemplate, JUNO_FAILURE_HANDLER_T pfcnFailureHandler, JUNO_USER_DATA_T *pvFailureUserData);
#ifdef __cplusplus
}
#endif
#endif // TEMPLATE_IMPL_H


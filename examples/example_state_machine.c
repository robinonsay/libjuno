#include "juno/macros.h"
#include "juno/module.h"
#include "juno/sm/sm_api.h"
#include "juno/status.h"
#include "unistd.h"
#include <stdio.h>

typedef enum TRAFFIC_LIGHT_T
{
    TRAFFIC_RED,
    TRAFFIC_YELLOW,
    TRAFFIC_GREEN,
    TRAFFIC_COUNT
} TRAFFIC_LIGHT_T;

typedef struct TRAFFIC_LIGHT_STATE_T JUNO_MODULE_DERIVE(JUNO_SM_STATE_ROOT_T,
    int iCounter;
    TRAFFIC_LIGHT_T tNextLight;
    TRAFFIC_LIGHT_T tThisLight;
) TRAFFIC_LIGHT_STATE_T;

union JUNO_SM_STATE_T JUNO_MODULE(JUNO_SM_STATE_API_T, JUNO_SM_STATE_ROOT_T,
    TRAFFIC_LIGHT_STATE_T tTrafficLightState;
);

/// The action that should be executed in this state
static JUNO_STATUS_T StateAction(JUNO_SM_STATE_T *ptJunoSm)
{
    JUNO_STATUS_T tStatus = JunoSm_StateVerify(&ptJunoSm->tRoot);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TRAFFIC_LIGHT_STATE_T *ptTrafficLightState = (TRAFFIC_LIGHT_STATE_T*) ptJunoSm;
    printf("Current State: %u\n", ptTrafficLightState->tThisLight);
    sleep(1);
    ptTrafficLightState->iCounter += 1;
    return JUNO_STATUS_SUCCESS;
}
/// Returns a bool result whether the current state should exit
static JUNO_RESULT_BOOL_T ShouldExit(JUNO_SM_STATE_T *ptJunoSm)
{
    JUNO_STATUS_T tStatus = JunoSm_StateVerify(&ptJunoSm->tRoot);
    JUNO_RESULT_BOOL_T tResult = {0};
    TRAFFIC_LIGHT_STATE_T *ptTrafficLightState = (TRAFFIC_LIGHT_STATE_T*) ptJunoSm;
    tResult.tStatus = tStatus;
    JUNO_ASSERT_SUCCESS(tStatus, return tResult);
    tResult.tOk = ptTrafficLightState->iCounter == 10;
    return tResult;
}


static JUNO_STATUS_T ResetState(JUNO_SM_STATE_T *ptJunoSm)
{
    JUNO_STATUS_T tStatus = JunoSm_StateVerify(&ptJunoSm->tRoot);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    TRAFFIC_LIGHT_STATE_T *ptTrafficLightState = (TRAFFIC_LIGHT_STATE_T*) ptJunoSm;
    ptTrafficLightState->iCounter = 0;
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_SM_STATE_API_T tSmStateApi = {
    .StateAction = StateAction,
    .ShouldExit = ShouldExit,
    .ResetState = ResetState
};


typedef struct TRAFFIC_LIGHT_SM_T JUNO_MODULE_DERIVE(JUNO_SM_ROOT_T,
    TRAFFIC_LIGHT_STATE_T tStates[TRAFFIC_COUNT];
) TRAFFIC_LIGHT_SM_T;

union JUNO_SM_T JUNO_MODULE(void, JUNO_SM_ROOT_T,
    TRAFFIC_LIGHT_SM_T tTrafficLightSm;
);

TRAFFIC_LIGHT_SM_T tSm = {
        .tStates[TRAFFIC_RED] = {
            .tNextLight = TRAFFIC_YELLOW,
            .tThisLight = TRAFFIC_RED
        },
        .tStates[TRAFFIC_YELLOW] = {
            .tNextLight = TRAFFIC_GREEN,
            .tThisLight = TRAFFIC_YELLOW,
        },
        .tStates[TRAFFIC_GREEN] = {
            .tNextLight = TRAFFIC_RED,
            .tThisLight = TRAFFIC_GREEN
        },
};

int main()
{
    JUNO_STATUS_T tStatus = JunoSm_Init(&tSm.tRoot, &tSm.tStates[TRAFFIC_RED].tRoot, NULL, NULL);
    for(size_t i = 0; i < TRAFFIC_COUNT; i++)
    {
        TRAFFIC_LIGHT_STATE_T *ptSmState = &tSm.tStates[i];
        tStatus = JunoSm_StateInit(&tSm.tRoot, &ptSmState->tRoot, &tSm.tStates[ptSmState->tNextLight].tRoot, &tSmStateApi, NULL, NULL);
    }
    while(true)
    {
        JUNO_SM_RESULT_STATE_T tStateResult = JunoSm_GetCurrentState(&tSm.tRoot);
        JUNO_ASSERT_SUCCESS(tStateResult.tStatus, break;);
        JUNO_SM_STATE_ROOT_T *ptSmState = &tStateResult.tOk->tRoot;
        if(ptSmState)
        {
            const JUNO_SM_STATE_API_T *ptSmStateApi = ptSmState->ptApi;
            JUNO_RESULT_BOOL_T tBoolResult = ptSmStateApi->ShouldExit((JUNO_SM_STATE_T *)ptSmState);
            JUNO_ASSERT_SUCCESS(tBoolResult.tStatus, break;);
            if(tBoolResult.tOk)
            {
                ptSmStateApi->ResetState((JUNO_SM_STATE_T *)ptSmState);
                JunoSm_TransitionState(&tSm.tRoot);
            }
            else
            {
                ptSmStateApi->StateAction((JUNO_SM_STATE_T *)ptSmState);
            }
        }
    }
    return 0;
}


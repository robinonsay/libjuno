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
    JUNO_STATUS_T tStatus = JunoSm_StateVerify(ptJunoSm);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    printf("Current State: %u\n", ptJunoSm->tTrafficLightState.tThisLight);
    sleep(1);
    ptJunoSm->tTrafficLightState.iCounter += 1;
    return JUNO_STATUS_SUCCESS;
}
/// Returns a bool result whether the current state should exit
static JUNO_RESULT_BOOL_T ShouldExit(JUNO_SM_STATE_T *ptJunoSm)
{
    JUNO_STATUS_T tStatus = JunoSm_StateVerify(ptJunoSm);
    JUNO_RESULT_BOOL_T tResult = {0};
    tResult.tStatus = tStatus;
    JUNO_ASSERT_SUCCESS(tStatus, return tResult);
    tResult.tOk = ptJunoSm->tTrafficLightState.iCounter == 10;
    return tResult;
}


static JUNO_STATUS_T ResetState(JUNO_SM_STATE_T *ptJunoSm)
{
    JUNO_STATUS_T tStatus = JunoSm_StateVerify(ptJunoSm);
    JUNO_ASSERT_SUCCESS(tStatus, return tStatus);
    ptJunoSm->tTrafficLightState.iCounter = 0;
    return JUNO_STATUS_SUCCESS;
}

static const JUNO_SM_STATE_API_T tSmStateApi = {
    .StateAction = StateAction,
    .ShouldExit = ShouldExit,
    .ResetState = ResetState
};


typedef struct TRAFFIC_LIGHT_SM_T JUNO_MODULE_DERIVE(JUNO_SM_ROOT_T,
    JUNO_SM_STATE_T tStates[TRAFFIC_COUNT];
) TRAFFIC_LIGHT_SM_T;

union JUNO_SM_T JUNO_MODULE(void, JUNO_SM_ROOT_T,
    TRAFFIC_LIGHT_SM_T tTrafficLightSm;
);

JUNO_SM_T tSm = {
    .tTrafficLightSm = {
        .tStates[TRAFFIC_RED] = {
            .tTrafficLightState.tNextLight = TRAFFIC_YELLOW,
            .tTrafficLightState.tThisLight = TRAFFIC_RED
        },
        .tStates[TRAFFIC_YELLOW] = {
            .tTrafficLightState.tNextLight = TRAFFIC_GREEN,
            .tTrafficLightState.tThisLight = TRAFFIC_YELLOW,
        },
        .tStates[TRAFFIC_GREEN] = {
            .tTrafficLightState.tNextLight = TRAFFIC_RED,
            .tTrafficLightState.tThisLight = TRAFFIC_GREEN
        },
    }
};

int main()
{
    JUNO_STATUS_T tStatus = JunoSm_Init(&tSm, &tSm.tTrafficLightSm.tStates[TRAFFIC_RED], NULL, NULL);
    for(size_t i = 0; i < TRAFFIC_COUNT; i++)
    {
        JUNO_SM_STATE_T *ptSmState = &tSm.tTrafficLightSm.tStates[i];
        tStatus = JunoSm_StateInit(&tSm, ptSmState, &tSm.tTrafficLightSm.tStates[ptSmState->tTrafficLightState.tNextLight], &tSmStateApi, NULL, NULL);
    }
    while(true)
    {
        JUNO_SM_RESULT_STATE_T tStateResult = JunoSm_GetCurrentState(&tSm);
        JUNO_ASSERT_SUCCESS(tStateResult.tStatus, break;);
        JUNO_SM_STATE_T *ptSmState = tStateResult.tOk;
        if(ptSmState)
        {
            const JUNO_SM_STATE_API_T *ptSmStateApi = ptSmState->ptApi;
            JUNO_RESULT_BOOL_T tBoolResult = ptSmStateApi->ShouldExit(ptSmState);
            JUNO_ASSERT_SUCCESS(tBoolResult.tStatus, break;);
            if(tBoolResult.tOk)
            {
                ptSmStateApi->ResetState(ptSmState);
                JunoSm_TransitionState(&tSm);
            }
            else
            {
                ptSmStateApi->StateAction(ptSmState);
            }
        }
    }
    return 0;
}


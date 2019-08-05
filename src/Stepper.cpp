/*

    Stepper motor functions.

    Written by Artamonov Dmitry <screwer@gmail.com>

*/
//---------------------------------------------------------------------------------------

#include "Functions.h"

//---------------------------------------------------------------------------------------
//
// Подключение шагового двигателя.
// Двигатель биполярный, двухфазный: A и B.
// В каждой фазе по 3 отвода от катушки отвод от середины обмотки - не используется.
// Итого 4 провода: А1 (RED), А2 (BLU), B1 (BLK), B2 (GRN)
//
// Wiring:
//      PA8 <-> EN1/OUT1 <-> RED <-> A1
//      PA9 <-> EN2/OUT2 <-> BLU <-> A2
//      PB6 <-> EN3/OUT3 <-> BLK <-> B1
//      PB7 <-> EN4/OUT4 <-> GRN <-> B2
//
// Вращение:
//          A1  A2      B1  B2
//      1)  +   -
//    1-1)  +   -       +   -   <== полушаг
//      2)              +   -
//    2-2)  -   +       +   -   <== полушаг
//      3)  -   +
//    3-3)  -   +       -   +   <== полушаг
//      4)              -   +
//    4-4)  +   -       -   +   <== полушаг
//
// Управление:
//      [+ -] : [1 0]
//      [- +] : [0 1]
//      [   ] : [0 0]
//      [   ] : [1 1]
//
//---------------------------------------------------------------------------------------

static uint32_t g_StepInProgressTick = 0;
static uint8_t g_StepIndex = 0;
static int g_StepTask = 0;
static uint32_t g_StepLastControlTick = 0;

//---------------------------------------------------------------------------------------

struct StepData
{
    GPIO_TypeDef*   Port;
    uint16_t        Pin;
};

#define DECLARE_STEPDATA(LINE) { CFG_STEPPER_##LINE##_PORT, CFG_STEPPER_##LINE##_PIN }

static const StepData Steps[] = {
    DECLARE_STEPDATA(B2),
    DECLARE_STEPDATA(A2),
    DECLARE_STEPDATA(B1),
    DECLARE_STEPDATA(A1),
};

//---------------------------------------------------------------------------------------

static void DoStep(uint8_t index)
{
    uint8_t pos = (index % _countof(Steps));
    const StepData& data = Steps[pos];

    GPIO::SetBits(data.Port, data.Pin);
    DWT_Delay(CFG_STEP_PULSE_WIDTH_US);
    GPIO::ResetBits(data.Port, data.Pin);

    g_StepInProgressTick = CFG_STEP_DURATION_MS;
    g_StepLastControlTick = HAL_GetTick();
}

//---------------------------------------------------------------------------------------

static void DoStepForward()
{
    DoStep(++g_StepIndex);
}

//---------------------------------------------------------------------------------------

static void DoStepBackward()
{
    DoStep(--g_StepIndex);
}

//---------------------------------------------------------------------------------------

void Stepper_OnTick()
{
    if (0 != g_StepInProgressTick)
    {
        --g_StepInProgressTick;
        return;
    }

    if (0 == g_StepTask)
    {
        return;
    }

    if (g_StepTask < 0)
    {
        DoStepBackward();
        ++g_StepTask;
    }
    else
    {
        DoStepForward();
        --g_StepTask;
    }
}

//---------------------------------------------------------------------------------------

bool Stepper_IsReady()
{
    return ((0 == g_StepTask) && (0 == g_StepInProgressTick));
}

//---------------------------------------------------------------------------------------

static void Stepper_WaitReady()
{
    while (!Stepper_IsReady());
}

//---------------------------------------------------------------------------------------
//
// Установка мотора в положение "0"
//
void Stepper_Calibrate()
{
    DoStep(2);
    Stepper_WaitReady();
    
    DoStep(3);
    Stepper_WaitReady();

    DoStep(0);
    Stepper_WaitReady();

	Stepper_Rotate(-300);
}

//---------------------------------------------------------------------------------------

void Stepper_Rotate(int steps)
{
	DbgPrint("\nStepper rotate=%d", steps);
    g_StepTask += steps;
}

//---------------------------------------------------------------------------------------

bool Stepper_Stop()
{
	DbgPrint("\nStepper STOP");
    g_StepTask = 0;
}

//---------------------------------------------------------------------------------------

uint32_t Stepper_GetLastControlTick()
{
    return g_StepLastControlTick;
}

//---------------------------------------------------------------------------------------

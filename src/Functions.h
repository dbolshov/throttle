/*

    All functions list.

    Written by Artamonov Dmitry <screwer@gmail.com>

*/
//---------------------------------------------------------------------------------------

#pragma once

#include "Config.h"

//---------------------------------------------------------------------------------------

#define _countof(x) (sizeof(x)/sizeof(*x))
inline int abs(int x)
{
	return (x >= 0) ? x : -x;
}

inline int min(int a, int b)
{
    return (a < b) ? a : b;
}

//---------------------------------------------------------------------------------------

namespace GPIO
{
    inline uint16_t Write(GPIO_TypeDef* Port, uint16_t Value)
    {
        Port->ODR = Value;
    }

    inline uint16_t Read(GPIO_TypeDef* Port)
    {
        return Port->IDR;
    }

    inline void SetBits(GPIO_TypeDef* Port, uint16_t Pin)
    {
        Port->BSRR = Pin;
    }

    inline void ResetBits(GPIO_TypeDef* Port, uint16_t Pin)
    {
        Port->BRR = Pin;
    }
} // namespace GPIO

//---------------------------------------------------------------------------------------
//
// "Init.cpp"
//
void Init();
uint32_t DWT_Get(void);
void DWT_Delay(uint32_t us);

//---------------------------------------------------------------------------------------
//
// "Debug.cpp"
//
#ifdef DEBUG

int DbgReport(const char* file, int line, const char* msg);
int DbgBreak();
#define ASSERT(expr) (void)(!!(expr) || (1 != DbgReport(__FILE__, __LINE__, #expr) || DbgBreak()))
#define VERIFY(expr) ASSERT(expr)
#define HAL_VERIFY(expr) ASSERT(HAL_OK == (expr))
void DbgPrint(const char* pszFmt, ...);

extern UART_HandleTypeDef DbgPort;

#else

#define ASSERT(expr) ((void)0)
#define VERIFY(expr) (expr)
#define HAL_VERIFY(expr) (expr)
#define DbgPrint(...) ((void)0)

#endif

//---------------------------------------------------------------------------------------
//
// "main.cpp"
//
void OnSysTick();
void OnExtInterrupt_0();

//---------------------------------------------------------------------------------------
//
// "Stepper.cpp"
//
void Stepper_Calibrate();
bool Stepper_IsReady();
void Stepper_OnTick();
void Stepper_Rotate(int steps);
bool Stepper_Stop();
uint32_t Stepper_GetLastControlTick();

//---------------------------------------------------------------------------------------

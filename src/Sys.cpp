/*

    System firmware file.

    Written by Artamonov Dmitry <screwer@gmail.com>

*/
//---------------------------------------------------------------------------------------

#include <stm32f1xx_hal.h>
#include <stm32_hal_legacy.h>
#include "Functions.h"

//---------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
#endif
void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();

    OnSysTick();
}

#ifdef __cplusplus
extern "C"
#endif
void EXTI0_IRQHandler(void)
{
    OnExtInterrupt_0();
}

//---------------------------------------------------------------------------------------

void SystemClock_Config()
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    //
    // Initializes the CPU, AHB and APB busses clocks
    //
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_VERIFY(HAL_RCC_OscConfig(&RCC_OscInitStruct));

    //
    // Initializes the CPU, AHB and APB busses clocks
    //
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
        | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_VERIFY(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2));

    //
    // Configure the Systick interrupt time
    //
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    //
    // Configure the Systick
    //
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    //
    // SysTick_IRQn interrupt configuration
    //
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

//---------------------------------------------------------------------------------------

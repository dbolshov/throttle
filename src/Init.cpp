/*

    Low-Level GPIO functions.

    Written by Artamonov Dmitry <screwer@gmail.com>

*/
//---------------------------------------------------------------------------------------

#include "Functions.h"

//---------------------------------------------------------------------------------------

static void InitOutput(GPIO_TypeDef* Port, uint16_t Pins)
{
    GPIO_InitTypeDef gpio;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Pin = Pins;

    HAL_GPIO_Init(Port, &gpio);
}

//---------------------------------------------------------------------------------------
/*
static void InitInput(GPIO_TypeDef* Port, uint16_t Pins)
{
    GPIO_InitTypeDef gpio;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    gpio.Pin = Pins;

    HAL_GPIO_Init(Port, &gpio);
}
*/
//---------------------------------------------------------------------------------------
//
// Init USART3: Debug port
//
#ifdef DEBUG
static void InitUsart3_Debug()
{
	GPIO_InitTypeDef gpio;

	gpio.Pin = CFG_UART_TX_PIN;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(CFG_UART_PORT, &gpio);

	gpio.Pin = CFG_UART_RX_PIN;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(CFG_UART_PORT, &gpio);

	// UART
	DbgPort.Instance = CFG_UART_INSTANCE;
	auto& Init = DbgPort.Init;
	Init.BaudRate = 9600;
	Init.WordLength = UART_WORDLENGTH_8B;
	Init.StopBits = UART_STOPBITS_1;
	Init.Parity = UART_PARITY_NONE;
	Init.Mode = UART_MODE_TX_RX;
	Init.HwFlowCtl = UART_HWCONTROL_NONE;
	Init.OverSampling = UART_OVERSAMPLING_16;

	HAL_VERIFY(HAL_UART_Init(&DbgPort));

}
#endif		

//---------------------------------------------------------------------------------------
    //gpio.Mode = GPIO_MODE_INPUT;

static void InitPulseIn()
{
    GPIO_InitTypeDef gpio;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    //gpio.Mode = GPIO_MODE_IT_RISING;
	gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
	//gpio.Pull = GPIO_PULLDOWN;
	gpio.Pull = GPIO_PULLUP;
    gpio.Pin = CFG_ROTATE_PULSE_PIN;

    HAL_GPIO_Init(CFG_ROTATE_PULSE_PORT, &gpio);
}

//---------------------------------------------------------------------------------------

static void InitStepper()
{
	InitOutput(CFG_STEPPER_A1_PORT, CFG_STEPPER_A1_PIN);
	InitOutput(CFG_STEPPER_A2_PORT, CFG_STEPPER_A2_PIN);
	InitOutput(CFG_STEPPER_B1_PORT, CFG_STEPPER_B1_PIN);
	InitOutput(CFG_STEPPER_B2_PORT, CFG_STEPPER_B2_PIN);

    DbgPrint("  * Prepare stepper motor calibration.\n");
    Stepper_Calibrate();
    DbgPrint("  * Stepper motor calibration completed.\n");
}

//---------------------------------------------------------------------------------------


#define    DWT_CYCCNT    *(volatile uint32_t *)0xE0001004
#define    DWT_CONTROL   *(volatile uint32_t *)0xE0001000
#define    SCB_DEMCR     *(volatile uint32_t *)0xE000EDFC

void InitDWT(void)
{
    if (!(DWT_CONTROL & 1))
    {
        SCB_DEMCR |= 0x01000000;
        DWT_CYCCNT = 0;
        DWT_CONTROL |= 1; // enable the counter
    }
}

//---------------------------------------------------------------------------------------

uint32_t DWT_Get(void)
{
    return DWT_CYCCNT;
}

//---------------------------------------------------------------------------------------

__inline
uint8_t DWT_Compare(int32_t tp)
{
    return (((int32_t)DWT_Get() - tp) < 0);
}

//---------------------------------------------------------------------------------------

void DWT_Delay(uint32_t us) 
{
    int32_t tc = DWT_Get();
    int32_t tp = tc + us * (SystemCoreClock / 1000000);
    while (DWT_Compare(tp));
}
//---------------------------------------------------------------------------------------

extern void SystemClock_Config();

void Init()
{
	HAL_Init();
	SystemClock_Config();
    InitDWT();

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

#ifdef DEBUG
	__HAL_RCC_USART3_CLK_ENABLE();
	InitUsart3_Debug();    
#endif

    DbgPrint("Firmware started, version 0.1\n");
    DbgPrint("Initialization\n");
    InitPulseIn();
	InitStepper();

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
    HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    DbgPrint("Initialization completed.\n");
}

//---------------------------------------------------------------------------------------


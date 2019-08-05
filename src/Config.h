/*

    Firmware configuration.

    Written by Artamonov Dmitry <screwer@gmail.com>

*/

//---------------------------------------------------------------------------------------
//
// ��������� ������� ��������
//
#define CFG_ROTATE_PULSES_MIN           1400    // ����������� ���������� ��������� (� ������)
#define CFG_ROTATE_PULSES_PREF          4100    // �������� ���������� ��������� (� ������)
#define CFG_ROTATE_PULSES_CRITICAL      6000    // ����������� ���������� ���������, ������� �������� ��������� �������� �������� �������� (� ������)
#define CFG_ROTATE_PULSES_MAX           6800    // ������������ ���������� ��������� (� ������)

#define CFG_ROTATE_DEVIATION_PERCENT    5      // ����c����� ���������� � ����� ��������� (%), �� ���������� � ��������� ����������.
//
// ��������� ����������� ��������
//
#define CFG_THROTTLE_RANGE_DEGREES      100      // �������� ���� ���������� ����������� ��������
                                                // FIXME: ����������� ������������ �������� �� ���� ��������

//
// ��������� �������� ���������
//
#define CFG_STEP_PULSE_WIDTH_US         5000    // ������ �������� ���� (���), >= 300 (����� �� �������� ������� ����������� ��� ��� �����������)
#define CFG_STEP_DURATION_MS            0       // ������������ ���� (��)

#define CFG_STEPS_PER_REVOLUTION        1000     // ����� ����� �� ������ ���������

//
// ��������� ����������
//
#define CFG_CONTROL_POINTS_INTERVAL_MS  2000    // ����� (��) ����� ����������� ��������� ����������� ��������
                                                // (�����, ���� �������� ��������, �� �����������)

#define CFG_ROTATE_MEASURE_INTERVAL_MS  500    // ��������, � ������� �������� ���������� �������� �������� (����� ���������)

#define CFG_MAX_STEPS_PER_ONCE_CONTROL  200      // ����������� ���������� ����� ����� �� ���� ��������� ����������� ��������

//---------------------------------------------------------------------------------------
//
// ��������� ������� STM32
//
#define CFG_STEPPER_A1_PORT     GPIOA       // EN1
#define CFG_STEPPER_A1_PIN      GPIO_PIN_8
#define CFG_STEPPER_A2_PORT     GPIOA       // EN2
#define CFG_STEPPER_A2_PIN      GPIO_PIN_9
#define CFG_STEPPER_B1_PORT     GPIOB       // EN3
#define CFG_STEPPER_B1_PIN      GPIO_PIN_6
#define CFG_STEPPER_B2_PORT     GPIOB       // EN4
#define CFG_STEPPER_B2_PIN      GPIO_PIN_7

#ifdef DEBUG
    #define CFG_UART_INSTANCE       USART3
    #define CFG_UART_PORT           GPIOB
    #define CFG_UART_TX_PIN         GPIO_PIN_10
    #define CFG_UART_RX_PIN         GPIO_PIN_11
#endif

#define CFG_ROTATE_PULSE_PORT   GPIOB
#define CFG_ROTATE_PULSE_PIN    GPIO_PIN_0

//
// �������� ���� �� �������: PB1, PA7, PA6, PA5, PA4
//
//---------------------------------------------------------------------------------------

#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_gpio.h>
#include <stm32f1xx_ll_exti.h>


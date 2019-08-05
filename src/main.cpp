/*

    Main firmware file.

    Written by Artamonov Dmitry <screwer@gmail.com>

*/
//---------------------------------------------------------------------------------------

#include "Functions.h"

//---------------------------------------------------------------------------------------
//
// Аппаратное прерывание для подсчёта импульсов от датчика скорости
//
static int volatile PulsesCount = 0;

#ifdef DEBUG
static int volatile PulseIndex = 0;
static int volatile PulsesTime[2048];
#endif

void OnExtInterrupt_0()
{
    if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) == SET)
	{
		bool On = (0 == (GPIO::Read(CFG_ROTATE_PULSE_PORT) & CFG_ROTATE_PULSE_PIN));
		static uint32_t tick = 0;
		if (On)
		{
			tick = DWT_Get();
		}
		else
		{
#ifdef DEBUG
			if (PulseIndex < _countof(PulsesTime))
			{
				PulsesTime[PulseIndex++] = (DWT_Get() - tick);
			}
#endif			
			++PulsesCount;
		}
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
    }
}

//---------------------------------------------------------------------------------------
//
// Преобразование интервала импульсов в число шагов
//
int ConvertPulsesRangeToSteps(int PulsesStart, int PulsesEnd)
{
    int WholePulsesRange = (CFG_ROTATE_PULSES_MAX - CFG_ROTATE_PULSES_MIN);
    int WholeThrottleRange = (CFG_THROTTLE_RANGE_DEGREES * 60);
    int PulsesRange = (PulsesEnd - PulsesStart);

    //
    // количество угловых минут (со знаком) на которые требуется повернуть дроссель
    //
    int ThrottleRange = (PulsesRange * WholeThrottleRange) / WholePulsesRange;

    //
    // Преобразование угловых минут в шаги
    //
    int OneStepRange = ((360 * 60) / CFG_STEPS_PER_REVOLUTION);
    int Steps = ThrottleRange / OneStepRange;

	int ang = ThrottleRange / 60;
	int angf = abs(ThrottleRange % 60);
	DbgPrint("\nPulses to steps: %d -> %d (%d), trottle_angle=%d.%d, steps=%d", PulsesStart, PulsesEnd, PulsesRange, ang, angf, Steps);
	return Steps;
}

//---------------------------------------------------------------------------------------
//
// Текущее состояние управления двигателем.
//
struct ControlState
{
    enum Enum
    {
        Idle,           // ожидание запуска двигателя
        WaitControl,    // ожидание возможности начать управление
        Stable,         // стабильные обороты
        Boost,          // разгон (добавление газа)
        Slowdown,       // замeдление (убирание газа)
        FaultDrop,      // экстренный сброс газа (превышены критические обороты)
    };
};

static ControlState::Enum g_ControlState = ControlState::Idle;

//---------------------------------------------------------------------------------------

#ifdef DEBUG
//
// Для отладочной печати состояния в виде текстовой строки
//
const char* GetControlStateStr()
{
    static const char* Strings[] = {
        "Idle",
        "WaitControl",
        "Stable",
        "Boost",
        "Slowdown",
        "FaultDrop",
    };
    return Strings[g_ControlState];
}
#endif

//---------------------------------------------------------------------------------------
//
// Функция управления.
// Реализует всю логику по перемещению дроссельной заслонки.
//
void DoControl()
{
    //
    // Состояние, сохраняемое между вызовами
    //
    static uint32_t TickMeasureStart = HAL_GetTick();   // интервал измерения числа импульсов (скорости)
    static int RotationSpeed = 0;                       // текущая скорость (импульсы в минуту)

    //
    // Рассчёт текущей скорости вращения
    //
    uint32_t TickNow = HAL_GetTick();
    uint32_t Elapsed = (TickNow - TickMeasureStart);

	if (Elapsed < CFG_ROTATE_MEASURE_INTERVAL_MS)
	{
		return;
	}

	RotationSpeed = (60000 * PulsesCount) / Elapsed; // 60000 - число миллисекунд в минуте, т.к. всё время в миллисекундах

	// начало нового интервала измерения
	TickMeasureStart = TickNow;
	PulsesCount = 0;

#ifdef DEBUG
/*
	DbgPrint("\nPulses count=%d [", PulseIndex);
	for (int i = 0; i < PulseIndex; i++)
	{
		if (i > 0)
			DbgPrint(", ");
		DbgPrint("%u", PulsesTime[i]);
	}
	DbgPrint("]");
	PulseIndex = 0;
*/
	int sec = TickNow / 1000;
	int secf = (TickNow / 10) % 100;
	DbgPrint("\n%5d.%02d [%s] speed=%d ", sec, secf, GetControlStateStr(), RotationSpeed);
#endif

	if (RotationSpeed >= CFG_ROTATE_PULSES_CRITICAL)
	{
		//
		// Достигли критической скорости вращения.
		//
		if (ControlState::FaultDrop != g_ControlState)
		{
			//
			// При первом обнаружении критических оборотов: отменяем текущую команду
			//
			Stepper_Stop();
		}
		g_ControlState = ControlState::FaultDrop;

		if (Stepper_IsReady())
		{
			int Steps = ConvertPulsesRangeToSteps(RotationSpeed, CFG_ROTATE_PULSES_PREF);
			Stepper_Rotate(Steps);
		}
		return;
	}

    if (RotationSpeed <= CFG_ROTATE_PULSES_MIN)
    {
        //
        // Двигатель не может вращаться со скоростью, ниже минимальной.
        // Переключаемся в режим Idle
        //
        g_ControlState = ControlState::Idle;
		return; // Неработающий двигатель не требует управления.
    }

    //
    // Проверка, необходимо ли вносить корректировки
    //
    if (!Stepper_IsReady())
    {
        g_ControlState = ControlState::WaitControl;
        return; // корректировки вносить нельзя - не готов шаговый двигатель.
    }

    Elapsed = (TickNow - Stepper_GetLastControlTick());
    if (Elapsed < CFG_CONTROL_POINTS_INTERVAL_MS)
    {
        g_ControlState = ControlState::WaitControl;
        return; // корректировки вносить нельзя - не истёк таймаут от предыдущей корректировки
    }

    int Deviation = (abs(CFG_ROTATE_PULSES_PREF - RotationSpeed) * 100) / CFG_ROTATE_PULSES_PREF;
	if (Deviation <= CFG_ROTATE_DEVIATION_PERCENT)
    {
        g_ControlState = ControlState::Stable; // отклонение укладывается в допустимые рамки
        return;
    }

	//
    // Сюда попадаем только в случае необходимости начать разгон или торможение
    //
    g_ControlState = (RotationSpeed < CFG_ROTATE_PULSES_PREF)
        ? ControlState::Boost
        : ControlState::Slowdown;

    int Steps = ConvertPulsesRangeToSteps(RotationSpeed, CFG_ROTATE_PULSES_PREF);
    Steps = min(Steps, CFG_MAX_STEPS_PER_ONCE_CONTROL);
    Stepper_Rotate(Steps);
}

//---------------------------------------------------------------------------------------
//
// Системные часы, "тикают" с частотой 1000 раз в секунду (интервал 1ms)
//
void OnSysTick()
{
    Stepper_OnTick(); // сообщаем мотору что истекла очередная миллисекунда

    DoControl(); // Вызываем функцию управления
}

//---------------------------------------------------------------------------------------

int main()
{
    Init();

    //
    // Бесконечный цикл, не грузящий CPU.
    //
    for (;;)
    {
        __asm__ volatile("yield");
    }
}

//---------------------------------------------------------------------------------------
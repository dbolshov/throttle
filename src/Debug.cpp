/*

    Debug functions.

    Written by Artamonov Dmitry <screwer@gmail.com>

*/
//---------------------------------------------------------------------------------------

#include "Functions.h"
#include <stdarg.h>

#ifdef DEBUG

UART_HandleTypeDef DbgPort = {};

inline void WaitDebugPortReady()
{
	while (!DbgPort.Instance);
}

static const int g_DbgPrintBufferMaxLen = 512;
static uint8_t szBuf[g_DbgPrintBufferMaxLen];

//---------------------------------------------------------------------------------------

void DbgPrint(const char* pszFmt, ...)
{
	WaitDebugPortReady();

    va_list vl;
    va_start(vl, pszFmt);
    int n = vsnprintf(reinterpret_cast<char*>(szBuf), sizeof(szBuf), pszFmt, vl);
    va_end(vl);

    HAL_UART_Transmit(&DbgPort, szBuf, n, HAL_MAX_DELAY);
    //uint8_t lr = '\n';
    //HAL_UART_Transmit(&UART::Printer, &lr, 1, HAL_MAX_DELAY);
}

//---------------------------------------------------------------------------------------

int DbgReport(const char* file, int line, const char* msg)
{    
    DbgPrint("Assertion failed: '%s'\n", msg);
    for (uint8_t c;;)
    {
        DbgPrint("Press (B) for break or (I) for ignore.\n");
        HAL_UART_Receive(&DbgPort, &c, 1, HAL_MAX_DELAY);
        c &= ~(0x20); // toupper
        switch (c)
        {
        case 'B':
            return 1;
        case 'I':
            return 0;
        }
    }
}

//---------------------------------------------------------------------------------------

int DbgBreak()
{
    asm("bkpt 255");
    return 0;
}

//---------------------------------------------------------------------------------------

#endif // DEBUG
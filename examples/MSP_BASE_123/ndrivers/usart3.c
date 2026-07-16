#include "ti_msp_dl_config.h"
#include "jy61P.h"
#include "usart3.h"


/* UART3 (PB3/PB2) is now reserved for K230 vision reception.
 * Keep this legacy entry point as a no-op so old mode code still links. */
void jy61p_Init(void)
{
}


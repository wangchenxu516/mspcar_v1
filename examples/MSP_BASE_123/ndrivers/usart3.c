#include "ti_msp_dl_config.h"
#include "jy61P.h"
#include "usart3.h"


/* 主函数添加初始化jy61p_Init());即可开启jy62串口协议
   可以直接读取Yaw
*/

void jy61p_Init(void)
{
	NVIC_ClearPendingIRQ(UART3_INT_IRQn);
	NVIC_EnableIRQ(UART3_INT_IRQn);
	DL_UART_clearInterruptStatus(UART3,DL_UART_INTERRUPT_RX);//清除中断标志位

}

void UART3_IRQHandler(void)
{
	uint8_t RxData = DL_UART_receiveData(UART3);
	jy61p_ReceiveData(RxData);
	
}


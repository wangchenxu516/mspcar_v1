#include "ti_msp_dl_config.h"
#include "head.h"


void usart1_Init(void)
{	
	
	NVIC_ClearPendingIRQ(UART_1_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_1_INST_INT_IRQN);
	DL_UART_clearInterruptStatus(UART1,DL_UART_INTERRUPT_RX);//清除中断标志位
}

//void UART_1_INST_IRQHandler(void)          
//{
//	
//    switch( DL_UART_getPendingInterrupt(UART_1_INST) )
//    {
//        case DL_UART_IIDX_RX:
//			

//        default:
//            break;
//    }
//	
//}
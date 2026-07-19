#ifndef _USART1_H
#define _USART1_H

#include <stdint.h>

/* K230 frame: AA 55 01 command 00 checksum.
 * command 1/2/3/4 maps to 90/180/270/360 degrees. */
#define VISION_CONFIRM_COUNT 1U

extern volatile uint16_t vision_candidate_angle;
extern volatile uint16_t vision_locked_angle;
extern volatile uint8_t vision_same_count;
extern volatile uint8_t vision_valid;
extern volatile uint32_t vision_rx_byte_count;
extern volatile uint32_t vision_valid_frame_count;
extern volatile uint32_t vision_rejected_frame_count;
extern volatile uint8_t vision_last_byte;
extern volatile uint32_t hc12_tx_frame_count;

/* Legacy filename: vision UART now uses UART3 on PB3/PB2. */
void Vision_UART_Init(void);
void Vision_Reset(void);

/* HC-12 uses UART1: PB6 TX, PB7 RX, 9600 8-N-1. */
void HC12_UART_Init(void);
void HC12_Tick10ms(void);
void HC12_NotifyCarStopped(void);
void HC12_Process(void);

#endif

#include "ti_msp_dl_config.h"
#include "head.h"

volatile uint16_t vision_candidate_angle = 0;
volatile uint16_t vision_locked_angle = 0;
volatile uint8_t vision_same_count = 0;
volatile uint8_t vision_valid = 0;
volatile uint32_t vision_rx_byte_count = 0;
volatile uint32_t vision_valid_frame_count = 0;
volatile uint32_t vision_rejected_frame_count = 0;
volatile uint8_t vision_last_byte = 0;

static uint8_t vision_rx_state = 0;
static uint8_t vision_angle_low = 0;
static uint8_t vision_angle_high = 0;

#define VISION_ISR_BYTE_BUDGET 16U

static uint16_t Vision_CommandToAngle(
    uint8_t command, uint8_t commandHigh)
{
    if (commandHigh != 0U)
        return 0U;

    switch (command)
    {
        case 1U:
            return 90U;
        case 2U:
            return 180U;
        case 3U:
            return 270U;
        case 4U:
            return 360U;
        default:
            return 0U;
    }
}

static void Vision_AcceptAngle(uint16_t angle)
{
    vision_valid_frame_count++;

    /* Keep the first valid result. K230 may only transmit once when the
     * recognition result changes, so the MCU must not require repeats. */
    if (vision_valid != 0U)
        return;

    if (angle == vision_candidate_angle)
    {
        if (vision_same_count < VISION_CONFIRM_COUNT)
            vision_same_count++;
    }
    else
    {
        vision_candidate_angle = angle;
        vision_same_count = 1U;
    }

    if (vision_same_count >= VISION_CONFIRM_COUNT)
    {
        vision_locked_angle = angle;
        vision_valid = 1U;
    }
}

static void Vision_ParseByte(uint8_t data)
{
    uint16_t angle;
    uint8_t checksum;

    switch (vision_rx_state)
    {
        case 0:
            if (data == 0xAAU)
                vision_rx_state = 1U;
            break;

        case 1:
            if (data == 0x55U)
                vision_rx_state = 2U;
            else if (data != 0xAAU)
                vision_rx_state = 0U;
            break;

        case 2:
            if (data == 0x01U)
                vision_rx_state = 3U;
            else if (data == 0xAAU)
                vision_rx_state = 1U;
            else
                vision_rx_state = 0U;
            break;

        case 3:
            vision_angle_low = data;
            vision_rx_state = 4U;
            break;

        case 4:
            vision_angle_high = data;
            vision_rx_state = 5U;
            break;

        case 5:
            checksum = (uint8_t)(vision_angle_low + vision_angle_high);
            angle = Vision_CommandToAngle(
                vision_angle_low, vision_angle_high);

            if ((data == checksum) && (angle != 0U))
                Vision_AcceptAngle(angle);
            else
                vision_rejected_frame_count++;

            vision_rx_state = (data == 0xAAU) ? 1U : 0U;
            break;

        default:
            vision_rx_state = 0U;
            break;
    }
}

void Vision_Reset(void)
{
    NVIC_DisableIRQ(UART_3_INST_INT_IRQN);

    vision_candidate_angle = 0U;
    vision_locked_angle = 0U;
    vision_same_count = 0U;
    vision_valid = 0U;
    vision_rx_byte_count = 0U;
    vision_valid_frame_count = 0U;
    vision_rejected_frame_count = 0U;
    vision_last_byte = 0U;
    vision_rx_state = 0U;
    vision_angle_low = 0U;
    vision_angle_high = 0U;

    while (!DL_UART_Main_isRXFIFOEmpty(UART_3_INST))
        (void)DL_UART_Main_receiveData(UART_3_INST);

    DL_UART_Main_clearInterruptStatus(
        UART_3_INST, DL_UART_MAIN_INTERRUPT_RX);
    NVIC_ClearPendingIRQ(UART_3_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_3_INST_INT_IRQN);
}

void Vision_UART_Init(void)
{
    /* K230 is physically connected to UART3: PB3 RX, PB2 TX. */
    DL_UART_Main_disableInterrupt(
        UART_3_INST, DL_UART_MAIN_INTERRUPT_TX);

    /* A six-byte result may be transmitted only once.  The reset/default
     * FIFO threshold can leave the final one or two bytes below the RX
     * interrupt level forever, so interrupt as soon as one byte is present. */
    DL_UART_Main_setRXFIFOThreshold(
        UART_3_INST, DL_UART_MAIN_RX_FIFO_LEVEL_ONE_ENTRY);

    DL_UART_Main_enableInterrupt(
        UART_3_INST, DL_UART_MAIN_INTERRUPT_RX);
    DL_UART_Main_clearInterruptStatus(
        UART_3_INST, DL_UART_MAIN_INTERRUPT_RX);

    /* This handler only drains a few bytes. It must preempt the much longer
     * TIMER_0 software-I2C work so the small UART FIFO cannot overflow. */
    NVIC_SetPriority(UART_3_INST_INT_IRQN, 0U);
    NVIC_ClearPendingIRQ(UART_3_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_3_INST_INT_IRQN);
}

void UART_3_INST_IRQHandler(void)
{
    uint8_t byteBudget = VISION_ISR_BYTE_BUDGET;

    switch (DL_UART_Main_getPendingInterrupt(UART_3_INST))
    {
        case DL_UART_IIDX_RX:
            while ((!DL_UART_Main_isRXFIFOEmpty(UART_3_INST)) &&
                   (byteBudget > 0U))
            {
                uint8_t data =
                    (uint8_t)DL_UART_Main_receiveData(UART_3_INST);

                vision_last_byte = data;
                vision_rx_byte_count++;
                Vision_ParseByte(data);
                byteBudget--;
            }
            break;

        default:
            break;
    }
}

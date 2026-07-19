#include "head.h"

int32_t gz_sum = 0;
int16_t gz_avg = 0;

static void Display_TaskState(void)
{
    switch (taskState)
    {
        case TASK_WAIT_FIRST_START:
            display_6_8_string(1, 0, "WAIT FIRST KEY3     ");
            break;
        case TASK_FIRST_RUNNING:
            display_6_8_string(1, 0, "FIRST LAP RUNNING   ");
            break;
        case TASK_WAIT_SECOND_START:
            display_6_8_string(1, 0, "WAIT SECOND KEY3    ");
            break;
        case TASK_SECOND_RUNNING:
            display_6_8_string(1, 0, "SECOND RUNNING      ");
            break;
        case TASK_FINISHED:
            display_6_8_string(1, 0, "TASK FINISHED       ");
            break;
        default:
            display_6_8_string(1, 0, "STATE ERROR         ");
            break;
    }

    display_6_8_string(1, 1, "RX:        LAST:    ");
    display_6_8_number(20, 1, (int)vision_rx_byte_count);
    display_6_8_number(100, 1, (int)vision_last_byte);
    display_6_8_string(1, 2, "VISION:             ");
    display_6_8_number(70, 2, (int)vision_candidate_angle);
    display_6_8_string(1, 3, "LOCKED:             ");
    display_6_8_number(70, 3, (int)vision_locked_angle);
    display_6_8_string(1, 4, "SAVED :             ");
    display_6_8_number(70, 4, (int)taskSavedTargetAngle);
    display_6_8_string(1, 5, "TRAVEL:             ");
    display_6_8_number(70, 5, (int)taskTravelledAngle);
    display_6_8_string(1, 6, "GOOD/BAD:           ");
    display_6_8_number(58, 6, (int)vision_valid_frame_count);
    display_6_8_number(94, 6, (int)vision_rejected_frame_count);

    if ((taskState == TASK_WAIT_SECOND_START) &&
        (vision_valid == 0U))
        display_6_8_string(1, 7, "NO DATA - WAIT K230 ");
    else if ((taskState == TASK_WAIT_FIRST_START) ||
             (taskState == TASK_WAIT_SECOND_START))
        display_6_8_string(1, 7, "PRESS KEY3 TO START ");
    else
        display_6_8_string(1, 7, "                    ");
}

int main(void)
{
    SYSCFG_DL_init();

    /* K230 may report immediately after power-on. Start and clear UART3
     * before the relatively slow OLED and MPU6050 initialization. */
    Vision_UART_Init();
    Vision_Reset();
    HC12_UART_Init();

    oled_init();

    MPU6050_Init();
    delay_ms(300);
    for (uint16_t i = 0; i < 100U; i++)
    {
        MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
        gz_sum += GZ;
        delay_us(8);
    }
    gz_avg = (int16_t)(gz_sum / 100);

    /* A UART byte takes about 87 us at 115200 baud. Let its short handler
     * preempt the long software-I2C/MPU tracking interrupt to prevent FIFO
     * overrun; the tracking timer remains the next-highest priority. */
    NVIC_SetPriority(UART_3_INST_INT_IRQN, 0U);
    NVIC_SetPriority(TIMER_0_INST_INT_IRQN, 1U);

    NVIC_EnableIRQ(GPIO_EncoderA_INT_IRQN);
    NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    NVIC_EnableIRQ(Motor_INST_INT_IRQN);
    NVIC_EnableIRQ(Steer_INST_INT_IRQN);

    DL_TimerG_startCounter(Motor_INST);
    DL_TimerA_startCounter(TIMER_0_INST);
    DL_TimerA_startCounter(Steer_INST);

    Task_Control_Init();

    while (1)
    {
        check_buttons();

        if (KeyNum == 3)
        {
            Task_HandleStartButton();
            KeyNum = 0;
        }

        HC12_Process();
        Display_TaskState();
    }
}

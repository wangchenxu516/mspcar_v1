#include "head.h"

/*
 * 圆形轨迹分段控制参数
 * 4、5号附近：轻微顺时针右转；
 * 2、3号：向左纠偏；6、7号：向右纠偏；
 * 1、8号：紧急纠偏。
 */
#define TRACK_CURVE_BIAS          1.20f
#define TRACK_NORMAL_TURN         2.70f
#define TRACK_EMERGENCY_TURN      5.00f
#define TRACK_STEER_DIRECTION    -1.00f
#define TRACK_MIN_SPEED           7.00f
#define TRACK_MAX_SPEED          30.00f
#define TRACK_LOST_SPEED          8.00f
#define TRACK_LOST_STOP_COUNT    15U     /* 约150ms持续丢线后停车 */
#define TRACK_CONFIRM_COUNT       2U     /* 普通左右状态连续确认约20ms */
#define MOTOR_CHANNEL_TEST_ENABLE 0U     /* 1=通道0映射测试，0=恢复正常循迹 */

volatile int sensorPins[8] = {0}, Cycle_TIMES;
volatile float error1, derivative1, lastError, correction;

int HUIDU_test = 0, HUIDU_test1 = 0, HUIDU_test2, HUIDU_test3;
int HUIDU_test4, HUIDU_test5, coner_speed = 16;

typedef enum
{
    TRACK_CENTER = 0,
    TRACK_TURN_LEFT,
    TRACK_TURN_RIGHT,
    TRACK_EMERGENCY_LEFT,
    TRACK_EMERGENCY_RIGHT
} TrackState;

static TrackState currentTrackState = TRACK_CENTER;
static TrackState pendingTrackState = TRACK_CENTER;
static uint8_t pendingStateCount = 0;
static uint16_t lostLineCount = 0;
static float requestedTrackSpeed = 14.0f;

static float HUIDU_Limit(float value, float minimum, float maximum)
{
    if (value > maximum) return maximum;
    if (value < minimum) return minimum;
    return value;
}

static void HUIDU_SetMotor(float left, float right)
{
    left = HUIDU_Limit(left, 0.0f, TRACK_MAX_SPEED);
    right = HUIDU_Limit(right, 0.0f, TRACK_MAX_SPEED);
    leftSpeed = left;
    rightSpeed = right;

    if (left > 0.0f)
    {
        DL_GPIO_setPins(PORTA_PORT, PORTA_PIN_14_PIN);
        DL_GPIO_clearPins(PORTA_PORT, PORTA_PIN_13_PIN);
        Set_Duty(left + 1.0f, 0);
    }
    else
    {
        DL_GPIO_clearPins(PORTA_PORT, PORTA_PIN_13_PIN | PORTA_PIN_14_PIN);
        Set_Duty(0.0f, 0);
    }

    if (right > 0.0f)
    {
        DL_GPIO_setPins(PORTA_PORT, PORTA_PIN_15_PIN);
        DL_GPIO_clearPins(PORTA_PORT, PORTA_PIN_16_PIN);
        Set_Duty(right + 1.0f, 1);
    }
    else
    {
        DL_GPIO_clearPins(PORTA_PORT, PORTA_PIN_15_PIN | PORTA_PIN_16_PIN);
        Set_Duty(0.0f, 1);
    }
}

void HUIDU(void)
{
    sensorPins[0] = DL_GPIO_readPins(HUIDU_PORT, HUIDU_PIN_4_PIN)  ? 0 : 1;
    sensorPins[1] = DL_GPIO_readPins(HUIDU_PORT, HUIDU_PIN_5_PIN)  ? 0 : 1;
    sensorPins[2] = DL_GPIO_readPins(HUIDU_PORT, HUIDU_PIN_6_PIN)  ? 0 : 1;
    sensorPins[3] = DL_GPIO_readPins(HUIDU_PORT, HUIDU_PIN_7_PIN)  ? 0 : 1;
    sensorPins[4] = DL_GPIO_readPins(HUIDU_PORT, HUIDU_PIN_8_PIN)  ? 0 : 1;
    sensorPins[5] = DL_GPIO_readPins(HUIDU_PORT, HUIDU_PIN_9_PIN)  ? 0 : 1;
    sensorPins[6] = DL_GPIO_readPins(HUIDU_PORT, HUIDU_PIN_10_PIN) ? 0 : 1;
    sensorPins[7] = DL_GPIO_readPins(HUIDU_PORT, HUIDU_PIN_11_PIN) ? 0 : 1;
}

void HUIDU_Track_Reset(void)
{
    currentTrackState = TRACK_CENTER;
    pendingTrackState = TRACK_CENTER;
    pendingStateCount = 0;
    lostLineCount = 0;
    requestedTrackSpeed = (float)basespeed;
    error1 = 0.0f;
    derivative1 = 0.0f;
    lastError = 0.0f;
    correction = 0.0f;
    leftSpeed = 0.0f;
    rightSpeed = 0.0f;
}

void HUIDU_Track_SetSpeed(float speed)
{
    requestedTrackSpeed = HUIDU_Limit(speed, TRACK_MIN_SPEED, TRACK_MAX_SPEED);
}

void HUIDU_Track_Stop(void)
{
    HUIDU_SetMotor(0.0f, 0.0f);
}

void Motor_Channel0_Test(void)
{
    /* 两组方向引脚都设置为原工程的前进方向。 */
    DL_GPIO_setPins(PORTA_PORT, PORTA_PIN_14_PIN);
    DL_GPIO_clearPins(PORTA_PORT, PORTA_PIN_13_PIN);

    DL_GPIO_setPins(PORTA_PORT, PORTA_PIN_15_PIN);
    DL_GPIO_clearPins(PORTA_PORT, PORTA_PIN_16_PIN);

    /* 只输出PWM通道0，通道1明确关闭。 */
    Set_Duty(0.0f, 0);
    Set_Duty(5.0f, 1);
}

void HUIDU_Track(void)
{
#if MOTOR_CHANNEL_TEST_ENABLE
    Motor_Channel0_Test();
    return;
#else
    TrackState detectedState;
    uint8_t activeCount = 0;
    float baseSpeed;
    float turnAmount;

    for (uint8_t i = 0; i < 8; i++)
    {
        if (sensorPins[i] != 0) activeCount++;
    }

    /* 完全丢线：立即取消差速直行，持续丢线则停车。 */
    if (activeCount == 0)
    {
        lostLineCount++;
        currentTrackState = TRACK_CENTER;
        pendingTrackState = TRACK_CENTER;
        pendingStateCount = 0;
        error1 = 0.0f;
        derivative1 = 0.0f;
        correction = 0.0f;
        lastError = 0.0f;
        HUIDU_test1 = 1;
        HUIDU_test4 = 0;

        if (lostLineCount >= TRACK_LOST_STOP_COUNT)
            HUIDU_SetMotor(0.0f, 0.0f);
        else
            HUIDU_SetMotor(TRACK_LOST_SPEED, TRACK_LOST_SPEED);
        return;
    }

    lostLineCount = 0;
    HUIDU_test1 = 0;

    /* 最外侧优先，2、3号和6、7号分别使用同一档普通纠偏。 */
    if (sensorPins[0] != 0)
        detectedState = TRACK_EMERGENCY_LEFT;
    else if (sensorPins[7] != 0)
        detectedState = TRACK_EMERGENCY_RIGHT;
    else if ((sensorPins[1] != 0) || (sensorPins[2] != 0))
        detectedState = TRACK_TURN_LEFT;
    else if ((sensorPins[5] != 0) || (sensorPins[6] != 0))
        detectedState = TRACK_TURN_RIGHT;
    else
        detectedState = TRACK_CENTER;

    if ((detectedState == TRACK_EMERGENCY_LEFT) ||
        (detectedState == TRACK_EMERGENCY_RIGHT) ||
        (detectedState == TRACK_CENTER))
    {
        /* 紧急状态和回到4、5号中心时立即生效。 */
        currentTrackState = detectedState;
        pendingTrackState = detectedState;
        pendingStateCount = 0;
    }
    else
    {
        /* 普通左右状态需要连续出现两次，过滤黑线边缘的瞬时跳变。 */
        if (detectedState != pendingTrackState)
        {
            pendingTrackState = detectedState;
            pendingStateCount = 1;
        }
        else if (pendingStateCount < TRACK_CONFIRM_COUNT)
        {
            pendingStateCount++;
        }

        if (pendingStateCount >= TRACK_CONFIRM_COUNT)
        {
            currentTrackState = detectedState;
        }
    }

    baseSpeed = HUIDU_Limit(requestedTrackSpeed, TRACK_MIN_SPEED, TRACK_MAX_SPEED);

    switch (currentTrackState)
    {
        case TRACK_TURN_LEFT:
            turnAmount = TRACK_NORMAL_TURN;
            break;

        case TRACK_TURN_RIGHT:
            turnAmount = -TRACK_NORMAL_TURN;
            break;

        case TRACK_EMERGENCY_LEFT:
            turnAmount = TRACK_EMERGENCY_TURN;
            baseSpeed -= 1.0f;
            break;

        case TRACK_EMERGENCY_RIGHT:
            turnAmount = -TRACK_EMERGENCY_TURN;
            baseSpeed -= 1.0f;
            break;

        case TRACK_CENTER:
        default:
            /* 回退到实测可稳定版本的中心圆弧前馈。 */
            turnAmount = -TRACK_CURVE_BIAS;
            break;
    }

    baseSpeed = HUIDU_Limit(baseSpeed, TRACK_MIN_SPEED, TRACK_MAX_SPEED);
    error1 = turnAmount;
    derivative1 = error1 - lastError;
    lastError = error1;
    correction = turnAmount;
    HUIDU_test4 = (int)(turnAmount * 10.0f);

    /* 统一方向系数依据实车结果修正传感器编号/电机安装方向差异。 */
    turnAmount *= TRACK_STEER_DIRECTION;
    HUIDU_SetMotor(baseSpeed - turnAmount, baseSpeed + turnAmount);
#endif
}

#include "head.h"

float leftSpeed,rightSpeed;
int basespeed=14,shot_test1=350;
int mode1_times=1,MODE1_flag1,MODE1_start;
int MODE2_start,MODE2_flag1,MODE2_flag2,MODE3_start,MODE4_start;

#define MODE1_SLOW_DOWN_ANGLE    30.0f
#define MODE1_MIN_SPEED           8.0f
#define MODE1_STOP_LEAD_ANGLE     3.0f

#define TASK_SLOW_DOWN_ANGLE      30.0f
#define TASK_MIN_SPEED             8.0f
#define TASK_STOP_LEAD_ANGLE       3.0f

static float mode1StartYaw = 0.0f;

volatile TaskState taskState = TASK_WAIT_FIRST_START;
volatile uint16_t taskSavedTargetAngle = 0U;
volatile float taskTravelledAngle = 0.0f;
volatile float taskRemainingAngle = 0.0f;

static float taskRunStartYaw = 0.0f;
static float taskRunTargetAngle = 0.0f;

static void Task_StopCar(void)
{
    HUIDU_Track_Stop();
    DL_GPIO_clearPins(PORTA_PORT, PORTA_motor_STBY_PIN);
}

static void Task_StartRun(float targetAngle)
{
    taskRunStartYaw = Rec_Yaw;
    taskRunTargetAngle = targetAngle;
    taskTravelledAngle = 0.0f;
    taskRemainingAngle = targetAngle;
    HUIDU_Track_Reset();
    HUIDU_Track_SetSpeed((float)basespeed);
    DL_GPIO_setPins(PORTA_PORT, PORTA_motor_STBY_PIN);
}

void Task_Control_Init(void)
{
    MODE1_start = 0;
    MODE2_start = 0;
    MODE3_start = 0;
    MODE4_start = 0;
    MODE1_flag1 = 0;
    MODE2_flag1 = 0;
    MODE2_flag2 = 0;

    taskState = TASK_WAIT_FIRST_START;
    taskSavedTargetAngle = 0U;
    taskTravelledAngle = 0.0f;
    taskRemainingAngle = 0.0f;

    /* Keep the unused legacy stepper controller at its current position. */
    angleError = motorPosition;
    Vision_Reset();
    Task_StopCar();
}

void Task_HandleStartButton(void)
{
    if (taskState == TASK_WAIT_FIRST_START)
    {
        /* Keep data received after power-on. Some K230 programs only send
         * once when the recognition result changes. */
        if (vision_valid != 0U)
            taskSavedTargetAngle = vision_locked_angle;
        Task_StartRun(360.0f);
        taskState = TASK_FIRST_RUNNING;
    }
    else if (taskState == TASK_WAIT_SECOND_START)
    {
        if (vision_valid == 0U)
            return;

        taskSavedTargetAngle = vision_locked_angle;
        Task_StartRun((float)taskSavedTargetAngle);
        taskState = TASK_SECOND_RUNNING;
    }
}

void Task_Update(void)
{
    float trackSpeed;

    if ((taskState != TASK_FIRST_RUNNING) &&
        (taskState != TASK_SECOND_RUNNING))
        return;

    taskTravelledAngle = fabsf(Rec_Yaw - taskRunStartYaw);
    taskRemainingAngle = taskRunTargetAngle - taskTravelledAngle;

    trackSpeed = (float)basespeed;
    if (taskRemainingAngle < TASK_SLOW_DOWN_ANGLE)
    {
        trackSpeed = TASK_MIN_SPEED +
                     ((float)basespeed - TASK_MIN_SPEED) *
                     taskRemainingAngle / TASK_SLOW_DOWN_ANGLE;
        if (trackSpeed < TASK_MIN_SPEED)
            trackSpeed = TASK_MIN_SPEED;
    }

    HUIDU_Track_SetSpeed(trackSpeed);
    HUIDU();
    HUIDU_Track();
    DL_GPIO_setPins(PORTA_PORT, PORTA_motor_STBY_PIN);

    if ((vision_valid != 0U) && (taskSavedTargetAngle == 0U))
        taskSavedTargetAngle = vision_locked_angle;

    if (taskTravelledAngle >=
        (taskRunTargetAngle - TASK_STOP_LEAD_ANGLE))
    {
        Task_StopCar();

        if (taskState == TASK_FIRST_RUNNING)
        {
            if (vision_valid != 0U)
                taskSavedTargetAngle = vision_locked_angle;
            taskState = TASK_WAIT_SECOND_START;
        }
        else
        {
            taskState = TASK_FINISHED;
            HC12_NotifyCarStopped();
        }
    }
}
	
void mode_1(void)
{
    float travelledYaw;
    float targetYaw;
    float remainingYaw;
    float trackSpeed;

    if(MODE1_flag1==0)
    {
        mode1StartYaw=Rec_Yaw;
        HUIDU_Track_Reset();
        MODE1_flag1=1;
    }

    travelledYaw=fabsf(Rec_Yaw-mode1StartYaw);
    targetYaw=(float)mode1_times*360.0f;
    remainingYaw=targetYaw-travelledYaw;

    /* 最后30度线性减速，减小断电后的惯性停车误差。 */
    trackSpeed=(float)basespeed;
    if(remainingYaw<MODE1_SLOW_DOWN_ANGLE)
    {
        trackSpeed=MODE1_MIN_SPEED+
                   ((float)basespeed-MODE1_MIN_SPEED)*
                   remainingYaw/MODE1_SLOW_DOWN_ANGLE;
        if(trackSpeed<MODE1_MIN_SPEED) trackSpeed=MODE1_MIN_SPEED;
    }

    HUIDU_Track_SetSpeed(trackSpeed);
    HUIDU();
	HUIDU_Track();
	DL_GPIO_setPins(PORTA_PORT, PORTA_motor_STBY_PIN);//电机使能信号

	if(travelledYaw>=targetYaw-MODE1_STOP_LEAD_ANGLE)
	{
	HUIDU_Track_Stop();
	MODE1_start=0;
	MODE1_flag1=0;
	DL_GPIO_clearPins(PORTA_PORT, PORTA_motor_STBY_PIN);//关闭模式1并移除使能信号
	}

}
//模式1，灰度寻迹

	
void mode_2(void)
{
	 
	if(MODE2_flag1==0)
	{
		angleError = Yaw_OUT;
	if(angleError<10)
		MODE2_flag1=1;
	}
	if(MODE2_flag1==1)
	{
		MODE2_flag2++;
		if(MODE2_flag2>30)
		{
			MODE2_flag1=2;
			MODE2_flag2=0;
		}
	}
	if(MODE2_flag1==2)
	{
		move_towards_target(shot_test1,Roll);
		angleError = servo_angle_x*0.1+Yaw_OUT;
		MODE2_flag2++;
		
		if(MODE2_flag2>70)
		{
		 DL_GPIO_setPins(PORTA_PORT, PORTA_Light_start_PIN);	
		}		
	}
 
}


void mode_3(void)
{
    HUIDU_Track_SetSpeed((float)basespeed);
    HUIDU();
	HUIDU_Track();
	DL_GPIO_setPins(PORTA_PORT, PORTA_motor_STBY_PIN);
	DL_GPIO_setPins(PORTA_PORT, PORTA_Light_start_PIN);
	
	move_towards_target(shot_test1,Roll);
	
	angleError = servo_angle_x*0.10-Rec_Yaw;
	
	
	if(Target_Yaw>mode1_times*360+80)
	{
	MODE1_start=0;
	DL_GPIO_clearPins(PORTA_PORT, PORTA_motor_STBY_PIN);
	DL_GPIO_clearPins(PORTA_PORT, PORTA_Light_start_PIN);
	}
}






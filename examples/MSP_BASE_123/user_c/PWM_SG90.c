#include "head.h"

int servo_angle_x = 0;  // 水平舵机角度
int servo_angle_y = 0;  // 垂直舵机角度

// 定义 PID 参数（根据实际系统调整）
const float Kp = 0.12;   // 比例系数
const float Ki = 0.10;   // 积分系数
const float Kd = 0.22;   // 微分系数

// 限制积分累计的范围，防止积分饱和
const float integral_limit = 5.0;   // 积分限幅值
const float output_limit = 15.0;    // 输出限幅值（最大调整量）

// 低通滤波系数（0 < alpha < 1，值越小滤波效果越强，响应越慢）
const float filter_alpha = 0.4;      // 可根据需要调整，例如 0.3 ~ 0.5

// 静态变量保存 PID 状态（仅在函数内使用，确保多次调用间状态连续）
static float integral_x = 0.0;        // 积分累计
static float prev_error_x = 0.0;      // 上一次误差（用于微分）
static float filtered_angle_x = 0.0;  // 滤波后的角度（低通滤波状态）

void move_towards_target(float target_x, float current_x) {
    // 计算当前误差
    float error = target_x - current_x;
    
    // 死区处理：误差小于 3 时认为已到达，停止移动并复位积分项
    if (fabs(error) < 3.0) {
        // 到达目标，清除积分累积，防止下次启动时过冲
        integral_x = 0.0;
        prev_error_x = 0.0;
        return;
    }
    
    // ----- PID 计算 -----
    // 比例项
    float P = Kp * error;
    
    // 积分项（带限幅）
    integral_x += error;                     // 误差累加
    // 积分限幅，避免积分饱和
    if (integral_x > integral_limit) integral_x = integral_limit;
    if (integral_x < -integral_limit) integral_x = -integral_limit;
    float I = Ki * integral_x;
    
    // 微分项（使用当前误差与上次误差的差）
    float D = Kd * (error - prev_error_x);
    prev_error_x = error;                    // 更新上次误差
    
    // 总控制量
    float adjustment = P + I + D;
    
    // 输出限幅，防止舵机单次调整过大
    if (adjustment > output_limit) adjustment = output_limit;
    if (adjustment < -output_limit) adjustment = -output_limit;
    
    // 更新原始角度（未滤波）
    int raw_angle = servo_angle_x + (int)adjustment;
    
    // 限幅处理（确保角度在 -5400 ~ 5400 范围内）
    if (raw_angle > 5400) raw_angle = 5400;
    if (raw_angle < -5400) raw_angle = -5400;
    
    // ----- 低通滤波 -----
    // 一阶低通滤波：filtered = alpha * raw + (1 - alpha) * previous_filtered
    // 第一次调用时需要初始化滤波状态，用原始值填充
    if (filtered_angle_x == 0.0 && raw_angle != 0) {
        filtered_angle_x = (float)raw_angle;   // 初始化
    }
    filtered_angle_x = filter_alpha * (float)raw_angle + (1.0f - filter_alpha) * filtered_angle_x;
    
    // 将滤波后的角度赋给舵机指令（保留整数精度）
    servo_angle_x = (int)filtered_angle_x;
    
  
}
//控制横向舵机  角度
void Set_Servo_Angle_X( float angle) {
	
	
//    if(step==0)		
//	{ 
//		if(Flag_mode4_start!=1)
//		step2=kii;
//		step=1;
//	}
//	if(step==1)
//    kii=angle+step2;
	kii=angle;
 
}


////控制纵向舵机 角度
//void Set_Servo_Angle_Y( int angle) { 
//	
//   uint32_t pulse_us;
//	
//    if (angle > 0) {	
//		 if(angle>=600)angle=600;
//         pulse_us = (uint32_t)(1.111f * (angle+750) + 500);	
//		
//    }
//	
//	   if (angle <= 0) {
//		 if(angle<=-200)angle=-200;
//         pulse_us = (uint32_t)(1.111f * (angle+750) + 500);
//    }
//       Servo_Angle = angle;
//    DL_TimerA_setCaptureCompareValue(Steer_INST, pulse_us, GPIO_Steer_C1_IDX);
//}
					  

void Servo_Set(float x,int y,float j,int k,int l)
{    
	
	Set_Servo_Angle_X(x+j);//横向
//	Set_Servo_Angle_Y(y+k);//纵向
	delay_ms(l);
}


//底层舵机  前俩问不需要用到
//void Set_Servo_Angle_D( int speed) {     //angle -1000至1000  旋转速度
//	
//	int pulse_us,angle_turn;
//	angle_turn=-speed;
//	if(angle_turn>0)
//	{
//	if(angle_turn>1000)   
//	angle_turn=1000;
//	pulse_us=-angle_turn+1450;  //死区 向右
//	}
//	if(angle_turn<0)
//	{ 
//    if(angle_turn<-1000)  
//	angle_turn=-1000;
//	pulse_us=-angle_turn+1550;  //死区
//	}
//		
//    DL_TimerG_setCaptureCompareValue(PWM_0_INST, pulse_us, GPIO_PWM_0_C0_IDX);
//}



/* 
    舵机向定点移动  目标（X，Y） 实际（X，Y）
*/

//void move_towards_target(float target_x,float target_y,float current_x,float current_y) {
//    // 计算误差（目标坐标与当前坐标的差值）
//     error_x = target_x - current_x;
//     error_y = current_y - target_y ;
//    
//    // 当误差小于阈值时，认为已到达目标点
//    if (fabs(error_x) < 1 && fabs(error_y) < 1) {
//	
//        return;  // 停止移动
//    }
//    
//    // 计算舵机调整量（比例控制）
//     adjustment_x = Kp_x * error_x;
//     adjustment_y = Kp_y * error_y;
//    
//    // 更新舵机角度
//    servo_angle_x += (int)adjustment_x;
//    servo_angle_y += (int)adjustment_y;
//    
//    // 限幅处理（确保角度在-900~900范围内）
//    if (servo_angle_x > 5400)  servo_angle_x = 5400;
//    if (servo_angle_x < -5400) servo_angle_x = -5400;
//    if (servo_angle_y > 600)  servo_angle_y = 600;
//    if (servo_angle_y < -200) servo_angle_y = -200;
//    
//    // 更新当前坐标（根据舵机角度反推，此处简化处理）
//    current_x += adjustment_x;
//    current_y += adjustment_y;
//    
//    // 控制舵机转动
//	Servo_Set((float)(servo_angle_x*0.1),servo_angle_y,0,0,4);

//}


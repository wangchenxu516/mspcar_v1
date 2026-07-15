#include "head.h"


float Actual_Yaw,Target_Yaw,Yaw_OUT,Rec_Yaw;
extern int16_t gz_avg;

#define GYRO_SENSITIVITY_2000DPS 16.4f     // 陀螺仪灵敏度（±2000°/s量程下，16.4 LSB/(°/s)
#define SAMPLING_TIME 0.01f                // 采样时间：定时器中断10ms = 0.01秒
  
void Actual_Yaw_cal()                      //mpu6050计算代码
 {
   
	 MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);	       
    float gyro_z_dps = (GZ-gz_avg) / GYRO_SENSITIVITY_2000DPS; // 转成°/s
    float angle_z_delta = gyro_z_dps * SAMPLING_TIME;          // 10ms内的角度变化
	if(abs(GZ-gz_avg)>10)
	{ // Actual_Yaw+= angle_z_delta;
	  
		Rec_Yaw+=    angle_z_delta;
	
	}
//   Actual_Yaw = fmod(Actual_Yaw, 360.0f);
//   if (Actual_Yaw < 0.0f) {
//    Actual_Yaw += 360.0f;
//    }
  
	Yaw_OUT = Target_Yaw - Rec_Yaw;
    // 环形归一化：确保差值在 [-180°, 180°]（最短路径）
    if (Yaw_OUT > 180.0f)
        Yaw_OUT -= 360.0f;
    else if (Yaw_OUT < -180.0f)
        Yaw_OUT += 360.0f;

}
 

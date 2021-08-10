#include "chassis.h"

#include <math.h>

#include "can.h"
#include "chassis_thread.h"
#include "gimbal.h"
#include "pid.h"
#include "wattmeter.h"

#define CHASSIS_ROTATE_RATIO 1.2F      // 底盘旋转速度倍率
#define POWER_CTL_KP         0.00045F  // 功率控制 PID

m3508_t            chassis_data[4];
chassis_behavior_t chassis_behavior;

static pid_type_t pid_speed[4];                              // 底盘电机速度 PID
const fp32        pid_speed_param[3] = {10.0F, 0.7F, 0.0F};  // PID 参数
static float      wheel_speed_ratio  = 20.0F;                // 麦轮速度倍率
static rt_timer_t chassis_timer;                             // 底盘电机 PID 计算定时器

static void chassis_timer_timeout(void *parameter);

void chassis_init(void) {
    /* 初始化 PID */
    for (uint8_t i = 0; i < 4; i++) {
        pid_init(pid_speed + i, PID_POSITION, pid_speed_param, 16000, 16000);
    }
    /* clang-format off */
    /* 初始化底盘定时器 */
    chassis_timer = rt_timer_create("chassis",
                                    chassis_timer_timeout,
                                    RT_NULL,
                                    10,
                                    RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_HARD_TIMER);
    rt_timer_start(chassis_timer);
    /* clang-format on */
}

void chassis_timer_timeout(void *parameter) {
    chassis_handler(chassis_behavior.x_speed, chassis_behavior.y_speed, chassis_behavior.angular_speed);
}

/**
 * @brief 底盘周期控制函数
 *
 * @param speed_x X 方向速度
 * @param speed_y Y 方向速度
 * @param speed_omega 旋转速度
 */
void chassis_handler(int16_t speed_x, int16_t speed_y, int16_t speed_omega) {
    int16_t speed_x_mapped, speed_y_mapped;
    double  PI = acos(-1);
    float   theta_rad;
    int16_t mech_deg;

    if (chassis_state == CHASSIS_STATE_STOP) {
        chassis_set_current(0, 0, 0, 0);
        return;
    }

    /* 角度计算 */
    BOUNDARY_PROCESS(mech_deg, YAW_ZERO_DEGREE - gimbal_data.yaw.degree, 0, 8192);
    theta_rad = 90.0F - (mech_deg) * (360.0F / 8192.0F);
    theta_rad = theta_rad * (PI / 180);

    /* 速度坐标变换 */
    speed_x_mapped = -speed_y * cosf(theta_rad) + speed_x * sinf(theta_rad);
    speed_y_mapped = speed_y * sinf(theta_rad) + speed_x * cosf(theta_rad);

    /* 麦轮速度解算 */
    chassis_data[0].expect_speed =
        (-speed_x_mapped + speed_y_mapped - speed_omega * CHASSIS_ROTATE_RATIO) * wheel_speed_ratio;  // 左前
    chassis_data[1].expect_speed =
        (speed_x_mapped + speed_y_mapped - speed_omega * CHASSIS_ROTATE_RATIO) * wheel_speed_ratio;  // 左后
    chassis_data[2].expect_speed =
        (speed_x_mapped - speed_y_mapped - speed_omega * CHASSIS_ROTATE_RATIO) * wheel_speed_ratio;  // 右后
    chassis_data[3].expect_speed =
        (-speed_x_mapped - speed_y_mapped - speed_omega * CHASSIS_ROTATE_RATIO) * wheel_speed_ratio;  // 右前

    /* 速度环 PID 计算 */
    for (uint8_t i = 0; i < 4; i++) {
        chassis_data[i].set_current = pid_calc(pid_speed + i, chassis_data[i].speed, chassis_data[i].expect_speed);
    }

    /* 输出电机电流 */
    chassis_set_current(chassis_data[0].set_current, chassis_data[1].set_current, chassis_data[2].set_current,
                        chassis_data[3].set_current);
}

/**
 * @brief 设置底盘电机电流
 *
 * @param motor_1 第一个3508的电流，范围[-16384, 16384]
 * @param motor_2 第二个3508的电流，范围[-16384, 16384]
 * @param motor_3 第三个3508的电流，范围[-16384, 16384]
 * @param motor_4 第四个3508的电流，范围[-16384, 16384]
 */
void chassis_set_current(int16_t motor_1, int16_t motor_2, int16_t motor_3, int16_t motor_4) {
    CAN_TxHeaderTypeDef tx_header;
    uint8_t             tx_data[8];
    uint32_t            tx_mailbox;

    /* 配置消息头 */
    tx_header.StdId              = 0x200U;
    tx_header.IDE                = CAN_ID_STD;
    tx_header.RTR                = CAN_RTR_DATA;
    tx_header.DLC                = 8;
    tx_header.TransmitGlobalTime = DISABLE;

    /* 计算发送数据 */
    tx_data[0] = motor_1 >> 8;
    tx_data[1] = motor_1;
    tx_data[2] = motor_2 >> 8;
    tx_data[3] = motor_2;
    tx_data[4] = motor_3 >> 8;
    tx_data[5] = motor_3;
    tx_data[6] = motor_4 >> 8;
    tx_data[7] = motor_4;

    HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &tx_mailbox);
}

/**
 * @brief 更新底盘单电机数据
 *
 * @param id 电机ID
 * @param data 电机原始报文数据
 */
void chassis_update_data(uint8_t id, uint8_t data[8]) {
    /* 结构体指针 */
    m3508_t *pData;

    pData = chassis_data + (id - 1);

    /* 处理数据 */
    pData->degree     = (data[0] << 8) | data[1];
    pData->speed      = (data[2] << 8) | data[3];
    pData->current    = (data[4] << 8) | data[5];
    pData->temprature = data[6];
}

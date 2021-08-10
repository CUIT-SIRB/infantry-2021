#include "gimbal.h"

#include "bsp_imu.h"
#include "can.h"
#include "chassis_thread.h"
#include "dmalib.h"
#include "dr16.h"
#include "pid.h"
#include "shoot.h"

gimbal_data_t     gimbal_data;  // 云台数据
gimbal_behavior_t gimbal_behavior = {
    .pitch_degree = -300,
};  // 云台控制

rt_event_t gimbal_event;         // 云台事件集
rt_timer_t gimbal_handle_timer;  // 云台初始化用定时器

/**
 * @brief Yaw轴 PID
 *
 */
static pid_type_t pid_yaw_speed;
static const fp32 pid_yaw_speed_param[3] = {20.0F, 0.5F, 0.0F};
static pid_type_t pid_yaw_angle;
static const fp32 pid_yaw_angle_param[3] = {8.0F, 0.0F, 1.0F};

static pid_type_t pid_pitch_speed;
static const fp32 pid_pitch_speed_param[3] = {10.0F, 0.5F, 0.0F};
static pid_type_t pid_pitch_angle;
static const fp32 pid_pitch_angle_param[3] = {7.0F, 0.00F, 0.5F};

static void gimbal_timer_timeout(void *parameter);
static void gimbal_pitch_limit(void);

/**
 * @brief 云台初始化函数
 *
 */
int8_t gimbal_init(void) {
    /* 初始化事件集 */
    gimbal_event = rt_event_create("gimbal", RT_IPC_FLAG_PRIO);

    /* clang-format off */
    /* 初始化定时器 */
    gimbal_handle_timer = rt_timer_create("gim_handle",
                                          gimbal_timer_timeout,
                                          RT_NULL,
                                          1,
                                          RT_TIMER_FLAG_HARD_TIMER | RT_TIMER_FLAG_PERIODIC);
    /* clang-format on */

    /* Yaw PID 初始化 */
    pid_init(&pid_yaw_speed, PID_POSITION, pid_yaw_speed_param, 20000, 15000);
    pid_init(&pid_yaw_angle, PID_POSITION, pid_yaw_angle_param, 5000, 0);
    /* Pitch PID 初始化 */
    pid_init(&pid_pitch_speed, PID_POSITION, pid_pitch_speed_param, 20000, 15000);
    pid_init(&pid_pitch_angle, PID_POSITION, pid_pitch_angle_param, 5000, 0);

    /* IMU 初始化 */
    mpu_device_init();

    rt_thread_mdelay(100);

    /* 启动定时器 */
    rt_timer_start(gimbal_handle_timer);
    /* Pitch 缓慢抬头 */
    for (int i = -400; i < 0; i++) {
        gimbal_behavior.pitch_degree = i;
        rt_thread_mdelay(1);
    }
    /* 校准完成事件 */
    rt_event_send(gimbal_event, GIMBAL_EVENT_READY);
    return 0;
}

/**
 * @brief 云台控制定时器超时函数
 *
 * @param parameter
 */
void gimbal_timer_timeout(void *parameter) {
    gimbal_handle();
}

/**
 * @brief 更新云台电机数据
 *
 * @param ID 电机ID
 * @param Data 电机原始数据
 */
void gimbal_update_data(uint8_t id, uint8_t data[8]) {
    if (id == 1) {
        /* Yaw 轴电机数据 */
        gimbal_data.yaw.degree = (data[0] << 8) | data[1];
        // *用原始速度即可，因为控制频率上去之后速度大小和电机反馈的差不多
        gimbal_data.yaw.speed      = (data[2] << 8) | data[3];
        gimbal_data.yaw.current    = (data[4] << 8) | data[5];
        gimbal_data.yaw.temprature = data[6];
    } else if (id == 2) {
        /* Pitch 轴电机数据 */
        gimbal_data.pitch.degree     = (data[0] << 8) | data[1];
        gimbal_data.pitch.speed      = (data[2] << 8) | data[3];
        gimbal_data.pitch.current    = (data[4] << 8) | data[5];
        gimbal_data.pitch.temprature = data[6];
    } else if (id == 3) {
        gimbal_data.plunk.degree = (data[0] << 8) | data[1];
        gimbal_data.plunk.speed  = (data[2] << 8) | data[3];
        gimbal_data.plunk.torque = (data[4] << 8) | data[5];
    }
}

/**
 * @brief 设置云台电机的电压
 *
 * @param Yaw GM6020的电压，范围[-30000, 30000]
 * @param Pitch 6623的电压，范围[-5000, 5000]
 * @param Plunk 2006的电压，范围[-10000, 10000]
 */
void gimbal_set_voltage(int16_t yaw, int16_t pitch, int16_t plunk) {
    CAN_TxHeaderTypeDef tx_header;
    uint8_t             tx_data[8];
    uint32_t            tx_mailbox;

    /* 配置消息头 */
    tx_header.StdId              = 0x1FFU;
    tx_header.IDE                = CAN_ID_STD;
    tx_header.RTR                = CAN_RTR_DATA;
    tx_header.DLC                = 8;
    tx_header.TransmitGlobalTime = DISABLE;

    /* 计算发送数据 */
    tx_data[0] = yaw >> 8;
    tx_data[1] = yaw;
    tx_data[2] = pitch >> 8;
    tx_data[3] = pitch;
    tx_data[4] = plunk >> 8;
    tx_data[5] = plunk;
    tx_data[6] = 0x00;
    tx_data[7] = 0x00;

    HAL_CAN_AddTxMessage(&hcan1, &tx_header, tx_data, &tx_mailbox);
}

/**
 * @brief 云台周期控制函数
 *
 */
void gimbal_handle(void) {
    extern int16_t plunk_voltage;
    float          yaw_speed_expect, pitch_speed_expect;
    int16_t        yaw_voltage, pitch_voltage;

    /* 获取 IMU 数据 */
    mpu_get_data();

    /* Pitch 限位 */
    gimbal_pitch_limit();

    /* 优弧劣弧处理 */
    int16_t yaw_degree_expect;
    BOUNDARY_PROCESS(yaw_degree_expect, gimbal_data.yaw.degree, gimbal_behavior.yaw_degree + YAW_ZERO_DEGREE, 8192);

    /* 计算角度环 PID */
    yaw_speed_expect = pid_calc(&pid_yaw_angle, yaw_degree_expect, gimbal_behavior.yaw_degree + YAW_ZERO_DEGREE);
    pitch_speed_expect =
        pid_calc(&pid_pitch_angle, gimbal_data.pitch.degree, gimbal_behavior.pitch_degree + PITCH_ZERO_DEGREE);

    /* 除了停止模式采用定角度控制，其余模式均采用定速度控制 */
    if (chassis_state == CHASSIS_STATE_STOP) {
        yaw_voltage = pid_calc(&pid_yaw_speed, mpu_data.gz, yaw_speed_expect);
    } else {
        yaw_voltage = pid_calc(&pid_yaw_speed, mpu_data.gz, gimbal_behavior.yaw_speed);
    }
    /* 除了 PC 控制模式采用定速度控制，其余模式均采用定角度控制 */
    if (chassis_state == CHASSIS_STATE_PC_CTL) {
        pitch_voltage = pid_calc(&pid_pitch_speed, mpu_data.gy, gimbal_behavior.pitch_speed);
    } else {
        pitch_voltage = pid_calc(&pid_pitch_speed, mpu_data.gy, pitch_speed_expect);
    }

    gimbal_set_voltage(yaw_voltage, pitch_voltage, plunk_voltage);
}

/**
 * @brief Pitch 轴角度限制
 * 
 */
void gimbal_pitch_limit(void) {
    uint16_t pitch_degree_min;

    /* 角度限位 */
    gimbal_behavior.pitch_degree = gimbal_behavior.pitch_degree < -300 ? -300 : gimbal_behavior.pitch_degree;
    gimbal_behavior.pitch_degree = gimbal_behavior.pitch_degree > 600 ? 600 : gimbal_behavior.pitch_degree;

    /* 速度限位 */
    if (chassis_spinning) {
        pitch_degree_min = PITCH_ZERO_DEGREE - 260;
    } else {
        pitch_degree_min = PITCH_ZERO_DEGREE - 300;
    }

    if (gimbal_data.pitch.degree <= pitch_degree_min && gimbal_behavior.pitch_speed < 0) {
        gimbal_behavior.pitch_speed = 0;
    } else if (gimbal_data.pitch.degree >= PITCH_ZERO_DEGREE + 550 && gimbal_behavior.pitch_speed > 0) {
        gimbal_behavior.pitch_speed = 0;
    }
}

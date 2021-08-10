#ifndef __REFEREE_H
#define __REFEREE_H

#include <stdbool.h>

#include "main.h"


#define REFEREE_LEN_HEADER 5  //帧头长度
#define REFEREE_LEN_CMDID  2  // cmd_id长度
#define REFEREE_LEN_TAIL   2  //帧尾长度

#define REFEREE_FRAME_HEADER 0xA5  //数据帧起始字节，固定值为 0xA5

/* 帧头偏移量 */
#define REFEREE_OFFSET_SOF         0  //帧头SOF偏移量
#define REFEREE_OFFSET_DATA_LENGTH 1  //帧头数据长度偏移量
#define REFEREE_OFFSET_SEQ         3  //帧头包序号偏移量
#define REFEREE_OFFSET_CRC8        4  //帧头CRC8偏移量

/* 帧内有效数据包偏移量 */
#define REFEREE_OFFSET_DATA (REFEREE_LEN_HEADER + REFEREE_LEN_CMDID)

/* com_id 命令码 ID */
#define ID_GAME_STATUS              0x0001  //比赛状态
#define ID_GAME_RESULT              0X0002  //比赛结果
#define ID_GAME_ROBOT_HP            0X0003  //机器人血量数据
#define ID_GAME_FIELD_EVENT         0x0101  //场地事件数据
#define ID_SUPPLY_PROJECTILE_ACTION 0x0102  //补给站动作标识
#define ID_REFEREE_WARNING          0x0104  //裁判警告信息
#define ID_DART_REMAIN_TIME         0x0105  //飞镖发射口倒计时
#define ID_GAME_ROBOT_STATUS        0x0201  //比赛机器人状态
#define ID_POWER_HEAT_DATA          0x0202  //实时功率热量数据
#define ID_GAME_ROBOT_POS           0x0203  //机器人位置
#define ID_BUFF                     0x0204  //机器人增益
#define ID_ROBOT_HURT               0x0206  //伤害状态
#define ID_SHOOT_DATA               0x0207  //实时射击信息
#define ID_REMAIN_BULLET            0x0208  //子弹剩余发射数
#define ID_RFID_STATUS              0x0209  //机器人 RFID 状态

/* 比赛状态数据： 0x0001。发送频率： 1Hz */
typedef __packed struct {
    uint8_t  game_type : 4;      //比赛类型
    uint8_t  game_progress : 4;  //当前比赛阶段
    uint16_t stage_remain_time;
    uint64_t SyncTimeStamp;
} ext_game_status_t;

/* 比赛结果数据： 0x0002。发送频率：比赛结束后发送 */
typedef __packed struct {
    uint8_t winner;  // 0 平局 1 红方胜利 2 蓝方胜利
} ext_game_result_t;

/* 机器人血量数据： 0x0003。发送频率： 1Hz */
typedef __packed struct {
    uint16_t red_1_robot_HP;
    uint16_t red_2_robot_HP;
    uint16_t red_3_robot_HP;
    uint16_t red_4_robot_HP;
    uint16_t red_5_robot_HP;
    uint16_t red_7_robot_HP;
    uint16_t red_outpost_HP;  //红方前哨站血量
    uint16_t red_base_HP;     //红方基地血量
    uint16_t blue_1_robot_HP;
    uint16_t blue_2_robot_HP;
    uint16_t blue_3_robot_HP;
    uint16_t blue_4_robot_HP;
    uint16_t blue_5_robot_HP;
    uint16_t blue_7_robot_HP;
    uint16_t blue_outpost_HP;
    uint16_t blue_base_HP;
} ext_game_robot_HP_t;

/* clang-format off */
/* 场地事件数据： 0x0101。发送频率： 1Hz 周期发送 */
typedef __packed struct {
    uint32_t event_type;
} ext_event_data_t;
/* clang-format on */

/* 补给站动作标识： 0x0102。发送频率：动作触发后发送 */
typedef __packed struct {
    uint8_t supply_projectile_id;    //补给站口 ID
    uint8_t supply_robot_id;         //补弹机器人 ID
    uint8_t supply_projectile_step;  //出弹口开闭状态
    uint8_t supply_projectile_num;   //补弹数量
} ext_supply_projectile_action_t;

/* 裁判警告信息： cmd_id (0x0104)。发送频率：警告发生后发送 */
typedef __packed struct {
    uint8_t level;          //警告等级
    uint8_t foul_robot_id;  //犯规机器人 ID
} ext_referee_warning_t;

/* clang-format off */
/* 飞镖发射口倒计时：cmd_id (0x0105)。发送频率：1Hz周期发送，发送范围：己方机器人 */
typedef __packed struct {
    uint8_t dart_remaining_time;
} ext_dart_remaining_time_t;
/* clang-format on */

/* 比赛机器人状态： 0x0201。发送频率： 10Hz */
typedef __packed struct {
    uint8_t  robot_id;                        //机器人 ID
    uint8_t  robot_level;                     //机器人等级
    uint16_t remain_HP;                       //机器人剩余血量
    uint16_t max_HP;                          //机器人上限血量
    uint16_t shooter_id1_17mm_cooling_rate;   //机器人 1 号 17mm 枪口每秒冷却值
    uint16_t shooter_id1_17mm_cooling_limit;  //机器人 1 号 17mm 枪口热量上限
    uint16_t shooter_id1_17mm_speed_limit;    //机器人 1 号 17mm 枪口上限速度 单位 m/s
    uint16_t shooter_id2_17mm_cooling_rate;   //机器人 2 号 17mm 枪口每秒冷却值
    uint16_t shooter_id2_17mm_cooling_limit;  //机器人 2 号 17mm 枪口热量上限
    uint16_t shooter_id2_17mm_speed_limit;    //机器人 2 号 17mm 枪口上限速度 单位 m/s
    uint16_t shooter_id1_42mm_cooling_rate;   //机器人 42mm 枪口每秒冷却值
    uint16_t shooter_id1_42mm_cooling_limit;  //机器人 42mm 枪口热量上限
    uint16_t shooter_id1_42mm_speed_limit;    //机器人 42mm 枪口上限速度 单位 m/s
    uint16_t chassis_power_limit;             //机器人底盘功率限制上限
    uint8_t  mains_power_gimbal_output : 1;   // gimbal 口输出： 1 为有 24V 输出， 0 为无 24v 输出；
    uint8_t  mains_power_chassis_output : 1;  // chassis 口输出： 1 为有 24V 输出， 0 为无 24v 输出；
    uint8_t  mains_power_shooter_output : 1;  // shooter 口输出： 1 为有 24V 输出， 0 为无 24v 输出；
} ext_game_robot_status_t;

/* 实时功率热量数据： 0x0202。 发送频率： 50Hz */
typedef __packed struct {
    volatile uint16_t chassis_volt;     //底盘输出电压 单位 毫伏
    volatile uint16_t chassis_current;  //底盘输出电流 单位 毫安
    volatile float    chassis_power;    //底盘输出功率 单位 W 瓦
    volatile uint16_t chassis_power_buffer;  //底盘功率缓冲 单位 J 焦耳， 备注：飞坡根据规则增加至 250J
    uint16_t          shooter_id1_17mm_cooling_heat;  // 1 号 17mm 枪口热量
    uint16_t          shooter_id2_17mm_cooling_heat;  // 2 号 17mm 枪口热量
    uint16_t          shooter_id1_42mm_cooling_heat;  // 42mm 枪口热量
} ext_power_heat_data_t;

/* 机器人位置： 0x0203。发送频率： 10Hz */
typedef __packed struct {
    float x;  //位置 x 坐标，单位 m，下同
    float y;
    float z;
    float yaw;  //位置枪口，单位度
} ext_game_robot_pos_t;

/* clang-format off */
/* 机器人增益： 0x0204。发送频率： 1Hz 周期发送 */
typedef __packed struct {
    uint8_t power_rune_buff;
} ext_buff_t;
/* clang-format on */

/* 伤害状态： 0x0206。发送频率：伤害发生后发送 */
typedef __packed struct {
    uint8_t armor_id : 4;   /* 当血量变化类型为装甲伤害，
                            代表装甲 ID，其中数值为 0-4 号代表机器人的五个装甲片，
                            其他血量变化类型，该变量数值为 0 */
    uint8_t hurt_type : 4;  //血量变化类型
} ext_robot_hurt_t;

/* 实时射击信息： 0x0207。发送频率：射击后发送 */
typedef __packed struct {
    uint8_t bullet_type;   //子弹类型: 1： 17mm 弹丸 2： 42mm 弹丸
    uint8_t shooter_id;    //发射机构 ID
    uint8_t bullet_freq;   //子弹射频 单位 Hz
    float   bullet_speed;  //子弹射速 单位 m/s
} ext_shoot_data_t;

/* 子弹剩余发射数： 0x0208。发送频率： 10Hz 周期发送， 所有机器人发送 */
typedef __packed struct {
    uint16_t bullet_remaining_num_17mm;  /* 17mm 子弹剩余发射数目,
                                         联盟赛: 全队步兵与英雄剩余可发射 17mm 弹丸总量 */
    uint16_t bullet_remaining_num_42mm;  // 42mm 子弹剩余发射数目
    uint16_t coin_remaining_num;         // 剩余金币数量
} ext_bullet_remaining_t;

/* clang-format off */
/* 机器人 RFID 状态： 0x0209。发送频率： 1Hz */
typedef __packed struct {
    uint32_t rfid_status;
} ext_rfid_status_t;
/* clang-format on */

/*********** 机器人间交互数据 **********/
/* 交互数据接收信息： 0x0301 */
typedef __packed struct {
    uint16_t data_cmd_id;  //数据段的内容 ID
    uint16_t sender_ID;
    uint16_t receiver_ID;
} ext_student_interactive_header_data_t;

/* 学生机器人间通信 cmd_id 0x0301， 内容 ID:0x0200~0x02FF */
typedef __packed struct {
    uint8_t data[128];  //此处大小可修改
} robot_interactive_data_t;

/* 客户端删除图形 机器人间通信： 0x0301 */
typedef __packed struct {
    uint8_t operate_tpye;
    uint8_t layer;
} ext_client_custom_graphic_delete_t;

/* 图形数据 */
typedef __packed struct {
    uint8_t  graphic_name[3];
    uint32_t operate_tpye : 3;
    uint32_t graphic_tpye : 3;
    uint32_t layer : 4;
    uint32_t color : 4;
    uint32_t start_angle : 9;
    uint32_t end_angle : 9;
    uint32_t width : 10;
    uint32_t start_x : 11;
    uint32_t start_y : 11;
    uint32_t radius : 10;
    uint32_t end_x : 11;
    uint32_t end_y : 11;
} graphic_data_struct_t;

/* clang-format off */
/* 客户端绘制一个图形 机器人间通信： 0x0301 */
typedef __packed struct {
    graphic_data_struct_t grapic_data_struct;
} ext_client_custom_graphic_single_t;

/* 客户端绘制二个图形 机器人间通信： 0x0301 */
typedef __packed struct {
    graphic_data_struct_t grapic_data_struct[2];
} ext_client_custom_graphic_double_t;

/* 客户端绘制五个图形 机器人间通信： 0x0301 */
typedef __packed struct {
    graphic_data_struct_t grapic_data_struct[5];
} ext_client_custom_graphic_five_t;

/* 客户端绘制七个图形 机器人间通信： 0x0301 */
typedef __packed struct {
    graphic_data_struct_t grapic_data_struct[7];
} ext_client_custom_graphic_seven_t;
/* clang-format on */

/* 客户端绘制字符 机器人间通信： 0x0301 */
typedef __packed struct {
    graphic_data_struct_t grapic_data_struct;
    uint8_t               data[30];
} ext_client_custom_character_t;

/* 发送数据结构体 */
typedef struct {
    uint16_t                              cmd_id;
    ext_student_interactive_header_data_t data_header;
    uint8_t                               data[107];
    uint16_t                              frame_tail;
} referee_send_data_t;

typedef enum {
    EmptyOperation = 0, /* 空操作 */
    AddGrapic,          /* 增加图形 */
    ModifyGrapic,       /* 修改图形 */
    DeleteGrapic        /* 删除图形 */
} GrapicOperation;

/* 外部声明裁判系统接收数据 */
extern ext_game_status_t              game_status;               //比赛状态
extern ext_game_result_t              game_result;               //比赛结果
extern ext_game_robot_HP_t            game_robot_HP;             //机器人血量数据
extern ext_event_data_t               game_field_event;          //场地事件数据
extern ext_supply_projectile_action_t supply_projectile_action;  //补给站动作标识
extern ext_referee_warning_t          judge_warning;             //裁判警告信息
extern ext_game_robot_status_t        game_robot_status;         //比赛机器人状态
extern ext_power_heat_data_t          power_heat_data;           //实时功率热量数据
extern ext_game_robot_pos_t           game_robot_pos;            //机器人位置
extern ext_buff_t                     robot_buff;                //机器人增益
extern ext_robot_hurt_t               robot_hurt;                //伤害状态
extern ext_shoot_data_t               shoot_data;                //实时射击信息
extern ext_rfid_status_t              rfid_status;               //机器人 RFID 状态

/* 裁判系统接收缓冲区大小 */
#define REFEREE_RECV_BUF_SIZE 255

extern uint8_t RefereeRecvBuf[REFEREE_RECV_BUF_SIZE];

bool Referee_Decode(uint8_t *RefereeRecvBuf);
int  Referee_ConfigFrameData(uint16_t cmd_id, void *data, int length);

#endif  //__REFEREE_H

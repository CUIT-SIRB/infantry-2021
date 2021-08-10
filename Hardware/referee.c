#include "referee.h"

#include "crc.h"
#include "referee_thread.h"
#include "usart.h"

/* 定义裁判系统接收数据 */
ext_game_status_t              game_status;               //比赛状态
ext_game_result_t              game_result;               //比赛结果
ext_game_robot_HP_t            game_robot_HP;             //机器人血量数据
ext_event_data_t               game_field_event;          //场地事件数据
ext_supply_projectile_action_t supply_projectile_action;  //补给站动作标识
ext_referee_warning_t          judge_warning;             //裁判警告信息
ext_game_robot_status_t        game_robot_status;         //比赛机器人状态
ext_power_heat_data_t          power_heat_data;           //实时功率热量数据
ext_game_robot_pos_t           game_robot_pos;            //机器人位置
ext_buff_t                     robot_buff;                //机器人增益
ext_robot_hurt_t               robot_hurt;                //伤害状态
ext_shoot_data_t               shoot_data;                //实时射击信息
ext_rfid_status_t              rfid_status;               //机器人 RFID 状态
ext_bullet_remaining_t         bullet_remaining;          //子弹剩余发射数
ext_dart_remaining_time_t      dart_remaining_time;       //飞镖发射口倒计时

uint8_t RefereeRecvBuf[REFEREE_RECV_BUF_SIZE]; /* 裁判系统接收原始数据 */
uint8_t RefereeSendBuf[128];                   /* 发送到裁判系统的数据 */

/**
 * @brief 解码裁判系统数据
 *
 * @param RefereeRecvBuf 裁判系统接收原始数据
 * @return true or false,是否成功读取到一帧数据
 */
bool Referee_Decode(uint8_t *RefereeRecvBuf) {
    uint16_t cmd_id;
    uint16_t frame_length;  //一帧数据长度
    uint16_t data_length;
    if (RefereeRecvBuf == NULL) {
        return false;
    }
    if (RefereeRecvBuf[REFEREE_OFFSET_SOF] == REFEREE_FRAME_HEADER)  //判断数据帧起始字节是否为固定值0xA5
    {
        //数据帧头CRC8校验通过
        if (Verify_CRC8_Check_Sum(RefereeRecvBuf, REFEREE_LEN_HEADER) == true) {
            // rt_kprintf("verify crc8 success.\n");
            /* 数据长度 */
            data_length =
                ((RefereeRecvBuf[REFEREE_OFFSET_DATA_LENGTH + 1] << 8) | (RefereeRecvBuf[REFEREE_OFFSET_DATA_LENGTH]));

            /* 计算一帧整包数据长度 */
            frame_length =
                REFEREE_LEN_HEADER + REFEREE_LEN_CMDID + REFEREE_LEN_TAIL + data_length;  // frame_header.length;
            // rt_kprintf("data_length:%d, frame_length:%d\n", data_length, frame_length);
            /* 整包校验通过 */
            if (Verify_CRC16_Check_Sum(RefereeRecvBuf, frame_length) == true) {
                // rt_kprintf("verify crc16 success.\n");
                cmd_id = ((RefereeRecvBuf[REFEREE_LEN_HEADER + 1] << 8) | (RefereeRecvBuf[REFEREE_LEN_HEADER]));
                // rt_kprintf("cmd id:0x%0X\n");
                switch (cmd_id) {
                    case ID_GAME_STATUS:  //比赛状态
                        rt_memcpy(&game_status, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_GAME_RESULT:  //比赛结果
                        rt_memcpy(&game_result, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_GAME_ROBOT_HP:  //机器人血量数据
                        rt_memcpy(&game_robot_HP, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_GAME_FIELD_EVENT:  //场地事件数据
                        rt_memcpy(&game_field_event, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_SUPPLY_PROJECTILE_ACTION:  //补给站动作标识
                        rt_memcpy(&supply_projectile_action, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_REFEREE_WARNING:  //裁判警告信息
                        rt_memcpy(&judge_warning, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_DART_REMAIN_TIME:  //飞镖发射口倒计时
                        rt_memcpy(&dart_remaining_time, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_GAME_ROBOT_STATUS:  //比赛机器人状态
                        rt_memcpy(&game_robot_status, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_POWER_HEAT_DATA:  //实时功率热量数据
                        rt_memcpy(&power_heat_data, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        chassis_power = power_heat_data.chassis_power;
                        cooling_heat  = power_heat_data.shooter_id1_17mm_cooling_heat;
                        break;
                    case ID_GAME_ROBOT_POS:  //机器人位置
                        rt_memcpy(&game_robot_pos, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_BUFF:  //机器人增益
                        rt_memcpy(&robot_buff, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_ROBOT_HURT:  //伤害状态
                        rt_memcpy(&robot_hurt, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_SHOOT_DATA:  //实时射击信息
                        rt_memcpy(&shoot_data, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_REMAIN_BULLET:  //子弹剩余发射数
                        rt_memcpy(&bullet_remaining, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    case ID_RFID_STATUS:  //机器人 RFID 状态
                        rt_memcpy(&rfid_status, &RefereeRecvBuf[REFEREE_OFFSET_DATA], data_length);
                        break;
                    default:
                        break;
                }
            } else {
            }
            /* 下一帧存在数据，递归处理 */
            if (*(RefereeRecvBuf + frame_length + REFEREE_OFFSET_SOF) == REFEREE_FRAME_HEADER) {
                Referee_Decode(RefereeRecvBuf + frame_length);
            }
            return true;

        } else {
            rt_kprintf("verify CRC8 failed\n");
        }
    }
    return false;
}

/**
 * @brief 将数据段配置帧头，CRC8，帧尾CRC16校验成发送到裁判系统的RefereeSendBuf
 *
 * @param cmd_id 数据段ID
 * @param data   要发送数据段，不含数据头和帧头
 * @param length 要发送数据长度，即内容数据段长度，不含数据头和帧头
 * @return int 整包数据长度
 */
int Referee_ConfigFrameData(uint16_t cmd_id, void *data, int length) {
    referee_send_data_t referee_send_data;
    uint16_t            data_length =
        length + sizeof(ext_student_interactive_header_data_t); /* 包含数据头的长度，写入到数据帧帧头data_length字段 */

    rt_memset(&referee_send_data, 0, sizeof(referee_send_data_t));

    /* 配置帧头 */
    RefereeSendBuf[REFEREE_OFFSET_SOF]             = REFEREE_FRAME_HEADER;
    RefereeSendBuf[REFEREE_OFFSET_DATA_LENGTH + 1] = (data_length & 0xFF00) >> 8;
    RefereeSendBuf[REFEREE_OFFSET_DATA_LENGTH]     = data_length & 0x00FF;
    RefereeSendBuf[REFEREE_OFFSET_SEQ]             = 0;

    /* 帧头数据追加CRC8校验 */
    Append_CRC8_Check_Sum(RefereeSendBuf, REFEREE_LEN_HEADER);

    /* 裁判系统命令码，数据交互固定值0x0301 */
    referee_send_data.cmd_id = 0x0301;

    /* 配置发送者ID，接收者ID */
    referee_send_data.data_header.data_cmd_id = cmd_id;
    referee_send_data.data_header.sender_ID   = game_robot_status.robot_id;
    if (game_robot_status.robot_id > 100) { /* 自己为蓝方 */
        referee_send_data.data_header.receiver_ID =
            0x0167 + (game_robot_status.robot_id - 103); /* 步兵操作手客户端（蓝） */
    } else {                                             /* 自己为红方 */
        referee_send_data.data_header.receiver_ID =
            0x0103 + (game_robot_status.robot_id - 3); /* 步兵操作手客户端（红） */
    }

    /* 先复制数据到发送数据结构体 */
    rt_memcpy(referee_send_data.data, data, length);

    /* 再将结构体复制到RefereeSendBuf */
    rt_memcpy(RefereeSendBuf + REFEREE_LEN_HEADER, &referee_send_data.cmd_id,
              data_length + REFEREE_LEN_CMDID); /* length+6+2：除帧头frame_header后的数据长度 */

    /* CRC16 整包校验 */
    Append_CRC16_Check_Sum(RefereeSendBuf, data_length + REFEREE_LEN_CMDID + REFEREE_LEN_HEADER +
                                               REFEREE_LEN_TAIL); /* length+6+2+5+2：整包数据长度（包括帧尾CRC16） */

    return data_length + REFEREE_LEN_CMDID + REFEREE_LEN_HEADER + REFEREE_LEN_TAIL;
}

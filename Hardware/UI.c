#include "UI.h"

#include "referee.h"
#include "stdlib.h"
#include "usart.h"

/* 图形对象 */
ext_client_custom_graphic_delete_t graphic_delete;    /* 删除所有图形数据 */
ext_client_custom_graphic_double_t graphic_target;    /* 两个图形数据，准心 */
ext_client_custom_character_t      graphic_character; /* 单个图形数据，画字符 */

extern uint8_t RefereeSendBuf[128]; /* 发送到裁判系统的数据 */

/**
 * @brief 准心图形对象操作函数，第一图层，图形索引：GRAPIC_TARGET_X, GRAPIC_TARGET_Y，坐标在target结构体中调用
 *
 * @param operation_tpye 操作类型：1新增图形，2修改图形
 * @param xy x,y坐标
 * @return void
 */
void Grapic_Target(int operation_tpye, int x, int y) {
    graphic_target.grapic_data_struct[0].graphic_name[0] = GRAPIC_TARGET_X;
    graphic_target.grapic_data_struct[0].operate_tpye    = operation_tpye; /* 1新增图形 */
    graphic_target.grapic_data_struct[0].graphic_tpye    = 0;              /* 0直线 */
    graphic_target.grapic_data_struct[0].layer           = 1;              /* 第1图层 */
    graphic_target.grapic_data_struct[0].color           = 2;              /* 2：绿色 */
    graphic_target.grapic_data_struct[0].width           = 2;              /* 线宽 */
    graphic_target.grapic_data_struct[0].start_x         = x - 10;
    graphic_target.grapic_data_struct[0].end_x           = x + 10;
    graphic_target.grapic_data_struct[0].start_y         = y;
    graphic_target.grapic_data_struct[0].end_y           = y;

    graphic_target.grapic_data_struct[1].graphic_name[0] = GRAPIC_TARGET_Y;
    graphic_target.grapic_data_struct[1].operate_tpye    = operation_tpye; /* 1新增图形 */
    graphic_target.grapic_data_struct[1].graphic_tpye    = 0;              /* 0直线 */
    graphic_target.grapic_data_struct[1].layer           = 1;              /* 第1图层 */
    graphic_target.grapic_data_struct[1].color           = 2;              /* 2：绿色 */
    graphic_target.grapic_data_struct[1].width           = 2;              /* 线宽 */
    graphic_target.grapic_data_struct[1].start_x         = x;
    graphic_target.grapic_data_struct[1].end_x           = x;
    graphic_target.grapic_data_struct[1].start_y         = y - 10;
    graphic_target.grapic_data_struct[1].end_y           = y + 10;
}

/**
 * @brief 字符图形对象操作函数，第二图层，图形索引：GRAPIC_CHARACTER_1
 *
 * @param operation_tpye 操作类型：1新增图形，2修改图形
 * @param char_data      字符数据
 * @param char_length    字符数据长度
 * @param xy             放置的x，y坐标
 * @param name           图形索引名称
 * @return void
 */
void Grapic_Character(int operation_tpye, char *char_data, int char_length, int x, int y, int name) {
    graphic_character.grapic_data_struct.graphic_name[0] = name;
    graphic_character.grapic_data_struct.operate_tpye    = operation_tpye;
    graphic_character.grapic_data_struct.graphic_tpye    = 7; /* 7字符 */
    graphic_character.grapic_data_struct.layer           = 2;
    graphic_character.grapic_data_struct.color           = 1;  /* 1黄 */
    graphic_character.grapic_data_struct.width           = 2;  /* 线宽 */
    graphic_character.grapic_data_struct.start_angle     = 20; /* 字体大小,字体大小与线宽比例10:1 */
    graphic_character.grapic_data_struct.end_angle       = char_length; /* 字符长度 */
    graphic_character.grapic_data_struct.start_x         = x;
    graphic_character.grapic_data_struct.start_y         = y;

    rt_memset(graphic_character.data, 0, 30);
    rt_strncpy(graphic_character.data, char_data, char_length);
}

/**
 * @brief 删除所有图形
 *
 */
void UI_DeleteAllGrapic() {
    int length;

    graphic_delete.operate_tpye = 2;
    length                      = Referee_ConfigFrameData(0x0100, &graphic_delete, 2);

    HAL_UART_Transmit(&REFEREE_UART, RefereeSendBuf, length, 0xFFFF);
}

/**
 * @brief 新增或修改准心坐标
 *
 * @param operation_tpye 操作类型：新增或修改，可选参数：AddGrapic, ModifyGrapic
 * @param xy 准心的xy坐标
 */
void UI_Target(int operation_tpye, int x, int y) {
    int length;

    Grapic_Target(operation_tpye, x, y);
    length = Referee_ConfigFrameData(0x0102, &graphic_target, 30);

    HAL_UART_Transmit(&REFEREE_UART, RefereeSendBuf, length, 0xFFFF);
}

/**
 * @brief 新增或修改超级电容电压字符
 *
 * @param operation_tpye 操作类型：新增或修改，可选参数：AddGrapic, ModifyGrapic
 * @param supercap_voltage 超级电容电压值
 */
void UI_CapVoltage(int operation_tpye, float supercap_voltage) {
    int  length;
    char str[20];

    rt_sprintf(str, "CapVolt:%4dJ", (int)(0.5F * supercap_voltage * supercap_voltage * 5));
    Grapic_Character(operation_tpye, str, sizeof(str), 570, 50, GRAPIC_CHARACTER_SUPERCAP);
    length = Referee_ConfigFrameData(0x0110, &graphic_character, 15 + 30);

    HAL_UART_Transmit(&REFEREE_UART, RefereeSendBuf, length, 0xFFFF);
}

/**
 * @brief 新增或修改底盘是否为小陀螺模式字符
 *
 * @param operation_tpye 操作类型：新增或修改，可选参数：AddGrapic, ModifyGrapic
 * @param is_chassis_spin 是否为小陀螺模式
 */
void UI_ChassisMode(int operation_tpye, uint8_t is_chassis_spin) {
    int  length;
    char str[10];

    if (is_chassis_spin) {
        rt_sprintf(str, "Spin:Yes");
    } else {
        rt_sprintf(str, "Spin:No");
    }
    Grapic_Character(operation_tpye, str, sizeof(str), 70, 850, GRAPIC_CHARACTER_CHASSIS);
    length = Referee_ConfigFrameData(0x0110, &graphic_character, 15 + 30);

    HAL_UART_Transmit(&REFEREE_UART, RefereeSendBuf, length, 0xFFFF);
}

/**
 * @brief 新增或修改卡弹提示字符
 *
 * @param operation_tpye 操作类型：新增或修改，可选参数：AddGrapic, ModifyGrapic
 * @param is_stuck 是否卡弹
 */
void UI_Stuck(int operation_tpye, uint8_t is_stuck) {
    int  length;
    char str[10];

    /* 卡弹字符提示 */
    if (is_stuck) {
        rt_sprintf(str, "Stuck:Yes");
    } else {
        rt_sprintf(str, "Stuck:No");
    }
    Grapic_Character(operation_tpye, str, sizeof(str), 70, 800, GRAPIC_CHARACTER_STUCK);
    length = Referee_ConfigFrameData(0x0110, &graphic_character, 15 + 30);

    HAL_UART_Transmit(&REFEREE_UART, RefereeSendBuf, length, 0xFFFF);
}

void modify_grapic(int argc, char **argv) {
    if (rt_strcmp(argv[1], "set") == 0) {
        UI_Target(ModifyGrapic, atoi(argv[2]), atoi(argv[3]));
    }
}

MSH_CMD_EXPORT(modify_grapic, modify grapic);

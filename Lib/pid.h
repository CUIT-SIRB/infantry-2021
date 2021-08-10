/**
  ****************************(C) COPYRIGHT 2016 DJI****************************
  * @file       pid.c/h
  * @brief      pidʵ�ֺ�����������ʼ����PID���㺯����
  * @note
  * @history
  *  Version    Date            Author          Modification
  *  V1.0.0     Dec-26-2018     RM              1. ���
  *
  @verbatim
  ==============================================================================

  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2016 DJI****************************
  */
#ifndef PID_H
#define PID_H
#include "main.h"

typedef float fp32;

enum PID_MODE { PID_POSITION = 0, PID_DELTA };

typedef struct pid_type {
    uint8_t mode;
    // PID ������
    fp32 Kp;
    fp32 Ki;
    fp32 Kd;

    fp32 max_out;   //������
    fp32 max_iout;  //���������

    fp32 set;
    fp32 fdb;

    fp32 out;
    fp32 Pout;
    fp32 Iout;
    fp32 Dout;
    fp32 Dbuf[3];   //΢���� 0���� 1��һ�� 2���ϴ�
    fp32 error[3];  //����� 0���� 1��һ�� 2���ϴ�

} pid_type_t;
extern void pid_init(pid_type_t *pid, uint8_t mode, const fp32 PID[3], fp32 max_out, fp32 max_iout);
extern fp32 pid_calc(pid_type_t *pid, fp32 ref, fp32 set);
extern void pid_clear(pid_type_t *pid);
#endif

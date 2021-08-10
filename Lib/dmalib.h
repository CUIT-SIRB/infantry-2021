#ifndef __DMALIB_H
#define __DMALIB_H

#include "main.h"
#include "usart.h"

HAL_StatusTypeDef DMALIB_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef DMALIB_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
void              DMALIB_UART_IdleHandler(UART_HandleTypeDef *huart);
void              UART_IdleRxCallback(UART_HandleTypeDef *huart);

#endif

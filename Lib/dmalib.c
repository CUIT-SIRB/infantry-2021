#include "dmalib.h"

#include "dr16.h"
#include "gimbal.h"
#include "referee.h"

/**
 * @brief  为串口开启没有中断的DMA传输，为了减少中断次数为其他中断空出资源。代替HAL库的函数(此处在main函数中调用)
 *
 * @param  hdma 指向DMA_HandleTypeDef结构体的指针，这个结构体包含了DMA流的配置信息.
 * @retval HAL status
 */
HAL_StatusTypeDef DMALIB_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size) {
    uint32_t *tmp;
    uint32_t  tmp1 = 0;

    tmp1 = huart->gState;
    if ((tmp1 == HAL_UART_STATE_READY) || (tmp1 == HAL_UART_STATE_BUSY_TX)) {
        if ((pData == NULL) || (Size == 0)) {
            return HAL_ERROR;
        }

        /* Process Locked */
        __HAL_LOCK(huart);

        /* Set Reception type to Standard reception */
        huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;

        huart->pRxBuffPtr = pData;
        huart->RxXferSize = Size;

        huart->ErrorCode = HAL_UART_ERROR_NONE;
        huart->RxState   = HAL_UART_STATE_BUSY_RX;

        /* Enable the DMA Stream */
        tmp = (uint32_t *)&pData;
        HAL_DMA_Start(huart->hdmarx, (uint32_t)&huart->Instance->DR, *(uint32_t *)tmp, Size);

        /* Enable the DMA transfer for the receiver request by setting the DMAR
        bit in the UART CR3 register */
        huart->Instance->CR3 |= USART_CR3_DMAR;

        /* Process Unlocked */
        __HAL_UNLOCK(huart);

        return HAL_OK;
    } else {
        return HAL_BUSY;
    }
}

/**
 * @brief  通过 DMA 接收数据，数据长度有最大限度，但是在最大限度之内可以接受任意长度的数据
 *
 * @param  huart: 指向 UART_HandleTypeDef 结构体的指针，该指针包含了UART的配置信息
 * @param  pData: 指向接受数据缓冲区的指针
 * @param  Size: 可接收数据的最大长度
 * @retval HAL status
 */
HAL_StatusTypeDef DMALIB_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size) {
    __HAL_UART_CLEAR_IDLEFLAG(huart);             //! 清除 IDLE标志位
    __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);    // 开启串口空闲中断
    DMALIB_UART_Receive_DMA(huart, pData, Size);  // 利用DMA接受数据

    return HAL_OK;
}

/**
 * @brief 串口空闲中断回调函数
 *
 * @param huart 指向 UART_HandleTypeDef 的指针
 */
void DMALIB_UART_IdleHandler(UART_HandleTypeDef *huart) {
    uint32_t DMA_FLAGS;  //根据串口的不同来选择清除不同的DMA标志位

    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE)) {
        /*清除IDLE标志位*/
        __HAL_UART_CLEAR_IDLEFLAG(huart);

        DMA_FLAGS = __HAL_DMA_GET_TC_FLAG_INDEX(huart->hdmarx);

        UART_IdleRxCallback(huart);

        /* 重启 DMA */
        __HAL_DMA_DISABLE(huart->hdmarx);
        __HAL_DMA_CLEAR_FLAG(huart->hdmarx, DMA_FLAGS);
        __HAL_DMA_SET_COUNTER(huart->hdmarx, huart->RxXferSize);
        __HAL_DMA_ENABLE(huart->hdmarx);
    } else if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE)) {
        /* 清除 ORE 标志位 */
        __HAL_UART_CLEAR_OREFLAG(huart);

        /* 重启 DMA */
        __HAL_DMA_DISABLE(huart->hdmarx);
        __HAL_DMA_CLEAR_FLAG(huart->hdmarx, DMA_FLAGS);
        __HAL_DMA_SET_COUNTER(huart->hdmarx, huart->RxXferSize);
        __HAL_DMA_ENABLE(huart->hdmarx);
    }
}

/**
 * @brief 回调函数
 *
 * @param huart 指向 UART_HandleTypeDef 的指针
 */
void UART_IdleRxCallback(UART_HandleTypeDef *huart) {
    rt_interrupt_enter();

    if (huart == &DBUS_USART) {
        if (huart->RxXferSize - huart->hdmarx->Instance->NDTR == 18) {
            /* 遥控器解码 */
            dr16_decode();
            /* 发送遥控器在线事件 */
            rt_event_send(dr16_event, DR16_EVENT_ONLINE);
        }
    } else if (huart == &MINIPC_UART) {
        /* 发送 MINI PC 数据接收事件 */
        rt_event_send(gimbal_event, GIMBAL_EVENT_MINIPC);
    } else if (huart == &REFEREE_UART) {
        /* 裁判系统数据解码 */
        Referee_Decode(RefereeRecvBuf);
    }

    rt_interrupt_leave();
}

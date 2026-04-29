#include "uart3.h"

#include <stddef.h>
#include <stdint.h>

#include "ch32v30x.h"
#include "ring_buffer.h"

static USART_TypeDef* uart3_dev = NULL;

static uart3_rx_cb_t rx_callback = NULL;
static uint16_t rx_watermark = 0;

static ring_buffer_t rx_rb;
static uint8_t rx_storage[256];
static ring_buffer_t tx_rb;
static uint8_t tx_storage[256];

void uart3_init(uint32_t baudrate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    uart3_dev = USART3;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(uart3_dev, &USART_InitStructure);
    USART_Cmd(uart3_dev, ENABLE);

    ring_buffer_init(&rx_rb, rx_storage, sizeof(rx_storage), 1);
    ring_buffer_init(&tx_rb, tx_storage, sizeof(tx_storage), 1);
    rx_callback = NULL;
    rx_watermark = 0;

    // NVIC and RX interrupt enable
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(uart3_dev, USART_IT_RXNE, ENABLE);
    USART_ITConfig(uart3_dev, USART_IT_TXE, DISABLE);
}

void uart3_deinit(void)
{
    if ( uart3_dev != NULL )
    {
        USART_ITConfig(uart3_dev, USART_IT_RXNE, DISABLE);
        USART_ITConfig(uart3_dev, USART_IT_TXE, DISABLE);
        USART_Cmd(uart3_dev, DISABLE);
        uart3_dev = NULL;
        rx_callback = NULL;
        ring_buffer_clear(&rx_rb);
        ring_buffer_clear(&tx_rb);
    }
}

void uart3_set_baudrate(uint32_t baudrate)
{
    if ( uart3_dev == NULL || baudrate == 0 )
    {
        return;
    }

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    NVIC_DisableIRQ(USART3_IRQn);
    USART_Cmd(uart3_dev, DISABLE);
    USART_Init(uart3_dev, &USART_InitStructure);
    USART_Cmd(uart3_dev, ENABLE);
    USART_ITConfig(uart3_dev, USART_IT_RXNE, ENABLE);
    NVIC_EnableIRQ(USART3_IRQn);
}

void uart3_write(const char* buf, int size)
{
    if ( (uart3_dev == NULL) || (buf == NULL) || (size <= 0) )
    {
        return;
    }

    ring_buffer_push_n(&tx_rb, buf, (uint32_t)size, true); // overwrite oldest on overflow
    USART_ITConfig(uart3_dev, USART_IT_TXE, ENABLE);
}

int uart3_read(char* buf, int size)
{
    if ( (uart3_dev == NULL) || (buf == NULL) || (size <= 0) )
    {
        return 0;
    }

    return (int)ring_buffer_pop_n(&rx_rb, buf, (uint32_t)size);
}

int uart3_available(void)
{
    return (int)ring_buffer_count(&rx_rb);
}

void uart3_set_rx_callback(uart3_rx_cb_t cb, uint16_t watermark)
{
    rx_callback = cb;
    rx_watermark = watermark;
}

void USART3_IRQHandler(void) __attribute__((interrupt));
void USART3_IRQHandler(void)
{
    if ( USART_GetITStatus(uart3_dev, USART_IT_RXNE) != RESET )
    {
        uint8_t ch = (uint8_t)USART_ReceiveData(uart3_dev);
        ring_buffer_push(&rx_rb, &ch, true); // overwrite oldest on overflow

        if ( rx_callback )
        {
            uint16_t pending = (uint16_t)ring_buffer_count(&rx_rb);
            if ( (rx_watermark == 0) || (pending >= rx_watermark) )
            {
                rx_callback(pending);
            }
        }
    }

    if ( USART_GetITStatus(uart3_dev, USART_IT_ORE_RX) != RESET )
    {
        (void)USART_ReceiveData(uart3_dev); // clear ORE
    }

    if ( USART_GetITStatus(uart3_dev, USART_IT_TXE) != RESET )
    {
        uint8_t ch;
        if ( ring_buffer_pop(&tx_rb, &ch) )
        {
            USART_SendData(uart3_dev, ch);
        }
        else
        {
            USART_ITConfig(uart3_dev, USART_IT_TXE, DISABLE); // no more data to send
        }
    }
}

#include "uart.h"

#include <stddef.h>
#include <stdint.h>

#include "stm32f4xx_hal.h"
#include "ring_buffer.h"

/* USART1 on PA9 (TX) / PA10 (RX), AF7. APB2 (HCLK) clocks USART1 directly so
 * the BRR resolution is twice that of USART3 on F411 (HCLK/2). */

static UART_HandleTypeDef huart;
static bool uart_initialized = false;

static uart_rx_cb_t rx_callback = NULL;
static uint16_t rx_watermark = 0;

static ring_buffer_t rx_rb;
static uint8_t rx_storage[256];
static ring_buffer_t tx_rb;
static uint8_t tx_storage[256];

void uart_init(uint32_t baudrate)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef cfg = {0};
    cfg.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    cfg.Mode = GPIO_MODE_AF_PP;
    cfg.Pull = GPIO_PULLUP;
    cfg.Speed = GPIO_SPEED_FREQ_HIGH;
    cfg.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &cfg);

    huart.Instance = USART1;
    huart.Init.BaudRate = baudrate;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    if ( HAL_UART_Init(&huart) != HAL_OK )
    {
        return;
    }

    ring_buffer_init(&rx_rb, rx_storage, sizeof(rx_storage), 1);
    ring_buffer_init(&tx_rb, tx_storage, sizeof(tx_storage), 1);
    rx_callback = NULL;
    rx_watermark = 0;
    uart_initialized = true;

    HAL_NVIC_SetPriority(USART1_IRQn, 3, 3);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    __HAL_UART_ENABLE_IT(&huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(&huart, UART_IT_TXE);
}

void uart_deinit(void)
{
    if ( !uart_initialized )
    {
        return;
    }

    __HAL_UART_DISABLE_IT(&huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(&huart, UART_IT_TXE);
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    HAL_UART_DeInit(&huart);

    rx_callback = NULL;
    ring_buffer_clear(&rx_rb);
    ring_buffer_clear(&tx_rb);
    uart_initialized = false;
}

void uart_set_baudrate(uint32_t baudrate)
{
    if ( !uart_initialized || baudrate == 0U )
    {
        return;
    }

    HAL_NVIC_DisableIRQ(USART1_IRQn);
    huart.Init.BaudRate = baudrate;
    HAL_UART_Init(&huart);
    __HAL_UART_ENABLE_IT(&huart, UART_IT_RXNE);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void uart_write(const char* buf, int size)
{
    if ( !uart_initialized || buf == NULL || size <= 0 )
    {
        return;
    }

    ring_buffer_push_n(&tx_rb, buf, (uint32_t)size, true); // overwrite oldest on overflow
    __HAL_UART_ENABLE_IT(&huart, UART_IT_TXE);
}

int uart_read(char* buf, int size)
{
    if ( !uart_initialized || buf == NULL || size <= 0 )
    {
        return 0;
    }

    return (int)ring_buffer_pop_n(&rx_rb, buf, (uint32_t)size);
}

int uart_available(void)
{
    return (int)ring_buffer_count(&rx_rb);
}

void uart_set_rx_callback(uart_rx_cb_t cb, uint16_t watermark)
{
    rx_callback = cb;
    rx_watermark = watermark;
}

void USART1_IRQHandler(void)
{
    uint32_t sr = USART1->SR;

    if ( sr & USART_SR_RXNE )
    {
        uint8_t ch = (uint8_t)(USART1->DR & 0xFFu);
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

    if ( sr & USART_SR_ORE )
    {
        (void)USART1->DR; // SR-then-DR read sequence clears ORE
    }

    if ( sr & USART_SR_TXE )
    {
        uint8_t ch;
        if ( ring_buffer_pop(&tx_rb, &ch) )
        {
            USART1->DR = ch;
        }
        else
        {
            __HAL_UART_DISABLE_IT(&huart, UART_IT_TXE);
        }
    }
}

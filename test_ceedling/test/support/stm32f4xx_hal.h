/*
 * stm32f4xx_hal.h  (STUB solo para tests en PC - NO usar en el firmware)
 *
 * Reemplaza al header real de ST para poder compilar en el host los módulos
 * que lo incluyen (uart_rs485.h, board_port.h, delay.h).
 * HAL_Delay() no espera: acumula los ms pedidos en g_hal_delay_ms, así los
 * tests pueden verificar CUÁNTO tiempo bloqueante se pidió (ver RF-04).
 * La variable g_hal_delay_ms se define en cada archivo de test.
 */
#ifndef STM32F4XX_HAL_STUB_H_
#define STM32F4XX_HAL_STUB_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define GPIO_PIN_5   ((uint16_t)0x0020)
#define GPIO_PIN_13  ((uint16_t)0x2000)
#define GPIOA        ((void*)0)
#define GPIOC        ((void*)0)

typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET } GPIO_PinState;

extern uint32_t g_hal_delay_ms;
static inline void HAL_Delay(uint32_t ms) { g_hal_delay_ms += ms; }

#endif /* STM32F4XX_HAL_STUB_H_ */

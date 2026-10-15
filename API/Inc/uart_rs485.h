/*
 * uart_rs485.h
 *
 *  Objeto "UartRS485": encapsula el puerto serie RS485 (USART1) usado
 *  para la comunicación Modbus con el sensor de suelo.
 *
 *  Patrón de objeto en C (visto en la materia):
 *   - Tipo opaco (puntero a struct incompleto)  -> encapsulamiento
 *   - Los métodos reciben "self" como 1er parámetro -> equivalente a "this"
 *   - Los atributos se definen en el .c (privados al módulo)
 *
 *  Created on: 19 abr 2026 (adaptado a objeto)
 *      Author: vicks
 */

#ifndef API_INC_UART_RS485_H_
#define API_INC_UART_RS485_H_

#include "types.h"
#include "stm32f4xx_hal.h"

typedef enum{
	RS485_OK=0,
	RS485_ERROR,
	RS485_TIMEOUT,
	RS485_BUSY
} rs485status_t;

/* Tipo opaco: quien usa el objeto solo ve el puntero, nunca los atributos */
typedef struct UartRS485 UartRS485;

/* "Constructor": reserva una instancia (pool estático, sin heap) */
UartRS485* UartRS485_create(void);

/* Métodos: todos reciben self como primer parámetro */
bool_t        UartRS485_init(UartRS485* self);
rs485status_t UartRS485_transmit(UartRS485* self, const uint8_t* data, uint16_t len);
rs485status_t UartRS485_sendString(UartRS485* self, uint8_t* pstring);
rs485status_t UartRS485_receiveByte(UartRS485* self, uint8_t* byte);
void          UartRS485_receiveStringSize(UartRS485* self, uint8_t* pstring, uint16_t size);
bool_t        UartRS485_dataAvailable(UartRS485* self);

#endif /* API_INC_UART_RS485_H_ */

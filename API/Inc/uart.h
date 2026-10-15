/*
 * API_uart.h
 *
 *  Created on: 2 abr 2026
 *      Author: vicks
 *  About: Capa de abstracción de hardware para nucleo stm32f446RE
 *  		para comunicacion UART/COM
 */

#ifndef API_INC_UART_H_
#define API_INC_UART_H_


#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include "board_port.h"

/*Defines que traigo del main*/
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA



typedef bool bool_t;
bool_t uartInit(); //inicializa la UART
void uartSendString(uint8_t * pstring); //envia el  String
void uartSendStringSize(uint8_t * pstring, uint16_t size); //envía el tamaño del string
void uartReceiveStringSize(uint8_t * pstring, uint16_t size); // Recibe el tamaño del string
bool_t uartDataAvailable(void);

#endif /* API_INC_UART_H_ */

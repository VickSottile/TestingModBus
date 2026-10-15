/*
 * uart_rs485.c
 *
 *  Implementación del objeto UartRS485.
 *  El struct completo (los "atributos privados") solo se conoce acá:
 *  desde afuera del módulo solo existe el puntero opaco UartRS485*.
 *
 *  Created on: 19 abr 2026
 *      Author: vicks
 */

#include "uart_rs485.h"
#include "board_port.h"
#include <string.h>

#define TIMEOUT_UART2 50
#define TIMEOUT_BYTE 20
#define RS485_DE_GPIO_Port GPIOA
#define RS485_DE_Pin GPIO_PIN_8

/* Cantidad máxima de instancias soportadas (pool estático) */
#define MAX_UART485_INSTANCES 1

/* Definición "privada" del objeto: solo visible dentro de este archivo */
struct UartRS485 {
	UART_HandleTypeDef huart;
	bool_t rxReady; /* refleja si hay datos disponibles para leer */
};

static UartRS485 instancePool[MAX_UART485_INSTANCES];
static uint8_t instanceCount = 0;

/* Habilitación de pines DE/RE del transceiver físico (propio de la placa,
 * no forma parte del "estado" del objeto sino del hardware fijo) */
static void enableTransmit(void){
	HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
}
static void enableReceive(void){
	HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
}

UartRS485* UartRS485_create(void){
	if(instanceCount >= MAX_UART485_INSTANCES){
		return NULL; /* pool agotado */
	}
	UartRS485* self = &instancePool[instanceCount++];
	self->rxReady = false;
	return self;
}

bool_t UartRS485_init(UartRS485* self){
	if(self == NULL) return false;

	/* 1. Habilitar clocks */
	__HAL_RCC_USART1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/* 2. Configurar pines PA9 (TX) y PA10 (RX) como AF7 */
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin       = GPIO_PIN_9 | GPIO_PIN_10;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* 3. Configurar pin PA8 como DE/RE */
	GPIO_InitStruct.Pin       = RS485_DE_Pin;
	GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = 0;
	HAL_GPIO_Init(RS485_DE_GPIO_Port, &GPIO_InitStruct);

	/* 4. Iniciar en modo recepción */
	enableReceive();

	/* 5. Configurar UART */
	self->huart.Instance          = USART1;
	self->huart.Init.BaudRate     = 4800;
	self->huart.Init.WordLength   = UART_WORDLENGTH_8B;
	self->huart.Init.StopBits     = UART_STOPBITS_1;
	self->huart.Init.Parity       = UART_PARITY_NONE;
	self->huart.Init.Mode         = UART_MODE_TX_RX;
	self->huart.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	self->huart.Init.OverSampling = UART_OVERSAMPLING_16;

	if(HAL_UART_Init(&self->huart) != HAL_OK){
		Error_Handler();
		return false;
	}
	return true;
}

rs485status_t UartRS485_transmit(UartRS485* self, const uint8_t* data, uint16_t len){
	if(self == NULL || data == NULL || len == 0){
		return RS485_ERROR;
	}
	enableTransmit();
	HAL_StatusTypeDef transmitido = HAL_UART_Transmit(&self->huart, (uint8_t*)data, len, 500);

	uint32_t tickstart = HAL_GetTick();
	while(__HAL_UART_GET_FLAG(&self->huart, UART_FLAG_TC) == RESET){
		if((HAL_GetTick() - tickstart) > 100){
			enableReceive();
			return RS485_ERROR; // sale en lugar de colgar
		}
	}
	enableReceive();
	return (transmitido == HAL_OK) ? RS485_OK : RS485_ERROR;
}

rs485status_t UartRS485_sendString(UartRS485* self, uint8_t* pstring){
	if(self == NULL || pstring == NULL){
		return RS485_ERROR;
	}
	return UartRS485_transmit(self, pstring, (uint16_t)strlen((char*)pstring));
}

rs485status_t UartRS485_receiveByte(UartRS485* self, uint8_t* byte){
	if(self == NULL || byte == NULL){
		return RS485_ERROR;
	}
	HAL_StatusTypeDef recibido = HAL_UART_Receive(&self->huart, byte, 1, TIMEOUT_BYTE);
	return (recibido == HAL_OK) ? RS485_OK : RS485_TIMEOUT;
}

void UartRS485_receiveStringSize(UartRS485* self, uint8_t* pstring, uint16_t size){
	if(self == NULL) return;
	self->rxReady = false;
	if(pstring != NULL){
		HAL_StatusTypeDef transmitido = HAL_UART_Receive(&self->huart, pstring, size, TIMEOUT_UART2);
		if(transmitido == HAL_ERROR){
			Error_Handler();
		}
		self->rxReady = (transmitido == HAL_OK);
	}
}

bool_t UartRS485_dataAvailable(UartRS485* self){
	if(self == NULL) return false;
	return self->rxReady;
}

/*
 * soilModBus.c
 *
 *  Implementación del objeto SoilSensor.
 *
 *  ABOUT: MEF interna para leer el Sensor de Suelo con MODBUS
 *      Estados: INIT → IDLE → REQUESTING → WAITING → PARSING → DONE
 *                                           → ERROR
 *
 *   Trama de solicitud (Modbus FC03, 3 registros desde 0x0000):
 *   [ADDR] [FC]   [REG_H] [REG_L] [QTY_H] [QTY_L] [CRC_L] [CRC_H]
 *   [0x01] [0x03] [0x00]  [0x00]  [0x00]  [0x03]  [0x05]  [0xCB]
 *
 *   Validación de la respuesta: longitud, CRC-16/MODBUS, addr/función/byte count.
 *
 *   Trama de respuesta esperada (11 bytes):
 *   [ADDR][FC][BYTE_CNT][HUM_H][HUM_L][TEMP_H][TEMP_L][EC_H][EC_L][CRC_L][CRC_H]
 *
 *  Created on: 19 abr 2026 (adaptado a objeto)
 *      Author: vicks
 */

#include "soilModBus.h"
#include "delay.h"

/*Constantes*/
#define SLAVE_ADDRESS 0x01
#define RESPONSE_LEN 11 //Longitud de la respuesta esperada
#define TIMEOUT 500
#define REQUEST_CMD 0x03
#define REG_H 0x00
#define REG_L 0x00
#define QTY_H 0x00
#define QTY_L 0x03
#define CRC_L 0x05
#define CRC_H 0xCB
#define MAX_SOILSENSOR_INSTANCES 1

/* CRC-16/MODBUS: polinomio 0xA001 (0x8005 reflejado), valor inicial 0xFFFF.
 * En la trama se transmite primero el byte bajo (CRC_L) y luego el alto (CRC_H). */
#define CRC16_INIT 0xFFFF
#define CRC16_POLY 0xA001
#define CRC_LEN 2 /* bytes de CRC al final de la respuesta */

/*Trama para leer 3 registros desde 0x0000 el sensor con ModBus*/
static const uint8_t REQUEST_FRAME[] = {
    SLAVE_ADDRESS, REQUEST_CMD, REG_H, REG_L, QTY_H, QTY_L, CRC_L, CRC_H
};

/* Definición "privada" del objeto (atributos) */
struct SoilSensor {
	UartRS485* uart;              /* asociación con el objeto puerto serie */
	MEFSensorState_t state;
	uint8_t rxBuffer[RESPONSE_LEN];
	uint8_t rxIdx;
	soilsensor_data_t data;
	bool_t dataValid;
	delay_t rxTimeout;
};

static SoilSensor instancePool[MAX_SOILSENSOR_INSTANCES];
static uint8_t instanceCount = 0;

/* Función privada (static): calcula el CRC-16/MODBUS de len bytes */
static uint16_t modbusCrc16(const uint8_t* data, uint16_t len){
	uint16_t crc = CRC16_INIT;
	for(uint16_t i = 0; i < len; i++){
		crc ^= data[i];
		for(uint8_t bit = 0; bit < 8; bit++){
			if(crc & 0x0001){
				crc = (uint16_t)((crc >> 1) ^ CRC16_POLY);
			} else {
				crc >>= 1;
			}
		}
	}
	return crc;
}

/* Método privado (static): valida y parsea rxBuffer, y llena self->data.
 * Orden de validación: longitud -> CRC -> encabezado (addr/función/byte count).
 * Si algo falla NO se modifica self->data (se descarta la trama completa). */
static bool_t parseResponse(SoilSensor* self){
	if (self->rxIdx < RESPONSE_LEN) {
		return false;
	}

	/* Integridad: CRC calculado sobre todo menos los 2 bytes de CRC recibidos */
	uint16_t crcCalc = modbusCrc16(self->rxBuffer, RESPONSE_LEN - CRC_LEN);
	uint16_t crcRecv = ((uint16_t)self->rxBuffer[RESPONSE_LEN - 1] << 8) |
	                    self->rxBuffer[RESPONSE_LEN - 2];
	if (crcCalc != crcRecv) {
		return false;
	}

	if(self->rxBuffer[0] != SLAVE_ADDRESS || self->rxBuffer[1] != 0x03 || self->rxBuffer[2] != 0x06){
		return false;
	}

	uint16_t humRaw  = ((uint16_t)self->rxBuffer[3] << 8) | self->rxBuffer[4];
	uint16_t tempRaw = ((uint16_t)self->rxBuffer[5] << 8) | self->rxBuffer[6];
	uint16_t ecRaw   = ((uint16_t)self->rxBuffer[7] << 8) | self->rxBuffer[8];

	self->data.humedad     = humRaw / 10.0f;
	self->data.temperatura = tempRaw / 10.0f;
	self->data.ec          = (float)ecRaw;

	return true;
}

SoilSensor* SoilSensor_create(UartRS485* uart){
	if(instanceCount >= MAX_SOILSENSOR_INSTANCES || uart == NULL){
		return NULL;
	}
	SoilSensor* self = &instancePool[instanceCount++];
	self->uart = uart; /* inyección de dependencia: el objeto puerto serie */
	self->state = IDLE485;
	self->rxIdx = 0;
	self->dataValid = false;
	return self;
}

void SoilSensor_init(SoilSensor* self){
	if(self == NULL) return;
	UartRS485_init(self->uart);
	HAL_Delay(500);
	self->dataValid = false;
	self->rxIdx = 0;
	self->state = IDLE485;
}

bool_t SoilSensor_requestData(SoilSensor* self){
	if(self == NULL) return false;
	if(self->state == IDLE485 || self->state == DONE485){
		self->state = REQUESTING485;
		return true;
	}
	return false;
}

/* Cuando está en DONE485 lo regresa a IDLE485 para que la próxima
 * llamada a SoilSensor_requestData() funcione. */
MEFSensorState_t SoilSensor_getState(SoilSensor* self){
	if(self == NULL) return ERROR485;
	MEFSensorState_t state = self->state;
	if(state == DONE485){
		self->state = IDLE485;
	}
	return state;
}

const soilsensor_data_t* SoilSensor_getData(const SoilSensor* self){
	if(self == NULL || !self->dataValid){
		return NULL;
	}
	return &self->data;
}

void SoilSensor_update(SoilSensor* self){
	if(self == NULL) return;

	switch(self->state){
	case INIT485:
		SoilSensor_init(self);
		self->state = IDLE485;
		break;

	case IDLE485:
		break;

	case REQUESTING485:
		if(UartRS485_transmit(self->uart, REQUEST_FRAME, sizeof(REQUEST_FRAME)) == RS485_OK){
			delayInit(&self->rxTimeout, TIMEOUT);
			self->rxIdx = 0;
			self->state = WAITING485;
		} else {
			self->state = ERROR485;
		}
		break;

	case WAITING485:
	{
		uint8_t byte;
		while(self->rxIdx < RESPONSE_LEN){
			rs485status_t status = UartRS485_receiveByte(self->uart, &byte);
			if(status == RS485_OK){
				self->rxBuffer[self->rxIdx++] = byte;
			} else {
				break;
			}
		}
		if(self->rxIdx >= RESPONSE_LEN){
			self->state = PARSING485;
		} else if(delayRead(&self->rxTimeout)){
			self->state = ERROR485;
		}
		break;
	}

	case PARSING485:
		if(parseResponse(self)){
			self->dataValid = true;
			self->state = DONE485;
		} else {
			self->state = ERROR485;
		}
		break;

	case DONE485:
		break;

	case ERROR485:
		self->state = INIT485;
		break;

	default:
		break;
	}
}

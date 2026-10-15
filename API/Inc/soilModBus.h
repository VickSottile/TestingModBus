/*
 * soilModBus.h
 *
 *  Objeto "SoilSensor": encapsula el sensor de suelo 3 en 1 (Modbus RTU).
 *  Se relaciona por composición/asociación con un objeto UartRS485, que
 *  es el puerto serie que usa para comunicarse (inyectado en el
 *  "constructor" -> bajo acoplamiento, se podría reemplazar el puerto
 *  sin tocar la lógica del sensor).
 *
 *  Created on: 19 abr 2026 (adaptado a objeto)
 *      Author: vicks
 */

#ifndef API_INC_SOILMODBUS_H_
#define API_INC_SOILMODBUS_H_

#include "types.h"
#include "uart_rs485.h"

typedef enum{
    INIT485=0,
	IDLE485,
	REQUESTING485,
	WAITING485,
	PARSING485,
	DONE485,
	ERROR485
} MEFSensorState_t;

typedef struct {
	float temperatura; /*°C*/
	float humedad;      /* % */
	float ec;            /* uS/cm */
} soilsensor_data_t;

/* Tipo opaco del objeto */
typedef struct SoilSensor SoilSensor;

/* "Constructor": recibe el puerto serie (objeto UartRS485) que va a usar */
SoilSensor* SoilSensor_create(UartRS485* uart);

void SoilSensor_init(SoilSensor* self);
void SoilSensor_update(SoilSensor* self);              // actualiza la MEF interna, llamar cada tick
bool_t SoilSensor_requestData(SoilSensor* self);        // pide el disparo de una medición
MEFSensorState_t SoilSensor_getState(SoilSensor* self); // getter del estado de la MEF
const soilsensor_data_t* SoilSensor_getData(const SoilSensor* self); // NULL si no hay datos válidos

#endif /* API_INC_SOILMODBUS_H_ */

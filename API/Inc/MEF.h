/*
 * MEF.h
 *
 *  MEF principal. Orquesta (por composición/inyección) los objetos
 *  SoilSensor y Button, que son creados en main.c (App) y pasados
 *  a MEF_init().
 *
 *  Created on: 11 abr 2026 (adaptado a objeto)
 *      Author: vicks
 */

#ifndef API_INC_MEF_H_
#define API_INC_MEF_H_

#include "types.h"
#include "soilModBus.h"
#include "debounce.h"
#include "lcd.h"

typedef enum{
    INIT=0,
	IDLE,
	REQ_SENSING,
	SENSING,
	SHOW,
	ERROR1
} MEFState_t;

/* Inicializa la MEF, recibiendo los objetos que va a orquestar */
void MEF_init(SoilSensor* sensor, Button* button);

/* Actualización de la máquina de estados (llamar cada tick del super-loop) */
void MEFUpdate(void);

#endif /* API_INC_MEF_H_ */

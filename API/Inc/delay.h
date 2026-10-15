/*
 * API_delay.h
 *
 *  Created on: 22 mar 2026
 *      Author: Victoria Sottile
 */
/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */



/* USER CODE END Includes */

#ifndef API_INC_API_DELAY_H_
#define API_INC_API_DELAY_H_

#include "types.h"
#include "board_port.h"

typedef uint32_t tick_t; // Qué biblioteca se debe incluir para que esto compile? stdint.h
//se define en un typedef para mayor portabilidad y se usa tick_t como "tics del sistema"
typedef bool bool_t;	  //Qué biblioteca se debe incluir para que esto compile? stsbool.h
typedef uint8_t ent_t;
typedef struct{
   tick_t startTime; //cuándo comenzó el delay
   tick_t duration; //cuánto debe durar
   bool_t running;  //Está corriendo
} delay_t;
void delayInit(delay_t * delay, tick_t duration ); // carga el valor de duración del retardo en la estructura
bool_t delayRead(delay_t * delay ); //verifica el estado del flag running
void delayWrite(delay_t * delay, tick_t duration ); //permite cambiar el tiempo de duración de un delay existente
bool_t delayIsRunning(delay_t * delay);
#endif /* API_INC_API_DELAY_H_ */

/*
 * board_port.h
 *
 *  Created on: 9 abr 2026
 *      Author: vicks
 */

#ifndef API_INC_BOARD_PORT_H_
#define API_INC_BOARD_PORT_H_

#include "types.h"


#include "stm32f4xx_hal.h"
/* Private defines*/

/*Borrarlos del main.h*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC

#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA




//ESTADOS DEL LED
typedef enum {
	LED_OFF = 0, LED_ON, LED_TOGGLE
} led_state_t;


//TIEMPOS TICKS DEL CONTROLADOR
typedef uint32_t ticker_t;



/* Inicialización de las GPIO a utilizar*/
void MX_GPIO_Init(void);



/* FUNCIONES BASICAS PARA EL MANEJO DE LEDs */
void ledOn(void);
void ledOff(void);
void ledToggle(uint32_t tiempo);
void ledUpdate();
void ledInit(void);


/*Button State*/
GPIO_PinState GPIOState(void);

/*Time Functions*/
ticker_t getTick(void);


/*Comunicaciones I2C*/



void Error_Handler(void);

#endif /* API_INC_BOARD_PORT_H_ */

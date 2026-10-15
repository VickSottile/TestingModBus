/*
 * I2C_port.h
 *
 *  Created on: 13 abr 2026
 *      Author: vicks
 */

#ifndef API_INC_LCD_I2C_PORT_H_
#define API_INC_LCD_I2C_PORT_H_

#include "types.h"
#include "stm32f4xx_hal.h"
#include "board_port.h"


void I2Cinit();
void I2CWriteByte(uint8_t data); // Prototipo de función que escribe un Byte en el puerto I2C
uint8_t I2CReadByte(); //prototipo de función para recibir un dato por I2C
void I2CDelay(uint8_t timeD); //función de delay


#endif /* API_INC_LCD_I2C_PORT_H_ */

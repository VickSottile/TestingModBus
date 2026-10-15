/*
 * API_lcd.h
 *
 *  Created on: 10 abr 2026
 *      Author: vicks
 */

#ifndef API_INC_LCD_H_
#define API_INC_LCD_H_

#include <lcd_I2C_port.h>

void LCD_init(void);
void LCD_clear(void);

void LCD_writeString(char * str);
void LCD_delay(uint8_t timeD);
void LCD_setCursor(uint8_t row, uint8_t col);
void LCD_writeStringRC(uint8_t row, uint8_t col, const char *str);
void LCD_writeStringF1(const char *str);
void LCD_writeStringF2(const char *str);
//void LCD_send_nibble(uint8_t nibble, uint8_t rs);
//void LCD_send_byte(uint8_t byte, uint8_t rs);







#endif /* API_INC_LCD_H_ */

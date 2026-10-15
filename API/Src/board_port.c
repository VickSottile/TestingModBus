/*
 * board_port.c
 *
 *  Created on: 9 abr 2026
 *      Author: vicks
 */

#define LED_DELAY 500 //delay del parpadeo
#include <delay.h>
#include "board_port.h"
#include "stm32f4xx_hal.h"




static led_state_t led=LED_OFF;

//Variables internas del módulo
static delay_t ledDelay;

// Inicializacion de GPIO
void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

ticker_t getTick(void){
	ticker_t time= HAL_GetTick();
	return(time);
}


void ledOn(void){			// debe encender el LED

	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
	led = LED_ON;
	return;
}
void ledOff(void){		// debe apagar el LED
	led = LED_OFF;
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
	return;
}

void ledUpdate(){
    switch(led){

        case LED_TOGGLE:
            if(delayRead(&ledDelay)){
                HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
            }
            break;

        case LED_OFF:
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
            break;

        case LED_ON:
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
            break;
    }
		}



void ledToggle(uint32_t tiempo){
	led = LED_TOGGLE;
	delayInit(&ledDelay, tiempo);
}


GPIO_PinState GPIOState(){
	return(HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin));
}



void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}


/*Comunicaciones I2C*/
/*
static void I2C_Write_Byte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout){
	//HAL_I2C_Transmit();
	HAL_I2C_Master_Transmit(*hi2c,DevAddress,*pData, Size, tTimeout);
}

static I2C_Read_Byte(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout){

	//HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout);
	HAL_I2C_Master_Receive(*hi2c, DevAddress,*pData,  Size, Timeout);
}*/

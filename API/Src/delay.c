/*
 * API_delay.c
 *
 *  Created on: 22 mar 2026
 *      Author: Victoria Sottile
 */
/* Includes ------------------------------------------------------------------*/
#include <delay.h>

void delayInit( delay_t * delay, tick_t duration )// carga el valor de duración del retardo en la estructura
  {
	if(delay==NULL)
	{
		return;
	}
    delay->duration=duration;
    delay->running=false;
  }
  bool_t delayRead( delay_t * delay ) //verifica el estado del flag running
  //si chequeo que el puntero no sea null que debo poner? porque tanto true como false tienen significado en la logica. se podría llamar a un Errorhandler o reiniciar el programa
  {
	  if(delay==NULL) //Si puntero es NULL  retornar false
	 	{
	 		return false;
	 	}
    if(delay->running==false)
    {
      delay->startTime=getTick();
      delay->running=true;
      return false;
    }else
    {
     tick_t currentTime=getTick();
     if((currentTime-delay->startTime)>=delay->duration)
     {
        delay->running=false;
        return true;
     }else{
        return false;
     }
    }
  }
  void delayWrite( delay_t * delay, tick_t duration )
  {
	  if(delay==NULL) //Si puntero es NULL
	  	 	{
	  	 		return;
	  	 	}else{
    delay->duration=duration;
	  	 	}
  }

  bool_t delayIsRunning(delay_t * delay)
  {
    if(delay==NULL || delay->running<0) //Si puntero es NULL o running no es valido retornar false
	{
		return false; 
	}
    return (delay->running);


  }



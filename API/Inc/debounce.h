/*
 * debounce.h
 *
 *  Objeto "Button": encapsula el antirrebote de un pulsador físico.
 *
 *  Patrón de objeto en C (visto en la materia):
 *   - Tipo opaco Button (puntero a struct incompleto)
 *   - Métodos con "self" como primer parámetro
 *   - Estado interno (atributos) privado al .c
 *
 *  Created on: 30 mar 2026 (adaptado a objeto)
 *      Author: vicks
 */

#ifndef API_INC_API_DEBOUNCE_H_
#define API_INC_API_DEBOUNCE_H_

#include "types.h"

typedef struct Button Button;

/* "Constructor": reserva una instancia (pool estático, sin heap) */
Button* Button_create(void);

void   Button_init(Button* self);   // carga el estado inicial
void   Button_update(Button* self); // lee la entrada y actualiza la MEF interna

/* Lee y resetea el flag de "tecla presionada" (equivalente a readKey()
 * de la versión anterior, ahora como método del objeto) */
bool_t Button_isPressed(Button* self);

#endif /* API_INC_API_DEBOUNCE_H_ */

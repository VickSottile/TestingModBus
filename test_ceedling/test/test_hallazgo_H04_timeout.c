/*
 * HALLAZGO H-04 (falla a proposito mientras el defecto siga abierto)
 * RNF-02: timeout Modbus <= 500 ms POR MUESTRA, contado desde la transmision.
 * El conteo del modulo delay arranca en el primer sondeo de WAITING485 (delayRead
 * inicia el reloj en su primera llamada), no al transmitir: el timeout real es
 * 500 ms + la demora hasta el primer sondeo.
 * Modulos reales: SoilSensor + delay.  Mocks: UartRS485 y getTick (reloj simulado).
 */
#include "unity.h"
#include "soilModBus.h"
#include "delay.h"
#include "mock_uart_rs485.h"
#include "mock_board_port.h"

uint32_t g_hal_delay_ms;
#define FAKE_UART ((UartRS485*)0x1000)
static uint32_t simNow;
static ticker_t tickCb(int n){ (void)n; return simNow; }
static SoilSensor* sensor = NULL;

void setUp(void){
    simNow = 0; getTick_Stub(tickCb);
    UartRS485_receiveByte_IgnoreAndReturn(RS485_TIMEOUT);
    if(sensor == NULL) sensor = SoilSensor_create(FAKE_UART);
    UartRS485_init_ExpectAndReturn(FAKE_UART, true);
    SoilSensor_init(sensor);
}
void tearDown(void){}

void test_H04_RNF02_timeoutMustBeCountedFromTransmission(void){
    UartRS485_transmit_ExpectAnyArgsAndReturn(RS485_OK);
    SoilSensor_requestData(sensor);
    simNow = 0;   SoilSensor_update(sensor);        /* transmite en t = 0 ms */
    simNow = 20;  SoilSensor_update(sensor);        /* 1er sondeo (tras bloquear en la lectura) */
    simNow = 500; SoilSensor_update(sensor);        /* 500 ms despues de transmitir */
    TEST_ASSERT_EQUAL_MESSAGE(ERROR485, SoilSensor_getState(sensor),
        "RNF-02: a los 500 ms de transmitir todavia no hay timeout (el conteo arranco en el 1er sondeo)");
}

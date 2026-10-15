/*
 * test_soilModBus_integracion.c  -  1 PRUEBA DE INTEGRACION
 *
 * Integra dos modulos REALES: SoilSensor (soilModBus.c) + delay (delay.c).
 * Solo se mockean los bordes con el hardware: el puerto RS485 y getTick()
 * (board_port), con un reloj simulado que controla el test.
 *
 * X1: sin respuesta del sensor, el timeout de 500 ms medido por el modulo delay
 *     REAL hace pasar a ERROR485 exactamente a los 500 ms de iniciado el conteo.
 */
#include "unity.h"
#include "soilModBus.h"
#include "delay.h"
#include "mock_uart_rs485.h"
#include "mock_board_port.h"

uint32_t g_hal_delay_ms;
#define FAKE_UART ((UartRS485*)0x1000)
#define TIMEOUT_MS 500

static uint32_t simNow;                                   /* reloj simulado (ms) */
static ticker_t tickCb(int n){ (void)n; return simNow; }
static SoilSensor* sensor = NULL;

void setUp(void){
    simNow = 0;
    getTick_Stub(tickCb);
    UartRS485_receiveByte_IgnoreAndReturn(RS485_TIMEOUT);        /* el sensor no contesta */
    if(sensor == NULL) sensor = SoilSensor_create(FAKE_UART);
    UartRS485_init_ExpectAndReturn(FAKE_UART, true);
    SoilSensor_init(sensor);
}
void tearDown(void){}

void test_X1_timeout500ms_isMeasuredByRealDelayModule(void){
    UartRS485_transmit_ExpectAnyArgsAndReturn(RS485_OK);
    TEST_ASSERT_TRUE(SoilSensor_requestData(sensor));
    SoilSensor_update(sensor);                                    /* REQUESTING -> WAITING (delayInit real) */
    TEST_ASSERT_EQUAL(WAITING485, SoilSensor_getState(sensor));

    simNow = 20;  SoilSensor_update(sensor);                      /* 1er sondeo: delay REAL arranca el conteo */
    TEST_ASSERT_EQUAL(WAITING485, SoilSensor_getState(sensor));

    simNow = 20 + TIMEOUT_MS - 1;  SoilSensor_update(sensor);     /* 499 ms: todavia espera */
    TEST_ASSERT_EQUAL(WAITING485, SoilSensor_getState(sensor));

    simNow = 20 + TIMEOUT_MS;      SoilSensor_update(sensor);     /* 500 ms: vence el timeout */
    TEST_ASSERT_EQUAL(ERROR485, SoilSensor_getState(sensor));
}

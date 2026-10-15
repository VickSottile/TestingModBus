/*
 * HALLAZGO H-02 (falla a proposito mientras el defecto siga abierto)
 * RF-04: ante error Modbus, reiniciar el subsistema de sensado en <= 100 ms.
 * SoilSensor_init() ejecuta HAL_Delay(500) y ERROR485 pasa por INIT485.
 */
#include "unity.h"
#include "soilModBus.h"
#include "mock_uart_rs485.h"
#include "mock_delay.h"

uint32_t g_hal_delay_ms;
#define FAKE_UART ((UartRS485*)0x1000)
#define RF04_MAX_RECOVERY_MS 100
static SoilSensor* sensor = NULL;

void setUp(void){
    if(sensor == NULL) sensor = SoilSensor_create(FAKE_UART);
    UartRS485_init_ExpectAndReturn(FAKE_UART, true);
    SoilSensor_init(sensor);
    g_hal_delay_ms = 0;
}
void tearDown(void){}

void test_H02_RF04_errorRecovery_blockingTimeMustBeAtMost100ms(void){
    UartRS485_transmit_ExpectAnyArgsAndReturn(RS485_ERROR);
    SoilSensor_requestData(sensor);
    SoilSensor_update(sensor);                  /* REQUESTING -> ERROR485 */
    SoilSensor_update(sensor);                  /* ERROR485 -> INIT485    */
    UartRS485_init_ExpectAndReturn(FAKE_UART, true);
    g_hal_delay_ms = 0;
    SoilSensor_update(sensor);                  /* INIT485 -> IDLE485 (reinicio) */
    TEST_ASSERT_LESS_OR_EQUAL_UINT32_MESSAGE(RF04_MAX_RECOVERY_MS, g_hal_delay_ms,
        "RF-04: el reinicio del sensor bloquea mas de 100 ms (HAL_Delay en SoilSensor_init)");
}

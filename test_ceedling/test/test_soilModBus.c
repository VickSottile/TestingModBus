/*
 * test_soilModBus.c  -  10 PRUEBAS UNITARIAS del objeto SoilSensor (Modbus RTU)
 *
 * Codigo bajo prueba (REAL): API/Src/soilModBus.c
 * Dependencias MOCKEADAS:    UartRS485 y delay.  HAL de ST: stub de PC.
 *
 * Tecnica: State Transition Testing sobre la sub-MEF de 7 estados.
 *
 *  Transiciones validas:
 *   t1  IDLE       --requestData-->               REQUESTING
 *   t2  DONE       --requestData-->               REQUESTING
 *   t3  REQUESTING --tick [tx OK]-->              WAITING
 *   t4  REQUESTING --tick [tx falla]-->           ERROR
 *   t5  WAITING    --tick [11 bytes]-->           PARSING
 *   t6  WAITING    --tick [<11 bytes, sin timeout]--> WAITING
 *   t7  WAITING    --tick [timeout]-->            ERROR
 *   t8  PARSING    --tick [trama valida]-->       DONE
 *   t9  PARSING    --tick [trama invalida]-->     ERROR
 *   t10 ERROR      --tick-->                      INIT
 *   t11 INIT       --tick-->                      IDLE
 *   t12 DONE       --getState-->                  IDLE
 *
 *  Paso 3 (scripts legales, terminan en IDLE): L1..L6 -> cubren t1..t12
 *      L1: t1 t4 t10 t11        L2: t1 t3 t5 t8 t12     L3: t1 t3 t5 t8 t2 t3 t5 t8 t12
 *      L4: t1 t3 t5 t9 t10 t11  L5: t1 t3 t6 t5 t8 t12  L6: t1 t3 t7 t10 t11
 *  Paso 4 (ilegales): I1 recorre las 13 combinaciones estado/evento ilegales
 *  Paso 5 (guardas):  L5 (borde 10/11 bytes) y G1 (validez compuesta, ECT)
 *  Extras: N1 (evento global init), C1 (construccion y robustez ante nulos)
 */
#include <string.h>
#include "unity.h"
#include "soilModBus.h"
#include "mock_uart_rs485.h"
#include "mock_delay.h"

uint32_t g_hal_delay_ms;                       /* lo usa el stub de HAL_Delay */
#define FAKE_UART ((UartRS485*)0x1000)         /* el mock nunca lo desreferencia */
#define FRAME_LEN 11

/* Trama valida: hum=41.0 %, temp=25.3 C, EC=1200 uS/cm (CRC calculado con oraculo) */
static const uint8_t FRAME_OK[FRAME_LEN]       = {0x01,0x03,0x06,0x01,0x9A,0x00,0xFD,0x04,0xB0,0xEA,0x3C};
static const uint8_t FRAME_BAD_ADDR[FRAME_LEN] = {0x02,0x03,0x06,0x01,0x9A,0x00,0xFD,0x04,0xB0,0xFE,0xCC};
static const uint8_t FRAME_BAD_FUNC[FRAME_LEN] = {0x01,0x04,0x06,0x01,0x9A,0x00,0xFD,0x04,0xB0,0xAB,0xDA};
static const uint8_t FRAME_BAD_BCNT[FRAME_LEN] = {0x01,0x03,0x04,0x01,0x9A,0x00,0xFD,0x04,0xB0,0xC9,0xFC};

static SoilSensor* sensor = NULL;

/* ---- Oraculo CRC-16/MODBUS independiente del codigo bajo prueba --------- */
static uint16_t oracleCrc16(const uint8_t* d, uint16_t n){
    uint16_t crc = 0xFFFF;
    for(uint16_t i = 0; i < n; i++){
        crc ^= d[i];
        for(int b = 0; b < 8; b++) crc = (crc & 1) ? (uint16_t)((crc >> 1) ^ 0xA001) : (uint16_t)(crc >> 1);
    }
    return crc;
}
static void buildFrame(uint8_t* f, uint16_t hum, uint16_t temp, uint16_t ec){
    f[0]=0x01; f[1]=0x03; f[2]=0x06;
    f[3]=hum>>8; f[4]=hum&0xFF; f[5]=temp>>8; f[6]=temp&0xFF; f[7]=ec>>8; f[8]=ec&0xFF;
    uint16_t crc = oracleCrc16(f, 9); f[9]=crc&0xFF; f[10]=crc>>8;
}

/* ---- Bytes que "llegan" por el puerto RS485 (callback de CMock) ---------- */
static const uint8_t* rxStream; static uint8_t rxLen, rxPos;
static rs485status_t rxCallback(UartRS485* self, uint8_t* byte, int n){
    (void)self; (void)n;
    if(rxPos < rxLen){ *byte = rxStream[rxPos++]; return RS485_OK; }
    return RS485_TIMEOUT;
}
static void feedRx(const uint8_t* data, uint8_t len){
    rxStream = data; rxLen = len; rxPos = 0;
    UartRS485_receiveByte_Stub(rxCallback);
}

/* ---- Pasos de los scripts (evento + resultado esperado) ------------------ */
static void assertState(MEFSensorState_t s){ TEST_ASSERT_EQUAL(s, SoilSensor_getState(sensor)); }
static void stepRequest(void){ TEST_ASSERT_TRUE(SoilSensor_requestData(sensor)); }
static void stepTx(rs485status_t r){
    UartRS485_transmit_ExpectAnyArgsAndReturn(r);
    if(r == RS485_OK) delayInit_ExpectAnyArgs();
    SoilSensor_update(sensor);
}
static void stepTxExactRequest(void){                       /* verifica los 8 bytes del pedido */
    uint8_t exp[8] = {0x01,0x03,0x00,0x00,0x00,0x03,0x00,0x00};
    uint16_t crc = oracleCrc16(exp, 6); exp[6] = crc & 0xFF; exp[7] = crc >> 8;   /* 05 CB */
    UartRS485_transmit_ExpectWithArrayAndReturn(FAKE_UART, 1, exp, 8, 8, RS485_OK);
    delayInit_ExpectAnyArgs();
    SoilSensor_update(sensor);
}
static void stepRx(const uint8_t* d, uint8_t n, int timeoutExpired){
    feedRx(d, n);
    if(timeoutExpired >= 0) delayRead_ExpectAnyArgsAndReturn(timeoutExpired);   /* -1: la trama se completa y no se consulta el timeout */
    SoilSensor_update(sensor);
}
static void stepTick(void){ SoilSensor_update(sensor); }
static void stepInitTick(void){ UartRS485_init_ExpectAndReturn(FAKE_UART, true); SoilSensor_update(sensor); }
static void resetSensor(void){ UartRS485_init_ExpectAndReturn(FAKE_UART, true); SoilSensor_init(sensor); g_hal_delay_ms = 0; }

/* ---- Posicionadores: llevan la sub-MEF a un estado ----------------------- */
static void toIdle(void){}
static void toRequesting(void){ stepRequest(); assertState(REQUESTING485); }
static void toWaiting(void){ toRequesting(); stepTx(RS485_OK); assertState(WAITING485); }
static void toParsing(const uint8_t* f){ toWaiting(); stepRx(f, FRAME_LEN, -1); assertState(PARSING485); }
static void toParsingOk(void){ toParsing(FRAME_OK); }
static void toDone(void){ toParsing(FRAME_OK); stepTick(); }         /* no lee estado: getState lo consumiria */
static void toError(void){ toRequesting(); stepTx(RS485_ERROR); assertState(ERROR485); }
static void toInit(void){ toError(); stepTick(); assertState(INIT485); }

void setUp(void){
    rxStream = NULL; rxLen = 0; rxPos = 0;
    if(sensor == NULL) sensor = SoilSensor_create(FAKE_UART);   /* pool estatico de 1 instancia */
    resetSensor();
}
void tearDown(void){}

/* ======================= PASO 3: SCRIPTS LEGALES ========================== */

void test_L1_txFailure_thenRecovery(void){                  /* t1 t4 t10 t11 */
    stepRequest();           assertState(REQUESTING485);
    stepTx(RS485_ERROR);     assertState(ERROR485);
    stepTick();              assertState(INIT485);
    stepInitTick();          assertState(IDLE485);
    TEST_ASSERT_NULL(SoilSensor_getData(sensor));
}

void test_L2_happyPath_sendsRequestFrameAndDecodesData(void){  /* t1 t3 t5 t8 t12 */
    stepRequest();           assertState(REQUESTING485);
    stepTxExactRequest();    assertState(WAITING485);        /* pedido 01 03 00 00 00 03 05 CB */
    stepRx(FRAME_OK, FRAME_LEN, -1);  assertState(PARSING485);
    stepTick();
    const soilsensor_data_t* d = SoilSensor_getData(sensor);
    TEST_ASSERT_NOT_NULL(d);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 41.0f, d->humedad);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 25.3f, d->temperatura);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 1200.0f, d->ec);
    assertState(DONE485);                                    /* t12: DONE se informa una vez */
    assertState(IDLE485);
}

void test_L3_requestFromDone_replacesData(void){            /* t1 t3 t5 t8 t2 t3 t5 t8 t12 */
    uint8_t second[FRAME_LEN]; buildFrame(second, 555, 180, 800);   /* 55.5 %, 18.0 C, 800 uS/cm */
    toDone();
    stepRequest();           assertState(REQUESTING485);     /* t2 */
    stepTx(RS485_OK);        assertState(WAITING485);
    stepRx(second, FRAME_LEN, -1);    assertState(PARSING485);
    stepTick();
    const soilsensor_data_t* d = SoilSensor_getData(sensor);
    TEST_ASSERT_NOT_NULL(d);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 55.5f, d->humedad);
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 18.0f, d->temperatura);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 800.0f, d->ec);
    assertState(DONE485);    assertState(IDLE485);
}

void test_L4_corruptedFrame_isDiscarded_thenRecovery(void){ /* t1 t3 t5 t9 t10 t11 (H-01) */
    uint8_t bad[FRAME_LEN]; memcpy(bad, FRAME_OK, FRAME_LEN); bad[3] ^= 0x10;   /* dato alterado */
    stepRequest();           assertState(REQUESTING485);
    stepTx(RS485_OK);        assertState(WAITING485);
    stepRx(bad, FRAME_LEN, -1);       assertState(PARSING485);
    stepTick();              assertState(ERROR485);
    TEST_ASSERT_NULL(SoilSensor_getData(sensor));            /* no se publican datos */
    stepTick();              assertState(INIT485);
    stepInitTick();          assertState(IDLE485);
}

void test_L5_responseInParts_boundary10then11bytes(void){   /* t1 t3 t6 t5 t8 t12 (borde 10/11) */
    stepRequest();           assertState(REQUESTING485);
    stepTx(RS485_OK);        assertState(WAITING485);
    stepRx(FRAME_OK, 10, 0); assertState(WAITING485);        /* t6: 10 de 11 bytes, sigue esperando */
    stepRx(FRAME_OK + 10, 1, -1);     assertState(PARSING485);   /* t5: llega el byte 11 */
    stepTick();
    TEST_ASSERT_NOT_NULL(SoilSensor_getData(sensor));
    assertState(DONE485);    assertState(IDLE485);
}

void test_L6_noResponse_timeout_thenRecovery(void){         /* t1 t3 t7 t10 t11 */
    stepRequest();           assertState(REQUESTING485);
    stepTx(RS485_OK);        assertState(WAITING485);
    stepRx(FRAME_OK, 0, 1);  assertState(ERROR485);          /* sin bytes y timeout vencido */
    stepTick();              assertState(INIT485);
    stepInitTick();          assertState(IDLE485);
    TEST_ASSERT_NULL(SoilSensor_getData(sensor));
}

/* ====================== PASO 4: COMBINACIONES ILEGALES ===================== */
/* 13 combinaciones: el sistema no reacciona (requestData=false, estado igual).  */

typedef struct { const char* name; void (*pos)(void); MEFSensorState_t st; } illegal_t;

void test_I1_illegalEvents_haveNoEffect(void){
    static const illegal_t reqCases[] = {                     /* requestData ilegal (5) */
        {"REQUESTING",toRequesting,REQUESTING485},{"WAITING",toWaiting,WAITING485},
        {"PARSING",toParsingOk,PARSING485},{"ERROR",toError,ERROR485},{"INIT",toInit,INIT485}};
    static const illegal_t getCases[] = {                     /* getState sin efecto lateral (6) */
        {"IDLE",toIdle,IDLE485},{"REQUESTING",toRequesting,REQUESTING485},{"WAITING",toWaiting,WAITING485},
        {"PARSING",toParsingOk,PARSING485},{"ERROR",toError,ERROR485},{"INIT",toInit,INIT485}};
    for(unsigned i = 0; i < sizeof(reqCases)/sizeof(reqCases[0]); i++){
        resetSensor(); reqCases[i].pos();
        TEST_ASSERT_FALSE_MESSAGE(SoilSensor_requestData(sensor), reqCases[i].name);
        TEST_ASSERT_EQUAL_MESSAGE(reqCases[i].st, SoilSensor_getState(sensor), reqCases[i].name);
    }
    for(unsigned i = 0; i < sizeof(getCases)/sizeof(getCases[0]); i++){
        resetSensor(); getCases[i].pos();
        TEST_ASSERT_EQUAL_MESSAGE(getCases[i].st, SoilSensor_getState(sensor), getCases[i].name);
        TEST_ASSERT_EQUAL_MESSAGE(getCases[i].st, SoilSensor_getState(sensor), getCases[i].name);
    }
    resetSensor(); toIdle();  stepTick(); assertState(IDLE485);  /* tick en IDLE (1) */
    resetSensor(); toDone();  stepTick(); assertState(DONE485);  /* tick en DONE (1): el dato no se pierde */
}

/* ===================== PASO 5: GUARDA COMPUESTA (ECT) ====================== */
/* [trama valida] = direccion Y funcion Y byte count Y CRC: cada condicion se invierte sola. */

void test_G1_frameValidity_eachConditionInverted_goesToError(void){
    static uint8_t badCrc[FRAME_LEN], badData[FRAME_LEN];
    memcpy(badCrc, FRAME_OK, FRAME_LEN);  badCrc[FRAME_LEN-1] ^= 0xFF;   /* solo el CRC erroneo */
    memcpy(badData, FRAME_OK, FRAME_LEN); badData[4] ^= 0x01;            /* 1 bit del dato alterado */
    const uint8_t* frames[] = {FRAME_BAD_ADDR, FRAME_BAD_FUNC, FRAME_BAD_BCNT, badCrc, badData};
    const char* names[] = {"direccion","funcion","byte count","CRC","1 bit de dato"};
    for(unsigned i = 0; i < 5; i++){
        resetSensor(); toParsing(frames[i]); stepTick();
        TEST_ASSERT_EQUAL_MESSAGE(ERROR485, SoilSensor_getState(sensor), names[i]);
        TEST_ASSERT_NULL_MESSAGE(SoilSensor_getData(sensor), names[i]);
    }
}

/* ====================== EXTRAS: init global y construccion ================= */

void test_N1_init_fromAnyState_resetsToIdleAndClearsData(void){   /* evento global init() */
    toWaiting();
    UartRS485_init_ExpectAndReturn(FAKE_UART, true); SoilSensor_init(sensor);
    assertState(IDLE485);    TEST_ASSERT_NULL(SoilSensor_getData(sensor));
    toDone();                TEST_ASSERT_NOT_NULL(SoilSensor_getData(sensor));
    UartRS485_init_ExpectAndReturn(FAKE_UART, true); SoilSensor_init(sensor);
    assertState(IDLE485);    TEST_ASSERT_NULL(SoilSensor_getData(sensor));
}

void test_C1_create_staticPool_andNullSafety(void){            /* RNF-06 y robustez */
    TEST_ASSERT_NULL(SoilSensor_create(NULL));                 /* puerto nulo */
    TEST_ASSERT_NULL(SoilSensor_create(FAKE_UART));            /* pool de 1 instancia agotado */
    SoilSensor_init(NULL);  SoilSensor_update(NULL);
    TEST_ASSERT_FALSE(SoilSensor_requestData(NULL));
    TEST_ASSERT_EQUAL(ERROR485, SoilSensor_getState(NULL));
    TEST_ASSERT_NULL(SoilSensor_getData(NULL));
}

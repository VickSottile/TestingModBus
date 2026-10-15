# Pruebas automatizadas - módulo SoilSensor (Modbus RTU)


    cd test_ceedling
    ceedling test:test_soilModBus               # 10 pruebas unitarias (STT: L1-L6, I1, G1, N1, C1)
    ceedling test:test_soilModBus_integracion   # 1 prueba de integración (SoilSensor + delay reales)
    ceedling gcov:test_soilModBus gcov:test_soilModBus_integracion   # tests + cobertura
    ceedling test:test_hallazgo_H02_reinicio    # hallazgo H-02 (FALLA a propósito: RF-04)
    ceedling test:test_hallazgo_H04_timeout     # hallazgo H-04 (FALLA a propósito: RNF-02)
    ceedling test:all                           # 11 pasan + 2 hallazgos fallan a propósito


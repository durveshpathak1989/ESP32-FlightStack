/**
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║   Drone_IMU_FreeRTOS.ino  (v3.0)                                 ║
 * ║   MPU-9250  +  Adafruit HUZZAH32 Feather (ESP32-WROOM-32E)      ║
 * ║   FreeRTOS Multi-Task  |  Mahony AHRS  |  NVS Calibration       ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  HUZZAH32 Feather SPI wiring                                     ║
 * ║  MPU-9250   Feather label   GPIO                                 ║
 * ║  VCC      → 3V              3.3V ONLY                            ║
 * ║  GND      → GND             GND                                  ║
 * ║  SCL/SCLK → SCK             GPIO 5                               ║
 * ║  SDA/MOSI → MO              GPIO 18                              ║
 * ║  AD0/MISO → MI              GPIO 19                              ║
 * ║  NCS/CS   → 33              GPIO 33                              ║
 * ║  INT      → 27              GPIO 27  (optional)                  ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  FreeRTOS tasks                                                   ║
 * ║  taskIMURead   Core1 Pri5   1 kHz SPI read + Mahony AHRS        ║
 * ║  taskTelemetry Core0 Pri2   20 Hz Serial print                   ║
 * ║  taskSerial    Core0 Pri1   50 Hz command poll                   ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  Serial commands:  C G A M S L E P R T I X H                    ║
 * ║  Board setting:  Tools → Board → "Adafruit ESP32 Feather"        ║
 * ╚══════════════════════════════════════════════════════════════════╝
 */

#include <Arduino.h>
#include <SPI.h>
#include "MPU9250.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// ─────────────────────────────────────────────────────────────
//  HUZZAH32 Feather SPI pins
// ─────────────────────────────────────────────────────────────
#define PIN_SCK     5
#define PIN_MOSI   18
#define PIN_MISO   19
#define PIN_CS     33
#define PIN_INT    27

// ─────────────────────────────────────────────────────────────
//  FreeRTOS / timing config
// ─────────────────────────────────────────────────────────────
#define TASK_IMU_STACK      5120
#define TASK_TELEM_STACK    3072
#define TASK_SERIAL_STACK   4096
#define TASK_IMU_PRI           5
#define TASK_TELEM_PRI         2
#define TASK_SERIAL_PRI        1
#define TELEM_RATE_HZ         20
#define AHRS_CONVERGE_MS    3000UL

// Arming thresholds
#define MAX_ARMING_ROLL    5.0f
#define MAX_ARMING_PITCH   5.0f
#define MAX_ARMING_GYRO    1.0f

// ─────────────────────────────────────────────────────────────
//  Global objects
// ─────────────────────────────────────────────────────────────
MPU9250          imu(PIN_CS);        // owns SPI bus + AHRS state

MPU_SensorData   g_sensor   = {};
MPU_Attitude     g_attitude = {};
SemaphoreHandle_t xMutex   = nullptr;

TaskHandle_t hIMU    = nullptr;
TaskHandle_t hTelem  = nullptr;
TaskHandle_t hSerial = nullptr;

volatile bool g_telemetryOn = false;
uint32_t      g_ahrsStartMs = 0;

// ═════════════════════════════════════════════════════════════
//  TASK: taskIMURead — Core 1, Priority 5
// ═════════════════════════════════════════════════════════════
void taskIMURead(void* pv) {
    const TickType_t xPeriod = pdMS_TO_TICKS(1);
    TickType_t xLastWake = xTaskGetTickCount();
    uint32_t lastUs = micros();

    for (;;) {
        vTaskDelayUntil(&xLastWake, xPeriod);

        uint32_t nowUs = micros();
        float dt = (nowUs - lastUs) * 1e-6f;
        lastUs = nowUs;

        MPU_SensorData s;
        MPU_Attitude   a;

        if (imu.readScaled(s)) {
            imu.mahonyUpdate(s, dt, a);

            if (xSemaphoreTake(xMutex, 0) == pdTRUE) {
                g_sensor   = s;
                g_attitude = a;
                xSemaphoreGive(xMutex);
            }
        }
    }
}

// ═════════════════════════════════════════════════════════════
//  TASK: taskTelemetry — Core 0, Priority 2
// ═════════════════════════════════════════════════════════════
void taskTelemetry(void* pv) {
    const TickType_t xPeriod = pdMS_TO_TICKS(1000 / TELEM_RATE_HZ);
    TickType_t xLastWake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&xLastWake, xPeriod);
        if (!g_telemetryOn) continue;

        MPU_SensorData s;
        MPU_Attitude   a;
        if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
            s = g_sensor; a = g_attitude;
            xSemaphoreGive(xMutex);
        }

        Serial.printf(
            "[T=%6lu] Roll=%+7.2f Pitch=%+7.2f Yaw=%6.1f | "
            "Ax=%+6.3f Ay=%+6.3f Az=%+6.3f | "
            "Gx=%+6.2f Gy=%+6.2f Gz=%+6.2f | "
            "Mx=%+6.1f My=%+6.1f Mz=%+6.1f | T=%.1fC\n",
            s.ts_ms, a.roll, a.pitch, a.yaw,
            s.ax_g, s.ay_g, s.az_g,
            s.gx_dps, s.gy_dps, s.gz_dps,
            s.mx_uT, s.my_uT, s.mz_uT,
            s.temp_c
        );
    }
}

// ─────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────
static void printAttitude() {
    MPU_SensorData s; MPU_Attitude a;
    if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        s = g_sensor; a = g_attitude; xSemaphoreGive(xMutex);
    }
    Serial.printf("\n  Roll  = %+.2f°\n  Pitch = %+.2f°\n  Yaw   = %.2f°\n",
                  a.roll, a.pitch, a.yaw);
    Serial.printf("  Q: w=%+.4f x=%+.4f y=%+.4f z=%+.4f\n",
                  a.q0, a.q1, a.q2, a.q3);
    Serial.printf("  Accel: %+.3f %+.3f %+.3f g\n",   s.ax_g, s.ay_g, s.az_g);
    Serial.printf("  Gyro:  %+.3f %+.3f %+.3f dps\n", s.gx_dps, s.gy_dps, s.gz_dps);
    Serial.printf("  Mag:   %+.1f %+.1f %+.1f uT\n",  s.mx_uT, s.my_uT, s.mz_uT);
}

static void runPreflightCheck() {
    Serial.println(F("\n=== PRE-FLIGHT ARMING CHECK ==="));
    bool ok = true;
    MPU_SensorData s; MPU_Attitude a;
    if (xSemaphoreTake(xMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        s = g_sensor; a = g_attitude; xSemaphoreGive(xMutex);
    }
    bool hw = imu.isConnected();
    Serial.printf("  [%s] MPU-9250 connected\n", hw?"OK":"!!"); if (!hw) ok=false;
    Serial.printf("  [%s] Calibration valid\n", imu.cal.valid?"OK":"!!"); if (!imu.cal.valid) ok=false;
    bool conv = (millis()-g_ahrsStartMs) > AHRS_CONVERGE_MS;
    Serial.printf("  [%s] AHRS converged\n", conv?"OK":"!!"); if (!conv) ok=false;
    bool lvl = (fabsf(a.roll)<MAX_ARMING_ROLL && fabsf(a.pitch)<MAX_ARMING_PITCH);
    Serial.printf("  [%s] Level  roll=%.1f  pitch=%.1f  (lim ±%.0f)\n",
                  lvl?"OK":"!!", a.roll, a.pitch, MAX_ARMING_ROLL); if (!lvl) ok=false;
    float gm = sqrtf(s.gx_dps*s.gx_dps+s.gy_dps*s.gy_dps+s.gz_dps*s.gz_dps);
    bool qt = gm < MAX_ARMING_GYRO;
    Serial.printf("  [%s] Gyro quiet  %.2f dps\n", qt?"OK":"!!",gm); if (!qt) ok=false;
    Serial.printf("  [%s] Die temp %.1fC\n",
                  (s.temp_c>-10&&s.temp_c<70)?"OK":"WW", s.temp_c);
    Serial.println(ok ? "\n  READY TO ARM" : "\n  NOT SAFE TO ARM — fix issues above");
}

static void printMenu() {
    Serial.println(F("\n┌──────┬─────────────────────────────────────────┐"));
    Serial.println(F("│  C   │ Full calibration wizard (G + A + M)      │"));
    Serial.println(F("│  G   │ Gyro-only  │  A  Accel  │  M  Mag        │"));
    Serial.println(F("│  S   │ Save NVS   │  L  Load   │  E  Erase      │"));
    Serial.println(F("│  P   │ Print cal  │  R  Attitude snapshot        │"));
    Serial.println(F("│  T   │ Toggle telemetry stream                   │"));
    Serial.println(F("│  D   │ Mag diagnostic (raw reg dump)             │"));
    Serial.println(F("│  I   │ IMU status │  X  Preflight  │  H  Help   │"));
    Serial.println(F("└──────┴─────────────────────────────────────────┘"));
}

// ═════════════════════════════════════════════════════════════
//  TASK: taskSerial — Core 0, Priority 1
// ═════════════════════════════════════════════════════════════
void taskSerial(void* pv) {
    for (;;) {
        if (Serial.available()) {
            char cmd = (char)toupper(Serial.read());
            while (Serial.available()) Serial.read();

            switch (cmd) {
                case 'C':
                    vTaskSuspend(hIMU);
                    imu.calibrateGyro();
                    imu.calibrateAccel();
                    imu.calibrateMag(20000);
                    vTaskResume(hIMU);
                    Serial.println(F("[CAL] Done — send S to save."));
                    break;
                case 'G':
                    vTaskSuspend(hIMU); imu.calibrateGyro();  vTaskResume(hIMU);
                    Serial.println(F("[CAL] Done — send S to save.")); break;
                case 'A':
                    vTaskSuspend(hIMU); imu.calibrateAccel(); vTaskResume(hIMU);
                    Serial.println(F("[CAL] Done — send S to save.")); break;
                case 'D':
                    vTaskSuspend(hIMU);
                    imu.diagMag();
                    vTaskResume(hIMU);
                    break;
                case 'M':
                    vTaskSuspend(hIMU); imu.calibrateMag(20000); vTaskResume(hIMU);
                    Serial.println(F("[CAL] Done — send S to save.")); break;
                case 'S': imu.saveCalibration(); break;
                case 'L':
                    if (imu.loadCalibration()) { Serial.println(F("[NVS] Loaded.")); imu.printCalibration(); }
                    else Serial.println(F("[NVS] None found."));
                    break;
                case 'E':
                    Serial.println(F("Type Y to confirm erase:"));
                    { uint32_t t=millis();
                      while(!Serial.available()&&millis()-t<5000) delay(10);
                      if(Serial.available()&&toupper(Serial.read())=='Y') imu.eraseCalibration();
                      else Serial.println(F("Cancelled.")); }
                    break;
                case 'P': imu.printCalibration(); break;
                case 'R': printAttitude(); break;
                case 'T':
                    g_telemetryOn=!g_telemetryOn;
                    Serial.printf("[TELEM] %s\n", g_telemetryOn?"ON":"OFF"); break;
                case 'I':
                    Serial.printf("\n[IMU] WHO_AM_I=0x%02X %s | Mag:%s\n",
                                  imu.whoAmI(), imu.isConnected()?"OK":"FAIL",
                                  imu.isMagConnected()?"OK":"absent");
                    Serial.printf("[SYS] Heap:%lu  StackHWM  IMU:%u TELEM:%u SER:%u\n",
                                  esp_get_free_heap_size(),
                                  uxTaskGetStackHighWaterMark(hIMU),
                                  uxTaskGetStackHighWaterMark(hTelem),
                                  uxTaskGetStackHighWaterMark(hSerial));
                    break;
                case 'X': runPreflightCheck(); break;
                case 'H': printMenu(); break;
                default:
                    if (cmd>=' ') Serial.printf("[?] '%c' unknown — H for help\n", cmd);
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// ═════════════════════════════════════════════════════════════
//  setup()
// ═════════════════════════════════════════════════════════════
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println(F("\n[INIT] MPU-9250 FreeRTOS v3.0  HUZZAH32 Feather"));

    // SPI bus init (Feather: SCK=5, MISO=19, MOSI=18)
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
    delay(10);

    // MPU-9250 init
    Serial.print(F("[INIT] MPU-9250... "));
    if (!imu.begin()) {
        Serial.println(F("FAILED — check wiring. Halting."));
        while (true) delay(1000);
    }
    Serial.println(F("OK"));

    // Load calibration
    Serial.print(F("[INIT] NVS calibration... "));
    if (imu.loadCalibration()) { Serial.println(F("loaded.")); imu.printCalibration(); }
    else Serial.println(F("not found — run C to calibrate."));

    // FreeRTOS
    xMutex = xSemaphoreCreateMutex();
    configASSERT(xMutex);
    g_ahrsStartMs = millis();

    xTaskCreatePinnedToCore(taskIMURead,   "imu",    TASK_IMU_STACK,    nullptr, TASK_IMU_PRI,    &hIMU,    1);
    xTaskCreatePinnedToCore(taskTelemetry, "telem",  TASK_TELEM_STACK,  nullptr, TASK_TELEM_PRI,  &hTelem,  0);
    xTaskCreatePinnedToCore(taskSerial,    "serial", TASK_SERIAL_STACK, nullptr, TASK_SERIAL_PRI, &hSerial, 0);

    printMenu();
    Serial.println(F("[INIT] Running. Send T to start telemetry."));
}

void loop() { vTaskDelay(portMAX_DELAY); }

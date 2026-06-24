/**
 * Build-time debug print control.
 *
 * Set VERNOSE_ON to 0 at build time to suppress Serial output while keeping
 * runtime telemetry endpoints and Wi-Fi calibration logs active.
 */
#pragma once
#ifndef DEBUG_CONFIG_H
#define DEBUG_CONFIG_H

#include <Arduino.h>

#ifndef VERNOSE_ON
#define VERNOSE_ON 1
#endif

#if VERNOSE_ON
#define DBG_PRINT(...)   Serial.print(__VA_ARGS__)
#define DBG_PRINTLN(...) Serial.println(__VA_ARGS__)
#define DBG_PRINTF(...)  Serial.printf(__VA_ARGS__)
#else
#define DBG_PRINT(...)   ((void)0)
#define DBG_PRINTLN(...) ((void)0)
#define DBG_PRINTF(...)  ((void)0)
#endif

#endif // DEBUG_CONFIG_H

#pragma once
#include <stdio.h>

#define DEVICE_NAME "es-led-module"
#define FIRMWARE_VERSION "1.0.0"

#define LOG_LEVEL_ERR 1
#define LOG_LEVEL_INF 2
#define LOG_LEVEL_DBG 3

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DBG
#endif

void log_version(void);
void log_prefix(const char *level, const char *function, int line);
#define LOG_ERR(...)  // уровень "err", проверка порога, подпись и printf
#define LOG_INF(...)  // уровень "inf"
#define LOG_DBG(...)  // уровень "dbg"
#pragma once
#include <iostream>
#include <mutex>
#ifdef WIN32
#include <Windows.h>
#endif
#include <helper/util_string.h>
#include <logger/console_logger.h>
#define  _CRT_SECURE_NO_WARNINGS
#define LOG LOG_CMD
extern console_logger::ILoggerPtr g_logger;
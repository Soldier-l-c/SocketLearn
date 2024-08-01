#pragma once
#include <iostream>
#include <mutex>
#ifdef WIN32
#include <Windows.h>
#endif
#include <helper/util_string.h>
#include <logger/console_logger.h>
#include <thread/thread_pool.h>

#define LOG LOG_CMD
extern console_logger::ILoggerPtr g_logger;
#pragma once
#include <iostream>
#include <mutex>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <cpp-httplib/httplib.h>
#ifdef WIN32
#include <Windows.h>
#endif
#include <helper/util_string.h>
#include <logger/console_logger.h>
#include <thread/thread_pool.h>
#include <map>
#include <vector>
#define LOG LOG_CMD
extern console_logger::ILoggerPtr g_logger;
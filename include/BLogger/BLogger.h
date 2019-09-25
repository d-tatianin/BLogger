// Main BLogger header

#pragma once

/* Base BLogger class.
   Responsible for message formatting
   and main logging functions.
*/ 
#include "Loggers/BaseLogger.h"

/* Blocking version of BLogger.
   All logging happens on a
   single thread.
*/
#include "Loggers/BlockingLogger.h"

/* Async version of BLogger.
   Logs the mesages using
   a dedicated threadpool
   and a message queue.
*/
#include "Loggers/AsyncLogger.h"

typedef BLogger::BLoggerBlock BlockingLogger;
typedef BLogger::BLoggerAsync AsyncLogger;

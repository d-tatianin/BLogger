// Main BLogger header

#pragma once

/* Base BLogger class.
   Responsible for message formatting
   and main logging functions.
*/ 
#include "Loggers/BLoggerBase.h"

/* Blocking version of BLogger.
   All logging happens on a
   single thread.
*/
#include "Loggers/BLoggerBlock.h"

/* Async version of BLogger.
   Logs the mesages using
   a dedicated threadpool
   using a message queue.
   NOTE: currently in alpha stage.
   (no file logging/colored output)
*/
#include "Loggers/BLoggerAsync.h"

typedef BLogger::BLoggerBlock BlockingLogger;
typedef BLogger::BLoggerAsync AsyncLogger;

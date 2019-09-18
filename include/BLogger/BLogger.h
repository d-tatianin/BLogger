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
   NOTE: not yet available.
*/
#include "Loggers/BLoggerAsync.h"

/* The default BLogger.
   Will probably be
   switched to BLoggerAsync
   as soon as it is done.
*/
typedef BLogger::BLoggerBlock blocking_logger;

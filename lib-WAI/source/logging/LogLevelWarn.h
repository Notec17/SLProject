/*!
 * \file
 * \brief Enables all the log levels down to Warn.
 */

#ifndef WAI_LOG_WARN
#define WAI_LOG_WARN

//THIS FILE IS ONLY SUPPOSED TO BE INCLUDED IN WAIHelper.h.
//OTHERWISE THE LOGGING SEVERITY PREPROCESSOR SWITCH WILL NOT WORK CORRECTLY!
#ifndef WAI_HELPER_H
#    error LogLevelWarn.h included before WAIHelper.h
#endif

#define WAI_LOG_LEVEL 3 //! failed because of multiple LogLevel includes

#ifndef WAI_DEBUG          //make sure first decision is not overwritten
#    define WAI_DEBUG(...) // nothing
#endif

#ifndef WAI_INFO          //make sure first decision is not overwritten
#    define WAI_INFO(...) // nothing
#endif

#define WAI_DEBUG_TIMER_START(...)
#define WAI_DEBUG_TIMER_ELAPSED_MILLIS(...)
#define WAI_INFO_TIMER_START(...)
#define WAI_INFO_TIMER_ELAPSED_MILLIS(...)

#include "Logger.h"

#endif
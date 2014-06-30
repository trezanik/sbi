#pragma once

/**
 * @file        crash_handler.h
 * @author      James Warren
 * @brief       Application crash handling
 */



#include "types.h"



#if defined(_WIN32)

/**
 * Called when an exception is raised within a __try/__catch block. Uses the
 * inbuilt Windows API functions to get the parameters - calling code should
 * never need to be modified.
 *
 * @param[in] code The error code raised; acquired in __catch
 * @param[in] ep The exception pointers; acquired in __catch
 * @return Always returns EXCEPTION_EXECUTE_HANDLER so the system can process it
 */
int32_t
write_dump(
	unsigned long code,
	struct _EXCEPTION_POINTERS* ep
);


#endif	// _WIN32

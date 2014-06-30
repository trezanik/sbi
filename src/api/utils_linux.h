#pragma once

/**
 * @file	utils_linux.h
 * @author	James Warren
 * @brief	Linux-specific utility functions
 */



BEGIN_NAMESPACE(APP_NAMESPACE)



/**
 * Retrieves the current path for the executing binary, storing the result in
 * the supplied @a buffer. Comes with the trailing forward-slash.
 *
 * @param[in,out] buffer The preallocated buffer to store the result in
 * @param[in] buffer_size The size of the preallocated buffer
 * @return Returns the amount of characters written to buffer if successful
 * @return On failure, 0 is returned
 */
SBI_API
uint32_t
get_current_binary_path(
	char* buffer,
	uint32_t buffer_size
);


END_NAMESPACE

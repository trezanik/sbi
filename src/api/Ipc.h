#pragma once

/**
 * @file	Ipc.h
 * @author	James Warren
 * @brief	Cross-interface + GUI communication
 */



#include <string>

#if defined(_WIN32)
#	include <Windows.h>		// named pipes
#elif defined(__linux__)
#	include
#endif

#include "definitions.h"
#include "types.h"



BEGIN_NAMESPACE(APP_NAMESPACE)


/**
 * Low-level class stored by the Interprocess class.
 */
class SBI_API Ipc
{
	friend class Interprocess;
private:

#if defined(_WIN32)
	HANDLE		_read;
	HANDLE		_write;
	uintptr_t	_thread_handle;
	uint32_t	_thread_id;
#elif defined(__linux__)

#endif

	std::string	_name;
	char		_read_buffer[8192];
	char		_write_buffer[8192];


	// private constructor; only Interprocess can create this class
	Ipc();

public:
	~Ipc();



	uint32_t
	Read();

	uint32_t
	Write(
		const char* data
	);
};




END_NAMESPACE

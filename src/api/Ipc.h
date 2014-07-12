#pragma once

/**
 * @file	Ipc.h
 * @author	James Warren
 * @brief	Cross-interface + GUI communication
 */



#include <string>
#include <set>				// listeners
#include <condition_variable>
#include <memory>			// unique_ptr

#if defined(_WIN32)
#	include <Windows.h>		// named pipes
#elif defined(__linux__)
//#	include
#endif

#include "definitions.h"
#include "types.h"


BEGIN_NAMESPACE(APP_NAMESPACE)



// forward declarations
class IpcListener;



/**
 * Low-level class stored by the Interprocess class.
 */
class SBI_API Ipc
{
	friend class Interprocess;
private:
	NO_CLASS_ASSIGNMENT(Ipc);
	NO_CLASS_COPY(Ipc);

#if defined(_WIN32)
	uintptr_t		_thread_handle;
#endif
	uint32_t		_thread_id;
	std::string		_name;
	std::unique_ptr<char>	_buffer;
	uint32_t		_buf_size;
	// mutex + conditional_var pairing
	std::mutex		_lock;
	std::condition_variable	_cv;

	std::set<IpcListener*>	_listeners;


	// private constructor; only Interprocess can create this class
	Ipc();

public:
	~Ipc();

};




END_NAMESPACE

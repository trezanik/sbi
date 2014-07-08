#pragma once

/**
 * @file	Ipc.h
 * @author	James Warren
 * @brief	Cross-interface + GUI communication
 */



#include <string>
#include <set>

#if defined(USING_BOOST_IPC)
#	include <boost/interprocess/managed_shared_memory.hpp>
#endif

#if defined(_WIN32)
#	include <Windows.h>		// named pipes
#elif defined(__linux__)
#	include
#endif

#include "definitions.h"
#include "types.h"



BEGIN_NAMESPACE(APP_NAMESPACE)




/**
 * The class to inherit from if you want to be notified on new data written to
 * an Ipc. Once the final listener has been notified, the data is removed, so
 * it is a listeners responsibility to copy data that must be retained in their
 * notification handler.
 *
 * @class IpcListener
 */
class SBI_API IpcListener
{
private:
	NO_CLASS_ASSIGNMENT(IpcListener);
	NO_CLASS_COPY(IpcListener);

public:
	IpcListener()
	{
	}
	~IpcListener()
	{
	}


	void
	Notify();
};




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

	std::set<IpcListener*>	_listeners;


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

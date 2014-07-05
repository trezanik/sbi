#pragma once

/**
 * @file	Interprocess.h
 * @author	James Warren
 * @brief	Cross-interface + GUI communication
 */



#include <memory>
#include <map>

#if defined(_WIN32)
#	include <Windows.h>		// named pipes
#elif defined(__linux__)
#	include
#endif

#include "definitions.h"



BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class Ipc;
class Interprocess;

// shorthand to save typing
typedef std::map<std::string, std::shared_ptr<Ipc>>	ipc_map;




/**
 * Return codes from IPC function calls (used by the Ipc class too)
 */
enum EIPCStatus
{
	Ok,
	Exists,
	CreateFailed,
	ThreadCreateFailed,
};


enum EIPCAction
{
	Bi,		// Bi-directional (read + write)
	Recv,		// Read-only
	Send		// Write-only
};



struct wfc_params
{
	std::shared_ptr<Ipc>	ipc;
	Interprocess*		thisptr;
};



// placeholder; implement later
struct access_control
{
};




/**
 * Since this is inbuilt into the api, none of the client libraries need to
 * worry about how to do these things. This shall be a wrapper around the IPC
 * mechanism used, enabling us to swap out one for another (e.g. boost) when/if
 * we determine it's more suitable for usage.
 *
 * Exported by the runtime for ease of access.
 *
 * IPC mechanism is abbreviated to IPC-M, and refers to the low level interface
 * such as a named pipe, socket, etc.
 *
 * @class Interprocess
 */
SBI_API
class Interprocess
{
	// we are created on the stack in Runtime::Interprocess()
	friend class Runtime;
private:
	NO_CLASS_ASSIGNMENT(Interprocess);
	NO_CLASS_COPY(Interprocess);



	ipc_map		_ipc_map;



	/**
	 * Calls the class function; static so that it can be the recipient to
	 * a new thread, as it's part of a class.
	 *
	 * [Windows] _beginthreadex requires uint, __stdcall (see msdn docs)
	 *
	 * @param[in] params A pointer to wfc_params, cast void.
	 */
	static uint32_t
#if defined(_WIN32)
	__stdcall
#endif
	ExecWaitForClient(
		void* params
	);



	/**
	 * Handles the maintenance of a created IPC-M.
	 */
	EIPCStatus 
	WaitForClient(
		wfc_params* tp
	);



	// private constructor; we want one instance that is controlled
	Interprocess();

protected:

public:
	~Interprocess();


	/**
	 * Closes the IPC-M with the specified name.
	 *
	 * Will fail if anything is currently connected to the IPC-M, and should
	 * be recalled if it is closed.
	 *
	 * @param[in] name The IPC-M name to close
	 */
	EIPCStatus
	Close(
		char* name
	);



	/**
	 * Connects to an IPC-M by the specified name. Will fail unless a prior
	 * call to Open() succeeded.
	 *
	 * To 'disconnect', just delete the shared_ptr (let it go out of scope
	 * or equivalent). The reference counter will be decremented, automating
	 * the need for connection tracking.
	 *
	 * The returned pointer can be passed around to anything else freely;
	 * but beware that using the raw pointer via get() will not be tracked,
	 * and deletion could occur whilst it is being accessed.
	 *
	 * @param[in] name The unique name to call the IPC-M
	 * @param[in] action
	 * @param unused Temporary holder, will be used by access_control later
	 */
	std::shared_ptr<Ipc>
	Connect(
		char* name,
		EIPCAction action,
		void* unused = nullptr
	);



	/**
	 * Opens a new IPC-M with the specified name.
	 * 
	 * @param[in] name The unique name to call the IPC-M
	 * @param[in] action 
	 * @param unused Temporary holder, will be used by access_control later
	 */
	EIPCStatus
	Open(
		char* name,
		EIPCAction action,
		void* unused = nullptr
	);

};



END_NAMESPACE

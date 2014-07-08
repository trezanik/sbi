#pragma once

/**
 * @file	Interprocess.h
 * @author	James Warren
 * @brief	Cross-interface + GUI communication
 */



#include <memory>
#include <map>
#include <mutex>

#if defined(USING_BOOST_IPC)
#	include <boost/interprocess/managed_shared_memory.hpp>
#endif

#if defined(_WIN32)
#	include <Windows.h>		// named pipes
#elif defined(__linux__)
#	include
#endif

#include "definitions.h"



BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class Ipc;
class IpcListener;
class Interprocess;

// allows type changes in one location
typedef uint16_t		ipc_interface_id;
typedef uint16_t		ipc_message_id;
// shorthand to save typing
typedef std::map<std::string, std::shared_ptr<Ipc>>	ipc_map;
typedef std::map<std::string, ipc_interface_id>		ipc_idmap;




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



struct smoproc_params
{
	std::shared_ptr<Ipc>	ipc;
	Interprocess*		thisptr;
};



// placeholder; implement later
struct access_control
{
};



struct ipc_header
{
	/** 
	 * 65,535 possible registered interface IDs (0 is reserved for invalid)
	 */
	ipc_interface_id	interface_id;
	/**
	 * 65,535 possible message types (consider expansion to uint32_t)
	 */
	ipc_message_id		msg_type;
	std::unique_ptr<char>	buffer;
	uint32_t	buf_size;
	std::mutex	lock;

	ipc_header(uint32_t size)
	{
		buf_size = size;
		buffer.reset(new char[size]);
	}
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
class SBI_API Interprocess
{
	// we are created on the stack in Runtime::Interprocess()
	friend class Runtime;
private:
	NO_CLASS_ASSIGNMENT(Interprocess);
	NO_CLASS_COPY(Interprocess);


	/** 
	 * Registered IPC codes. An interface must request a code for their
	 * usage, then use that one for their communications. This prevents
	 * hardcoding the values which will be far too unwieldly
	 */
	ipc_idmap		_ipc_ids;
	/**
	 * 
	 */
	ipc_map			_ipc_map;
	/**
	 * Next available interface id; starts at 0 (in constructor), which is
	 * an invalid id; each RegisterInterfaceId() call increments this value
	 * and then assigns it - so the first one is always '1'.
	 */
	ipc_interface_id	_available_id;


	static uint32_t
#if defined(_WIN32)
	__stdcall
#endif
	ExecProcSMO(
		void* params
	);
	EIPCStatus
	ProcSMO(
		smoproc_params* tp
	);


#if 0	// Code Removed: Native test setup; remove on boost success
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
#endif



	// private constructor; we want one instance that is controlled
	Interprocess();

protected:

public:
	~Interprocess();


	/**
	 * Attaches an IpcListener to receive notifications of data changes.
	 *
	 * @param[in] listener The IpcListener to add
	 */
	void
	AttachListener(
		const char* identifier,
		IpcListener* listener
	);



	/**
	 * Detaches an IpcListener previously attached. Once removed, the object
	 * will not longer receive notifications.
	 *
	 * @sa AttachListener(), Notify()
	 * @param[in] listener The IpcListener to remove
	 */
	void
	DetachListener(
		const char* identifier,
		IpcListener* listener
	);



	EIPCStatus
	CreateSMO(
		const char* identifier,
		const uint32_t size
		
	);
	EIPCStatus
	ReadSMO(
		const char* identifier,
		char* buf,
		uint32_t buf_size
	);
	EIPCStatus
	WriteSMO(
		const char* identifier,
		const char* data
	);


	/**
	 * Registers the interface called identifier with the next unused 
	 * interface id. If identifier already exists, the existing id is 
	 * returned.
	 *
	 * No method currently exists to unregister; this means that with
	 * ipc_interface_id as an uint16_t, there are 65,535 possible unique 
	 * calls to this function before it will throw a runtime_error due to 
	 * no free ids. 0 is reserved for invalid/unset.
	 *
	 * @param[in] identifier The interface name to register; must be unique
	 * to the interface or you'll get an existing one. Ones designed by us
	 * will be a direct name; user-created ones should follow a specific
	 * naming convention to avoid conflict (such as author_interface_name).
	 * @return An ipc_interface_id. On failure, this function throws, so a
	 * return value will always be a valid id, whether new or existing.
	 */
	ipc_interface_id
	GetInterfaceId(
		const char* identifier
	);


#if 0	// Code Removed: Native test setup; remove on boost success
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
#endif
};



END_NAMESPACE

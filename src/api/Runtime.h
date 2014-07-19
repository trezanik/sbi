#pragma once

/**
 * @file	Runtime.h
 * @author	James Warren
 * @brief	Application globally-accessible singleton
 * @todo	Try to think of a _clean_ way to get Allocator in this file as
 *		a unique_ptr, working around the override limitations. Currently
 *		resides as a static member of the Memory() function as a simple
 *		but effective hack. Addendum: ditto for Engine, Pool, now too!
 */



#include <set>
#include <vector>
#include <memory>

#if defined(_WIN32)
	// avoid including a whole header for one unchanging typedef
	typedef void* HANDLE;
//#	define WIN32_LEAN_AND_MEAN
//#	include <Windows.h>		// HANDLE
#else
#	include <pthread.h>		// pthread_t
#endif

#include "char_helper.h"
#include "definitions.h"


BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class Allocator;
class Configuration;
class Log;
class Interprocess;
class RpcServer;


// required definitions
#define THREADINFO_FUNCTION_BUFFER_SIZE		64
#define THREADINFO_MAX_FUNCTION_LENGTH		(THREADINFO_FUNCTION_BUFFER_SIZE-1)




/**
 * Holds the details of each thread we manually create (not those that the 3rd
 * party or system create); used for clean synchronization on shutdown.
 *
 * On Linux/Unix, these hold posix thread details; on Windows, it's the HANDLEs
 * and IDs.
 *
 * We make sure the thread id has the same name to avoid if preprocessor spam
 * in function calls, since Win32's thread handles are unique.
 *
 * @todo
 * Add a sync event for each, so we can reset after setting _quitting?
 *
 * @sa Runtime::AddManualThread, Runtime::ThreadStopping
 * @struct thread_info
 */
struct thread_info
{
#if defined(_WIN32)
	uintptr_t	thread_handle;
	uint32_t	thread;
#elif defined(__linux__) || defined(BSD)
	pthread_t	thread;
#endif
	char		called_by_function[THREADINFO_FUNCTION_BUFFER_SIZE];

	// yes, both of these are required as a result of WaitThenKillThread
	bool operator == (const thread_info& ti) const
	{
		if ( thread == ti.thread 
		    && strcmp(called_by_function, ti.called_by_function) == 0 )
		    return true;
		return false;
	}
	bool operator == (const thread_info* ti) const
	{
		if ( thread == ti->thread 
		    && strcmp(called_by_function, ti->called_by_function) == 0 )
		    return true;
		return false;
	}
};




typedef void* (*getter)(void* params);

struct runtime_object_accessor
{
	char	name[MAX_LEN_GENERIC];
	getter	get;
};



/**
 * Dedicated class for storing all 'global' variables. Meyers Singleton, so no
 * memory leaks.
 *
 * Globally accessible to any file including runtime.h, but only app_init() and
 * app_free() have the ability to create and destroy the class, and access any
 * other internals.
 *
 * The classes returned are all created on the stack as static variables in 
 * their respective functions. This has much less overhead than having them as
 * std::unique_ptr's that are allocated on the heap, and we can access the raw
 * pointer directly.
 *
 * While this is a technical misuse of a Singleton, since there can only ever
 * be one runtime (which we can then use to interface with the host OS and also
 * maintains if the app is quitting, for example), it seems somewhat appropriate
 * and can easily be replaced, the way it has been designed.
 *
 * Yes, this can be implemented in other ways - but this is clear and concise,
 * without _too_ many of the issues commonly associated with bad singleton use.
 *
 * @class Runtime
 */
class SBI_API Runtime
{
	/** only this function can expose created classes */
	//friend void app_init(int32_t argc, char** argv);
	/** and only this function can access internals when shutting down */
	//friend void app_stop();
private:
	NO_CLASS_ASSIGNMENT(Runtime);
	NO_CLASS_COPY(Runtime);

	Runtime();
	~Runtime();

	
	/**
	 * Set if the application is entering a shutdown state; can be used by
	 * threads that have no other synchronization object, as a sign for when
	 * to stop. Also used for detection that the application has closed
	 * cleanly.
	 *
	 * False by default, set to true when DoShutdown() is called. No public
	 * method exists to unset this, intentionally.
	 */
	bool		_quitting;


	/**
	 * Holds all the threads created by our application code; note that this
	 * does not cover ones created by the operating system or runtime
	 * libraries - this is just those done by our code, AND where they're
	 * then provided to the Runtime for recording.
	 *
	 * Third-party interfaces can choose to ignore this, although for the
	 * sake of debugging and some synchronization, it is highly recommended
	 * not to.
	 */
	std::vector<std::shared_ptr<thread_info>>	_manual_threads;





	std::set<runtime_object_accessor*>	_runtime_objects;

	runtime_object_accessor*
	FindModule(
		runtime_object_accessor* search
	);
	
public:

	static Runtime& Instance()
	{
		static Runtime	rtime;
		return rtime;
	}



	/**
	 * Adds a created threads details into the tracked vector.
	 *
	 * Recommended to do this in the thread function itself, as it's sure to
	 * have valid handles/ids, and will never execute after a thread could
	 * potentially have returned from its function already.
	 *
	 * @param[in] ti A populated thread_info struct
	 */
	void
	AddManualThread(
		std::shared_ptr<thread_info> ti
	);


	/**
	 * Gets the configuration.
	 *
	 * @return A pointer to the static instance within the runtime.
	 */
	Configuration*
	Config() const;


	/**
	 * Sets _quitting to true, and enters a waiting phase for all existing
	 * threads to close, and to kill on timeout.
	 *
	 * Pretty much responsible for cleaning up anything created in app_exec
	 * that is not handled elsewhere.
	 */
	void
	DoShutdown();


	/**
	 * Loads an interface from a module name (e.g. "libirc" will return a
	 * pointer to the IRC interface returned within the library).
	 *
	 * @todo Need a proper data type so we're not blindly casting
	 */
	void*
	GetObjectFromModule(
		const char* module_name
	);


	/**
	 * Gets the interprocess communication class.
	 *
	 * @return A pointer to the static instance within the runtime.
	 */
	class Interprocess*
	Interprocess() const;


	/**
	 * Gets whether DoShutdown() has been called; mostly used for threads
	 * and other sync objects to know when they should close down, or stop.
	 */
	bool
	IsQuitting()
	{
		return _quitting;
	}


	/**
	 * Gets the logging class.
	 *
	 * @return A pointer to the static instance within the runtime.
	 */
	Log*
	Logger() const;


#if defined(USING_MEMORY_DEBUGGING)
	/**
	 * Gets the memory allocator for the app. If not debugging memory, this
	 * will not exist, and the macros MALLOC and FREE will call the real
	 * malloc() and free() functions.
	 *
	 * @sa Pools()
	 * @return A pointer to the static instance within the runtime.
	 */
	Allocator*
	Memory() const;
#endif


	/**
	 * Brings up a notification dialog for the windowing system the 
	 * operating system provides.
	 *
	 * On Windows, this calls MessageBox(), which steals focus and blocks
	 * the thread that triggered the execution. As such, should only be used
	 * when the application pause/user notification is essential, or if an 
	 * error occurs that will trigger the app to be aborted anyway.
	 * 
	 * Currently no implementation on Linux/FreeBSD - investigating the best
	 * way to report a simple GUI message without having to connect to X,
	 * create atoms, etc. (and we're not using a framework for something so
	 * simple either - we're anti-bloat!)
	 *
	 * @todo Linux/Unix implementation
	 * @param[in] text_buffer The main body of the string to display
	 * @param[in] title The window title text
	 */
	void
	Report(
		const char* text_buffer,
		const char* title
	) const;


	/**
	 * Gets the RPC server class.
	 *
	 * @return A pointer to the static instance within the runtime.
	 */
	RpcServer*
	RPC() const;



	/**
	 * Causes the runtime to amend its _manual_threads storage - expected to
	 * be called BY the thread function, but can be executed externally if
	 * you're aware of abnormal/alternative termination.
	 *
	 * Usually executed after breaking out of a loop, e.g. on Windows:
	 @code
	 } // end loop/sync

	 runtime.ThreadStopping(GetCurrentThreadId(), __func__);
	 // end thread
	 @endcode
	 *
	 * While not mandatory to be called, on app closure the runtime will try
	 * to wait for all known threads to finish, then terminate them if they
	 * are taking too long. If the system responds to the handles in an
	 * unexpected way, this could cause a crash if this function has not
	 * been processed. Safer just to call this at the end of a thread func.
	 *
	 * @param[in] thread_id The ID of the executing thread that's stopping
	 * @param[in] function The name of the thread function itself
	 * @sa AddManualThread
	 */
	void
	ThreadStopping(
		uint32_t thread_id,
		const char* function
	);


	/**
	 * Locates the supplied thread_id in the stored thread list, and if
	 * found, waits timeout_ms for it to finish before terminating it by
	 * force.
	 *
	 * Should only be called after performing an action that would cause the
	 * thread to actually start its own cleanup routine, or just plain stop.
	 *
	 * When this function returns, the supplied thread_id is guaranteed to
	 * not exist (assuming it was added via AddManualThread); if it does
	 * not exist to begin with, the function returns immediately.
	 *
	 * @param[in] thread_id The ID of the thread to wait for
	 * @param[in] timeout_ms The time to wait in milliseconds before killing
	 */
	void
	WaitThenKillThread(
		uint32_t thread_id,
		uint32_t timeout_ms = 1000
	);
};



// declared in Runtime.cc
extern SBI_API Runtime		&runtime;



END_NAMESPACE

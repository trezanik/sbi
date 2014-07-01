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
 * @sa Runtime::CreateThread, E_THREAD_TYPE
 * @struct thread_info
 */
struct thread_info
{
#if defined(_WIN32)
	HANDLE		thread_handle;
	uint32_t	thread;
#elif defined(__linux__) || defined(BSD)
	pthread_t	thread;
#endif
	std::string	called_from_function;

	/* yes, both of these are required as a result of WaitThenKillThread */
	bool operator == (const thread_info& ti) const
	{
		if ( thread == ti.thread 
		    && called_from_function.compare(ti.called_from_function) == 0 )
		    return true;
		return false;
	}
	bool operator == (const thread_info* ti) const
	{
		if ( thread == ti->thread 
		    && called_from_function.compare(ti->called_from_function) == 0 )
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

	
	bool				_quitting;

	std::vector<thread_info>	_manual_threads;


	/* we no longer store the app classes in this header - they are all
	 * static within their relevant functions in Runtime.cc
	 * The one major downside is that we can't follow through when debugging
	 * - but it's not hard to work around. */


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
	 * Gets the configuration.
	 *
	 * @return A pointer to the static instance within the runtime.
	 */
	Configuration*
	Config() const;


	/**
	 *
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
	 * Causes the runtime to amend its _manual_threads storage - expected to
	 * be called BY the thread function, but can be executed externally if
	 * you're aware of abnormal/alternative termination.
	 *
	 * Usually executed after breaking out of a loop, e.g. on Windows:
	 @code
	 } // end loop/sync

	 runtime.ThreadStopping(GetCurrentThreadId(), __FUNCTION__);
	 @endcode
	 *
	 * While not mandatory to be called, on app closure the runtime will try
	 * to wait for all known threads to finish, then terminate them if they
	 * are taking too long. If the system responds to the handles in an
	 * unexpected way, this could cause a crash if this function has not
	 * been processed.
	 *
	 * @param[in] thread_id The ID of the executing thread that's stopping
	 * @param[in] function The name of the thread function itself
	 * @sa CreateThread
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
	 * Primarily used for ensuring the IrcConnection class doesn't get
	 * deleted while its thread is still running, but can/will expand into
	 * other uses too.
	 *
	 * When this function returns, the supplied thread_id is guaranteed to
	 * not exist (assuming it was spawned by our CreateThread); if it does
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

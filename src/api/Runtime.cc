
/**
 * @file	Runtime.cc
 * @author	James Warren
 * @copyright	James Warren, 2013-2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 * @todo	Sort out duplicate output (stdout/stderr + logging); either get
		the Log class to do ChainOfResponsibility, or use BUILD_STRING
		to store the string var, then output to both
 * @todo	Need a way to get the cheapest, quickest dialog box up inside
		X11/Wayland for reporting, when a GUI is present		
 */


#include <cassert>		// destructor assertions
#include <algorithm>

#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>		// MessageBox
#else
    // Quickest, !cheapest! way to get a dialog box using X/Wayland?
#	include <string.h>
#	include <signal.h>
#	include <pthread.h>
#endif


#include "Runtime.h"		// prototypes
#include "Allocator.h"
#include "Log.h"
#include "Configuration.h"
#include "RpcServer.h"
#include "Terminal.h"
#include "utils.h"		// string handling


BEGIN_NAMESPACE(APP_NAMESPACE)



// global singleton assignment
Runtime	&runtime = Runtime::Instance();



Runtime::Runtime()
{
}



Runtime::~Runtime()
{
}



void
Runtime::AddManualThread(
	std::shared_ptr<thread_info> ti
)
{
	_manual_threads.push_back(ti);

	std::cout << fg_white << "Thread id "
		<< ti->thread << " (" << ti->called_by_function << ") "
		<< "is starting execution\n";
	LOG(ELogLevel::Info) << "Thread id " 
		<< ti->thread << " (" << ti->called_by_function << ") " 
		<< "is starting execution\n";
}



Configuration*
Runtime::Config() const
{
	static Configuration	config;
	return &config;
}



runtime_object_accessor*
Runtime::FindModule(
	runtime_object_accessor* search
)
{
	for ( auto iter : _runtime_objects )
	{
		if ( strcmp((*iter).name, search->name) == 0 )
			return iter;
	}
	
	return nullptr;
}



void
Runtime::DoShutdown()
{
	/* some, not all, threads query with the runtime if the app is quitting
	 * when synchronized, so they can quit cleanly (otherwise, the only way
	 * to stop the thread is to kill it). */
	_quitting = true;

	/** @todo how can we force objects to quit since we have no knowledge of
	 * what modules they are?
	 * Current proposal: have them export a 'quit' function that can be
	 * called from here now, that way they're all done and clean... similar
	 * to the functionality behind the get() getter */

	// shutdown the RPC server if still running
	RPC()->Shutdown();

	// stop any existing threads still running
	while ( !_manual_threads.empty() )
	{
		WaitThenKillThread(_manual_threads[0]->thread);
	}
}



void*
Runtime::GetObjectFromModule(
	const char* module_name
)
{
	bool				could_not_load = true;
	runtime_object_accessor		search;
	runtime_object_accessor*	retval;

	strlcpy(search.name, module_name, sizeof(search.name));
	
	if (( retval = FindModule(&search)) == nullptr )
	{
		// not found, try to load the module

#if _WIN32
		wchar_t	w_module_name[MAX_LEN_GENERIC];
		HANDLE	mod = NULL;
		
		if ( mb_to_utf8(w_module_name, module_name, _countof(w_module_name)) )
			mod = LoadLibrary(w_module_name);

		if ( mod != NULL )
			could_not_load = false;
#else
#endif

		// if we couldn't load it, we'll have to abort
		if ( could_not_load )
		{
			throw std::runtime_error("Failed to load required module");
		}

		retval = (runtime_object_accessor*)MALLOC(sizeof(runtime_object_accessor));
		if ( retval == nullptr )
		{
			throw std::runtime_error("Memory allocation failed");
		}
		CONSTRUCT(retval, runtime_object_accessor);

#if _WIN32
		retval->get	= (getter)get_function_address("instance", w_module_name);
#else
#endif

		if ( retval->get == nullptr )
		{
			throw std::runtime_error("Module did not export the required instance function");
		}

		// add the loaded module to the set
		_runtime_objects.insert(retval);
	}

	return retval->get(nullptr);
}



class Log*
Runtime::Logger() const
{
	static class Log	log;
	return &log;
}



#if defined(USING_MEMORY_DEBUGGING)

Allocator*
Runtime::Memory() const
{
	static Allocator	allocator;
	return &allocator;
}

#endif	// USING_MEMORY_DEBUGGING



void
Runtime::Report(
	const char* text_buffer,
	const char* title
) const
{
#if defined(_WIN32)
	wchar_t		w_text[4096];
	wchar_t		w_title[1024];
	mb_to_utf8(w_text, text_buffer, _countof(w_text));
	mb_to_utf8(w_title, title, _countof(w_title));
	::MessageBox(GetDesktopWindow(), w_text, w_title, MB_OK);
#else
#endif
}



RpcServer*
Runtime::RPC() const
{
	static class RpcServer	rpc;
	return &rpc;
}



void
Runtime::ThreadStopping(
	thread_t thread,
	const char* function
)
{
	bool	found = false;

	// search for the thread id
	for ( auto t : _manual_threads )
	{
		if ( t->thread == thread )
		{
			std::cout << fg_white << "Thread id " << thread
				<< " (" << t->called_by_function << ") "
				<< "is ending execution (called by "
				<< function << ")\n";
			LOG(ELogLevel::Info) << "Thread id " << thread
				<< " (" << t->called_by_function << ") "
				<< "is ending execution (called by "
				<< function << ")\n";
			
			_manual_threads.erase(std::find(_manual_threads.begin(), _manual_threads.end(), t));
			found = true;
			break;
		}
	}

	if ( !found )
	{
		std::cerr << fg_red << "The supplied thread id (" << thread
			<< ") was not found in the list - did you call AddManualThread()?\n";
		LOG(ELogLevel::Info) << "The supplied thread id (" << thread
			<< ") was not found in the list - did you call AddManualThread()?\n";
	}
}



void
Runtime::WaitThenKillThread(
	thread_t thread,
	uint32_t timeout_ms
)
{
	std::shared_ptr<thread_info>	ti = nullptr;
	bool		killed = false;
	bool		success = true;

	for ( auto t : _manual_threads )
	{
#if defined(_WIN32)
		HANDLE	thread_handle = (HANDLE)t->thread_handle;

		if ( t->thread == thread )
		{
			ti = t;

			if ( thread_handle != nullptr && thread_handle != INVALID_HANDLE_VALUE )
			{
				DWORD	exit_code = 0;
				DWORD	wait_ret;
				DWORD	err;

				wait_ret = WaitForSingleObject(thread_handle, timeout_ms);

				if ( wait_ret != WAIT_OBJECT_0 && wait_ret != WAIT_TIMEOUT )
				{
					if ( GetLastError() == ERROR_INVALID_HANDLE )
					{
						std::cerr << fg_red << "The thread handle " << thread_handle << " was reported as invalid by the system\n";
						LOG(ELogLevel::Error) << "The thread handle " << thread_handle << " was reported as invalid by the system\n";
						// exit loop, just remove the thread_info
						success = false;
						break;
					}
				}

				if ( !GetExitCodeThread(thread_handle, &exit_code) || exit_code == STILL_ACTIVE )
				{
					// tried to let the thread go peacefully - kill it
					if ( TerminateThread(thread_handle, EXIT_FAILURE) )
					{
						killed = true;
						std::cerr << fg_yellow << "Thread id " << thread << " has been forcibly killed after timing out\n";
						LOG(ELogLevel::Warn) << "Thread id " << thread << " has been forcibly killed after timing out\n";
					}
					else
					{
						success = false;
						err = GetLastError();
						std::cerr << fg_red << "Failed to terminate thread id " << thread << "; Win32 error " << err << "\n";
						LOG(ELogLevel::Error) << "Failed to terminate thread id " << thread << "; Win32 error " << err << "\n";
					}
				}

				CloseHandle(thread_handle);
			}

#else

		if ( t->thread == thread )
		{
			int32_t		rc;
			timespec	wait_time;

			ti = t;

			wait_time.tv_nsec = (timeout_ms % 1000) * 1000000;

			rc = pthread_timedjoin_np(t->thread, nullptr, &wait_time);

			if ( rc == ETIMEDOUT )
			{
				// tried to let the thread go peacefully - stop it
				pthread_cancel(t->thread);
				// wait again
				rc = pthread_timedjoin_np(t->thread, nullptr, &wait_time);

				if ( rc == ETIMEDOUT )
				{
					std::cerr << fg_yellow << "Thread " << t->thread << " has been forcibly killed after failing to finish on request\n";

					// Just kill it and live with any resource leaks
					pthread_kill(t->thread, SIGKILL);
					killed = true;
				}
				else if ( rc != 0 )
				{
					std::cerr << fg_red << "Received errno " << rc << " after waiting for thread " << t->thread << " to finish\n";
				}
			}
#endif	// _WIN32

			break;
		}

	}

	/* the executing thread MUST call ThreadStopping(), in which case this
	 * thread_info will have already be removed from the list, so there's
	 * nothing to do.
	 * If we had to kill it however, the function would never have been
	 * called, so we need to remove it manually + notify */
	if ( ti != nullptr && killed )
	{
		std::cerr << fg_yellow << "Thread id " << thread << " has been killed\n";
		LOG(ELogLevel::Warn) << "Thread id " << thread << " has been killed\n";
		_manual_threads.erase(std::find(_manual_threads.begin(), _manual_threads.end(), ti));
	}
	else if ( ti != nullptr && !success )
	{
		// not killed, waiting failed, error reported - just remove it
		_manual_threads.erase(std::find(_manual_threads.begin(), _manual_threads.end(), ti));
	}
	else
	{
		/* if the thread_info still exists, we've been told a lie, or
		 * the thread function never called ThreadStopping() */

		for ( auto t : _manual_threads )
		{
			if ( t->thread == thread )
			{
				std::cerr << fg_red << "Thread id " << thread <<
					" still exists after a successful wait for the thread to finish;"
					" was Runtime::ThreadStopping() not executed or did the system lie?";
				LOG(ELogLevel::Warn) << "Thread id " << thread <<
					" still exists after a successful wait for the thread to finish;"
					" was Runtime::ThreadStopping() not executed or did the system lie?";

				// best to remove it anyway
				_manual_threads.erase(std::find(_manual_threads.begin(), _manual_threads.end(), t));
				break;
			}
		}
	}
}



END_NAMESPACE

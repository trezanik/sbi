
/**
 * @file	Ipc.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include "Ipc.h"


BEGIN_NAMESPACE(APP_NAMESPACE)



Ipc::Ipc()
{
#if defined(_WIN32)
	_thread_handle = 0;
#endif
	_thread_id = 0;
	_buf_size = 0;
}



Ipc::~Ipc()
{
#if defined(_WIN32)

	if ( (HANDLE)_thread_handle != nullptr 
	     && (HANDLE)_thread_handle != INVALID_HANDLE_VALUE )
	{
		// we could make this safer, but so unlikely to ever be an issue
		CloseHandle((HANDLE)_thread_handle);
	}

#elif defined(__linux__)



#endif
}




END_NAMESPACE

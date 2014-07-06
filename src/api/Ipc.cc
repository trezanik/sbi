
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
	_read = nullptr;
	_write = nullptr;
	_thread_handle = 0;
	_thread_id = 0;
}



Ipc::~Ipc()
{
#if defined(_WIN32)

	if ( _read != nullptr && _read != INVALID_HANDLE_VALUE )
	{
		CloseHandle(_read);
	}
	if ( _write != nullptr && _write != INVALID_HANDLE_VALUE )
	{
		CloseHandle(_write);
	}
	if ( (HANDLE)_thread_handle != nullptr 
	     && (HANDLE)_thread_handle != INVALID_HANDLE_VALUE )
	{
		// we could make this safer, but so unlikely to ever be an issue
		CloseHandle((HANDLE)_thread_handle);
	}

#elif defined(__linux__)



#endif
}



uint32_t
Ipc::Read()
{
#if defined(_WIN32)

	BOOL	ok;
	DWORD	cnt_read;
	
	ok = ReadFile(_read, _read_buffer, sizeof(_read_buffer), &cnt_read, NULL);

	return cnt_read;

#else



#endif
}



uint32_t
Ipc::Write(
	const char* data
)
{
#if defined(_WIN32)

	BOOL	ok;
	DWORD	cnt_written;

	if ( data == nullptr )
		ok = WriteFile(_read, _write_buffer, strlen(_write_buffer), &cnt_written, NULL);
	else
		ok = WriteFile(_read, data, strlen(data), &cnt_written, NULL);

	return cnt_written;

#else



#endif
}



END_NAMESPACE

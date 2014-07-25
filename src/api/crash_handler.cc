
/**
 * @file	src/api/crash_handler.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */


#if defined(_WIN32)
#	include <Windows.h>
#	include <dbghelp.h>
#	pragma comment ( lib, "dbghelp.lib" )
#endif


#include "crash_handler.h"
#include "utils.h"

using namespace APP_NAMESPACE;



#if defined(_WIN32)

/** @todo use WindowsErrorReporting (http://msdn.microsoft.com/en-us/library/windows/desktop/dd408167%28v=vs.85%29.aspx)
 * (requires Windows 7 or newer)  */

int32_t
write_dump(
	unsigned long code,
	struct _EXCEPTION_POINTERS *ep
)
{
	wchar_t		dump_path[MAX_PATH];
	HANDLE		file_handle;
	MINIDUMP_TYPE	dumptype = MiniDumpWithFullMemory;//MiniDumpWithDataSegs;
	MINIDUMP_EXCEPTION_INFORMATION	ex_info;

	UNREFERENCED_PARAMETER(code);

	// output to executables current directory
	GetCurrentDirectory(_countof(dump_path), dump_path);

	wcscat_s(dump_path, _countof(dump_path), L"\\crash_dump.dmp");

#if _DEBUG
	/* Don't attempt to print the exception code - it is completely incorrect
	 * no matter how you format it. Dump information is correct though.. */
	wprintf(L"A critical exception has occurred. The dump file is being written to '%s' for analysis.", dump_path);
#endif

	file_handle = CreateFile(dump_path, 
				 GENERIC_READ|GENERIC_WRITE,
				 0, NULL,
				 CREATE_ALWAYS,
				 FILE_ATTRIBUTE_NORMAL,
				 NULL);

	if ( file_handle == INVALID_HANDLE_VALUE )
	{
		// notify
		return EXCEPTION_EXECUTE_HANDLER;
	}

	ex_info.ClientPointers		= false;
	ex_info.ExceptionPointers	= ep;
	ex_info.ThreadId		= GetCurrentThreadId();

	if ( !MiniDumpWriteDump(GetCurrentProcess(),
		GetCurrentProcessId(),
		file_handle, dumptype, &ex_info, NULL, NULL) )
	{
		// notify
	}
	else
	{
		// log
	}

	CloseHandle(file_handle);
	return EXCEPTION_EXECUTE_HANDLER;
}


#endif	// _WIN32

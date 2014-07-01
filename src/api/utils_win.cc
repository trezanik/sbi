#if defined(_WIN32)
/**
 * @file	utils_win.cc
 * @author	James Warren
 * @copyright	James Warren, 2013-2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <vector>
#include <Windows.h>
#include <Shlwapi.h>
#include <Psapi.h>
#include "Allocator.h"			// memory allocation macros
#include "utils_win.h"			// prototypes
#include "Terminal.h"

#if IS_VISUAL_STUDIO
	// special, non-standard libraries
#	pragma comment ( lib, "Psapi.lib" )
#	pragma comment ( lib, "Version.lib" )
#	pragma comment ( lib, "Shlwapi.lib" )
#endif



BEGIN_NAMESPACE(APP_NAMESPACE)



bool
path_exists(
	const wchar_t* path
)
{
	return (PathFileExists(path) != 0);
}



wchar_t*
error_code_as_string(
	uint64_t code
)
{
	static wchar_t	error_str[512];

	error_str[0] = '\0';

	if ( !FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		(DWORD)code,
		0,
		error_str,
		_countof(error_str),
		nullptr)
	)
	{
		// always return a string, some callers expect it :)
		wcscpy_s(error_str, _countof(error_str), L"(unknown error code)");
		return &error_str[0];
	}


	wchar_t*	p;

	// some of the messages helpfully come with newlines at the end..
	while ( (p = wcsrchr(error_str, '\r')) != nullptr )
		*p = '\0';
	while ( (p = wcsrchr(error_str, '\n')) != nullptr )
		*p = '\0';

	return &error_str[0];
}



uint32_t
get_current_binary_path(
	wchar_t* buffer,
	uint32_t buffer_size
)
{
	wchar_t*	r;
	uint32_t	res = 0;

	if ( buffer_size < 2 )
		return 0;

	if ( (res = GetModuleFileName(nullptr, buffer, buffer_size)) == 0
	    || res == buffer_size )	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms683197(v=vs.85).aspx @ WinXP return value
	{
		//generate_error_str_arg(EC_ApiFunctionFailed, 0, "Failed to fully retrieve the current binarys module path");
		return 0;
	}

	// find the last trailing path separator
	if ( (r = wcsrchr(buffer, PATH_CHAR)) == nullptr )
	{
		//generate_error_str_arg(EC_ApiFunctionFailed, 0, "The buffer for the current path contained no path separators");
		return 0;
	}

	// nul out the character after it, ready for appending
	*++r = '\0';

	// return number of characters written, after taking into account the new nul
	return (r - &buffer[0]);
}



bool
get_file_version_info(
	wchar_t* path,
	file_version_info* fvi
)
{
	DWORD			size;
	DWORD			dummy;
	uint8_t*		data = nullptr;
	VS_FIXEDFILEINFO*	finfo;
	uint32_t		length;

	/* set to defaults; prevents old information if the struct is reused,
	 * no need for the caller to handle failures itself */
	fvi->description[0] = '\0';
	fvi->build = 0;
	fvi->major = 0;
	fvi->minor = 0;
	fvi->revision = 0;


	if ( (size = GetFileVersionInfoSize(path, &dummy)) == 0 )
		goto failed;

	if ( (data = (uint8_t*)malloc(size)) == nullptr )
		goto failed;

	if ( !GetFileVersionInfo(path, NULL, size, &data[0]) )
		goto failed;

	if ( !VerQueryValue(data, L"\\", (void**)&finfo, &length) )
		goto failed;

	fvi->major = HIWORD(finfo->dwFileVersionMS);
	fvi->minor = LOWORD(finfo->dwFileVersionMS);
	fvi->revision = HIWORD(finfo->dwFileVersionLS);
	fvi->build = LOWORD(finfo->dwFileVersionLS);

	/** @todo get file description */

	// required data copied, can now free it
	free(data);

	return true;

failed:
	if ( data != nullptr )
		free(data);
	return false;
}



void*
get_function_address(
	const char* func_name,
	const wchar_t* module_name
)
{
	HMODULE		module = NULL;
	void*		func_address;

	if ( func_name == nullptr )
		return nullptr;
	if ( module_name == nullptr )
		return nullptr;

	module = GetModuleHandle(module_name);

	if ( module == NULL )
	{
		return nullptr;
	}

	func_address = GetProcAddress(module, func_name);

	return func_address;
}



std::vector<ModuleInformation*>
get_loaded_modules()
{
	std::vector<ModuleInformation*>	ret;
	ModuleInformation*		mi;
	HANDLE		process_handle = GetCurrentProcess();
	HMODULE*	modules;
	wchar_t		module_path[MAX_PATH];
	DWORD		size;
	DWORD		module_count;

	/* EnumProcessModulesEx handles 32 and 64-bit binaries properly, or at
	 * least with choice; it is only available on Vista and Server 2008
	 * onwards however. */
#if MINIMUM_TARGET < _WIN32_WINNT_VISTA
	// xp, 5.2 era
	if ( !EnumProcessModules(process_handle, nullptr, 0, &size) )
		return ret;

	modules = (HMODULE*)malloc(size);

	if ( !EnumProcessModules(process_handle, modules, size, &size) )
	{
		free(modules);
		return ret;
	}
#else
	if ( !EnumProcessModulesEx(process_handle, nullptr, 0, &size, LIST_MODULES_ALL) )
		return ret;

	modules = (HMODULE*)malloc(size);

	if ( !EnumProcessModulesEx(process_handle, modules, size, &size, LIST_MODULES_ALL) )
	{
		app_free(modules);
		return ret;
	}
#endif

	module_count = (size / sizeof(HMODULE));

	for ( DWORD i = 0; i < module_count; i++ )
	{
		// errors not handled; not much we can do with them anyway..
		if ( GetModuleFileNameEx(process_handle, modules[i], module_path, _countof(module_path)) > 0 )
		{
			mi = (ModuleInformation*)MALLOC(sizeof(ModuleInformation));
			if ( mi == nullptr )
			{
				throw std::runtime_error("Memory allocation failed");
			}

			wcscpy_s(mi->name, module_path);
			get_file_version_info(mi->name, &mi->fvi);

			ret.push_back(mi);
		}
	}

	free(modules);

	return ret;
}



bool
mb_to_utf8(
	wchar_t* dest,
	const char* src,
	const uint32_t dest_size
)
{
	if ( src == nullptr || dest == nullptr || dest_size < 2 )
	{
		return false;
	}

	if ( MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, src, -1, dest, dest_size) == 0 )
		return false;

	return true;
}



uint32_t
wcs_format(
	wchar_t* destination,
	uint32_t dest_size,
	wchar_t* format,
	...
)
{
	int32_t		res = 0;
	va_list		varg;

	if ( destination == nullptr )
		return 0;
	if ( format == nullptr )
		return 0;
	if ( dest_size <= 1 )
		return 0;

	va_start(varg, format);

#if MSVC_IS_VS8_OR_LATER
#	pragma warning ( push )
#	pragma warning ( disable : 4996 ) // _vsnwprintf - unsafe function
#endif
	/* always leave 1 for the nul terminator - this is the security complaint
	 * that visual studio will warn us about. Since we have coded round it,
	 * forcing each instance to include '-1' with a min 'dest_size' of 1, this
	 * is perfectly safe. */
	res = _vsnwprintf(destination, (dest_size - 1), format, varg);

#if MSVC_IS_VS8_OR_LATER
#	pragma warning ( pop )
#endif

	va_end(varg);

	if ( res == -1 )
	{
		// destination text has been truncated/error
		destination[dest_size - 1] = '\0';
		return 0;
	}
	else
	{
		// to ensure nul-termination
		destination[res] = '\0';
	}

	// will be positive as not an error 
	return (uint32_t) res;
}



bool
wide_to_mb(
	char* dest,
	const wchar_t* src,
	const uint32_t dest_size
)
{
	if ( src == nullptr || dest == nullptr || dest_size < 2 )
	{
		return false;
	}

	if ( WideCharToMultiByte(
		CP_ACP, WC_NO_BEST_FIT_CHARS|WC_COMPOSITECHECK,
		src, -1, dest, dest_size, "?", NULL) == 0 )
	{
		// error_code_as_string (returns wchar_t!)
		std::cerr << fg_red << "WideCharToMultiByte() failed: " 
			<< "error code " << GetLastError() << "\n";
		return false;
	}

	return true;
}



END_NAMESPACE


#endif	// _WIN32

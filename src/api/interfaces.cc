
/**
 * @file	interfaces.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */


#if defined(USING_BOOST)
#endif
#if defined(_WIN32)
#	include <Windows.h>
#endif
#include "interfaces.h"		// prototypes
#include "Runtime.h"		// needed for Log
#include "Log.h"
#include "types.h"
#include "utils.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



std::vector<std::string>
get_available_interfaces()
{
	std::vector<std::string>	ret;
	uint32_t	func_num = 0;
	char*		func_names[] = {
		"destroy_interface",
		"instance",
		"spawn_interface"
	};
	uint32_t	funcarray_size = sizeof(func_names) / sizeof(char*);
	// as noted in the header, return value is actually EInterfaceStatus
	typedef int32_t (*fp_interface)();
	typedef void* (*fp_instance)(void*);
	fp_interface	pf_destroyinterface;
	fp_instance	pf_instance;
	fp_interface	pf_spawninterface;

#if defined(USING_BOOST)
#elif defined(_WIN32)
	HMODULE			module;
	WIN32_FIND_DATA		wfd;
	HANDLE			search_handle;
	wchar_t			search[] = L"*.dll";
	char			mb[MAX_PATH];
	bool			push_back = false;

	if (( search_handle = FindFirstFile(search, &wfd)) == INVALID_HANDLE_VALUE )
		return ret;

	do
	{
		module = LoadLibrary(wfd.cFileName);

		// error 126 means the file or one of its dependencies is missing
		if ( module == nullptr )
		{
			LOG(ELogLevel::Error) << "LoadLibrary failed - error code " << GetLastError() << "\n";
			continue;
		}

		push_back = true;

		for ( func_num = 0; func_num != funcarray_size; func_num++ )
		{
			if ( func_num == 0 )		// destroy_interface
			{
				if ( (pf_destroyinterface = (fp_interface)GetProcAddress(module, func_names[func_num])) == nullptr )
				{
					FreeLibrary(module);
					push_back = false;
					break;
				}
			}
			else if ( func_num == 1 )	// instance
			{
				if ( (pf_instance = (fp_instance)GetProcAddress(module, func_names[func_num])) == nullptr )
				{
					FreeLibrary(module);
					push_back = false;
					break;
				}
			}
			else if ( func_num == 2 )	// spawn_interface
			{
				if ( (pf_spawninterface = (fp_interface)GetProcAddress(module, func_names[func_num])) == nullptr )
				{
					FreeLibrary(module);
					push_back = false;
					break;
				}
			}
			else
			{
				// unknown
				throw std::runtime_error("Incomplete handler for interface function pointers");
			}
		}

		if ( push_back )
		{
			wide_to_mb(mb, wfd.cFileName, _countof(mb));
			ret.push_back(mb);
			push_back = false;
		}
		
	} while ( FindNextFile(search_handle, &wfd) );

	FindClose(search_handle);

#else

#endif

    // return the vector, empty or not
    return ret;
}



std::vector<std::string>
get_available_modules()
{
	std::vector<std::string>	ret;

	return ret;
}



END_NAMESPACE

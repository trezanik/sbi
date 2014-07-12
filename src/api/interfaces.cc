
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
#elif defined(__linux__)
#	include <dlfcn.h>
#	include <dirent.h>
#	include <unistd.h>		// getcwd
#	include <stdexcept>		// std::runtime_error
#	include <string.h>		// strchr
#endif
#include "interfaces.h"		// prototypes
#include "Runtime.h"		// needed for Log
#include "Log.h"
#include "types.h"
#include "utils.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



AvailableInterfaceDetails::AvailableInterfaceDetails()
{
	library_handle = nullptr;
	pf_destroy_interface = nullptr;
	pf_instance = nullptr;
	pf_spawn_interface = nullptr;
}



AvailableInterfaceDetails::~AvailableInterfaceDetails()
{
	if ( library_handle != nullptr )
	{
#if defined(_WIN32)
		FreeLibrary((HMODULE)library_handle);
#elif defined(__linux__)
		dlclose(library_handle);
#endif
	}
}



interfaces_vector_t
get_available_interfaces()
{
	interfaces_vector_t		ret;
	uint32_t	func_num = 0;
	char*		func_names[] = {
		"destroy_interface",
		"instance",
		"spawn_interface"
	};
	uint32_t	funcarray_size = sizeof(func_names) / sizeof(char*);

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
		
		// put here for automatic cleanup when out of scope unless stored
		std::shared_ptr<AvailableInterfaceDetails>	aid(new AvailableInterfaceDetails);

		for ( func_num = 0; func_num != funcarray_size; func_num++ )
		{
			if ( func_num == 0 )		// destroy_interface
			{
				if ( (aid->pf_destroy_interface = (fp_interface)GetProcAddress(module, func_names[func_num])) == nullptr )
				{
					FreeLibrary(module);
					push_back = false;
					break;
				}
			}
			else if ( func_num == 1 )	// instance
			{
				if ( (aid->pf_instance = (fp_instance)GetProcAddress(module, func_names[func_num])) == nullptr )
				{
					FreeLibrary(module);
					push_back = false;
					break;
				}
			}
			else if ( func_num == 2 )	// spawn_interface
			{
				if ( (aid->pf_spawn_interface = (fp_interface)GetProcAddress(module, func_names[func_num])) == nullptr )
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

			aid->file_name		= mb;
			aid->library_handle	= (void*)module;
			aid->group		= "Current Directory";

			//ret.push_back(std::make_shared<AvailableInterfaceDetails>(aid));
			ret.push_back(aid);
			push_back = false;
		}
		
	} while ( FindNextFile(search_handle, &wfd) );

	FindClose(search_handle);

#else

	void*		lib_handle;
	char*		err;
	bool		push_back = false;
	DIR*		dir;
	struct dirent*	file = nullptr;
	char		curdir[1024];
	char*		filename;
	char*		p;

	if ( getcwd(curdir, sizeof(curdir)) == nullptr )
	{
		LOG(ELogLevel::Error) << "getcwd failed - error: " << errno << "\n";
		return ret;
	}

	dir = opendir(curdir);

	if ( dir == nullptr )
	{
		LOG(ELogLevel::Error) << "opendir failed - error: " << errno << "\n";
		return ret;
	}

	while (( file = readdir(dir)) != nullptr )
	{
		// here you go, a strrstr equivalent ;) (...to validate file extension)
		if (( p = strrchr(file->d_name, '.')) == nullptr )
			continue;
		if ( strcmp(p, ".so") != 0 || *(p+3) != '\0' )
			continue;

		filename = file->d_name;

		if (( lib_handle = dlopen(filename, RTLD_NOW)) == nullptr )
		{
			LOG(ELogLevel::Error) << "dlopen failed - error: " << dlerror() << "\n";
			continue;
		}

		std::shared_ptr<AvailableInterfaceDetails>	aid(new AvailableInterfaceDetails);

		for ( func_num = 0; func_num != funcarray_size; func_num++ )
		{
			if ( func_num == 0 )		// destroy_interface
			{
				aid->pf_destroy_interface = (fp_interface)dlsym(lib_handle, func_names[func_num]);
				if (( err = dlerror()) != nullptr )
				{
					dlclose(lib_handle);

					LOG(ELogLevel::Error) << "Failed to load " << filename
						<< "; dlsym() reported '" << err
						<< "' with '" << func_names[--func_num] << "'\n";
					push_back = false;
					break;
				}
			}
			else if ( func_num == 1 )	// instance
			{
				aid->pf_instance = (fp_instance)dlsym(lib_handle, func_names[func_num]);
				if (( err = dlerror()) != nullptr )
				{
					dlclose(lib_handle);

					LOG(ELogLevel::Error) << "Failed to load " << filename
						<< "; dlsym() reported '" << err
						<< "' with '" << func_names[--func_num] << "'\n";
					push_back = false;
					break;
				}
			}
			else if ( func_num == 2 )	// spawn_interface
			{
				aid->pf_spawn_interface = (fp_interface)dlsym(lib_handle, func_names[func_num]);
				if (( err = dlerror()) != nullptr )
				{
					dlclose(lib_handle);

					LOG(ELogLevel::Error) << "Failed to load " << filename
						<< "; dlsym() reported '" << err
						<< "' with '" << func_names[--func_num] << "'\n";
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
			aid->file_name		= filename;
			aid->library_handle	= lib_handle;

			ret.push_back(aid);
			push_back = false;
		}
	}

	closedir(dir);

#endif

	// return the vector, empty or not
	return ret;
}



std::vector<std::shared_ptr<AvailableModuleDetails>>
get_available_modules()
{
	std::vector<std::shared_ptr<AvailableModuleDetails>>	ret;



	return ret;
}



END_NAMESPACE

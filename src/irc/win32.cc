#if 0	// File is for reference purposes only in regards to DllMain


#if defined(_WIN32)	// file valid only in Win32 builds
/**
 * @file	win32.cc
 * @author	James Warren
 * @copyright	James Warren, 2013
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#if defined(USING_OPENSSL)
#	pragma comment ( lib, "libeay32.lib" )
#	pragma comment ( lib, "ssleay32.lib" )
#endif

#include <Windows.h>

#include "IrcEngine.h"



BOOL WINAPI
DllMain(
	HINSTANCE dll_instance,
	unsigned long reason,
	LPVOID context
)
{
	// since DllMain can't be within a namespace..
	using namespace APP_NAMESPACE;

	dll_instance;

	if ( reason == DLL_PROCESS_ATTACH )
	{
		// Disable DllMain calls for DLL_THREAD_*
		//DisableThreadLibraryCalls(module_handle);

		if ( context == NULL )
		{
			// dynamic load

			/* create an instance of the IRC engine for when 
			 * demanded after this point. */
			//g_irc_engine.AttachListener();
			
			// Return FALSE if you don't want your module to be dynamically loaded
		}
		else
		{
			// static load
			// Return FALSE if you don't want your module to be statically loaded
			return false;
		}
	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		if ( context == NULL ) 
		{
			// Either loading the DLL has failed or FreeLibrary was called

			// Cleanup
		}
		else
		{
			// Process is terminating

			// Cleanup
		}
	}

	return true;
}



#endif	// _WIN32
#endif

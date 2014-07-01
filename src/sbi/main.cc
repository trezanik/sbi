
/**
 * @file	main.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */


#include <exception>		// C++ exceptions
#include <stdexcept>		// runtime_error
#include <iostream>		// cerr

#include <cstdlib>		// EXIT_FAILURE, EXIT_SUCCESS

#include <api/types.h>		// Standard data types
#include <api/Runtime.h>	// application runtime
#include <api/Log.h>		// Logging class
#include "app.h"		// Core Application



/*-----------------------------------------------------------------------------
 * application entry point
 *----------------------------------------------------------------------------*/

int32_t
main(
	int32_t argc,
	char** argv
)
{
	int32_t		exit_status = EXIT_FAILURE;

	using namespace APP_NAMESPACE;


	/* initialize the application. These are the core essentials; if an
	 * error or exception is raised, we cannot continue with normal startup
	 * and must quit. Only this function has access to expose items to the
	 * application runtime.
	 *
	 * If an exception occurs, the log may well not be open here, so there
	 * will be no record of it. This is why we duplicate the output to
	 * stderr. (We should think about doing this in the function...)
	 */
	try
	{
		app_init(argc, argv);
	}
	catch ( std::runtime_error& e )
	{
		std::cerr << "Initialization runtime error:\n\t" << e.what() << "\n";
		LOG(ELogLevel::Error) << "Initialization runtime error:\n\t" << e.what() << "\n";
		goto abort;
	}
	catch ( std::exception& e )
	{
		std::cerr << "Uncaught exception in initialization:\n\t" << e.what() << "\n";
		LOG(ELogLevel::Error) << "Uncaught exception in initialization:\n\t" << e.what() << "\n";
		goto abort;
	}
	catch ( ... )
	{
		std::cerr << "Unhandled exception in initialization";
		LOG(ELogLevel::Error) << "Unhandled exception in initialization";
		goto abort;
	}


	try
	{
		app_exec();
	}
	/* we throw runtime_errors on a failure we cannot recover from, but do
	 * not throw exceptions ourselves outside of this */
	catch ( std::runtime_error& e )
	{
		std::cerr << "runtime error:\n\t" << e.what() << "\n";
		LOG(ELogLevel::Error) << "runtime error:\n\t" << e.what() << "\n";
		goto abort;
	}
	catch ( std::exception& e )
	{
		std::cerr << "Uncaught exception in execution:\n\t" << e.what() << "\n";
		LOG(ELogLevel::Error) << "Uncaught exception in execution:\n\t" << e.what() << "\n";
		goto abort;
	}
	catch ( ... )
	{
		std::cerr << "Unhandled exception in execution";
		LOG(ELogLevel::Error) << "Unhandled exception in execution";
		goto abort;
	}


	try
	{
		app_stop();
	}
	catch ( std::runtime_error& e )
	{
		std::cerr << "Shutdown runtime error:\n\t" << e.what() << "\n";
		LOG(ELogLevel::Error) << "Shutdown runtime error:\n\t" << e.what() << "\n";
		goto abort;
	}
	catch ( std::exception& e )
	{
		std::cerr << "Uncaught exception in shutdown:\n\t" << e.what() << "\n";
		LOG(ELogLevel::Error) << "Uncaught exception in shutdown:\n\t" << e.what() << "\n";
		goto abort;
	}
	catch ( ... )
	{
		std::cerr << "Unhandled exception in shutdown\n";
		LOG(ELogLevel::Error) << "Unhandled exception in shutdown\n";
		goto abort;
	}


	exit_status = EXIT_SUCCESS;

abort:
	/* special case: should be done in app_stop(), but if an exception is
	 * raised, then we'll never be able to log it! */
	runtime.Logger()->Close();
	return exit_status;
}

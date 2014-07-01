
/**
 * @file	app.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 */


#include <cassert>			// assertions
#include <iostream>			// std::cout

#if defined(_WIN32)
#	include <Windows.h>		// OS API
#	define SECURITY_WIN32		// see sspi.h, needed for Security.h
#	include <Security.h>		// GetCurrentUserEx
#	pragma comment ( lib, "Secur32.lib" )
#else
#	include <sys/signal.h>
#	include <api/utils.h>		// sig_handler
#endif

#include <api/char_helper.h>		// ansi/wide conversion
#include <api/utils.h>			// utility functions
#include <api/Runtime.h>		// application runtime
#include <api/Log.h>			// logging
#include <api/Terminal.h>		// output
#include <api/Allocator.h>		// memory debug log name
#include <api/Configuration.h>		// configuration
#include "app.h"			// prototypes
#include "getopt.h"			// command line parser



BEGIN_NAMESPACE(APP_NAMESPACE)




void
app_exec()
{
	// all classes are safe to use at this point, so we can do ANYTHING now

	uint64_t	start_time = get_ms_time();
	uint64_t	end_time;
	
	if ( runtime.Config()->ui.library.pfunc_spawn_interface == nullptr )
	{
		LOG(ELogLevel::Error) << "There is no GUI to spawn; aborting\n";
		return;
	}
	
	if ( runtime.Config()->ui.library.pfunc_spawn_interface() == 0 )
	{
		LOG(ELogLevel::Debug) << "spawn_interface() executed successfully; handing control to GUI library\n";

		runtime.Config()->ui.library.pfunc_process_interface();
		runtime.Config()->ui.library.pfunc_destroy_interface();
	}

	LOG(ELogLevel::Debug) << "GUI library returned control\n";


	end_time = get_ms_time();
	LOG(ELogLevel::Info) << "The application ran for "
		<< (end_time - start_time) / 1000 << " seconds\n";

	// ensure log entries are written
	runtime.Logger()->Flush();
}



void
app_init(
	int32_t argc,
	char** argv
)
{
	uint64_t	start_time = get_ms_time();
	uint64_t	end_time;


#if defined(__linux__)
	// trap segmentation fault signals so we can print the call stack
	struct sigaction    sa;
	sa.sa_handler = segfault_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;

	if ( sigaction(SIGSEGV, &sa, NULL) == -1 )
		std::cerr << fg_red << "Unable to trap the SIGINT signal\n";
#endif	// __linux__

#if defined(_WIN32)
	/* very first thing; ensure our current directory is that of the binary
	* if we're on Windows; in Visual Studio debugging, this is not the case
	* automatically! */
	wchar_t		cur_path[MAX_PATH];

	get_current_binary_path(cur_path, _countof(cur_path));
	SetCurrentDirectory(cur_path);
#endif


	// have you been modifying anything..
	assert(runtime.Logger() != nullptr);
	assert(runtime.Config() != nullptr);
#if defined(USING_MEMORY_DEBUGGING)
	assert(runtime.Memory() != nullptr);
	// delete all the old log files, if present.
	/** @todo Security risk if these are symbolic links; check + resolve */
	unlink(MEM_LEAK_LOG_NAME);
#endif


	// configuration overrides defaults; opens log file
	runtime.Config()->Load();
	/* parse the command line, assign overrides before other objects are
	 * created. This way command line takes precedence over config file.
	 * We want this done after already logging any initial setups/info. 
	 * Naturally this causes an issue of us logging before obeying a command
	 * line override - so I guess we won't support that! */
	parse_commandline(argc, argv);
	// only execute if debug logging
	if ( runtime.Logger()->LogLevel() == ELogLevel::Debug )
	{
		// Log configuration
		runtime.Config()->Dump();
	}
	// Now load the GUI module (not really for Config() - relocate it)
	runtime.Config()->LoadUI();

	

#if defined(_WIN32)
	{
		// get running user, can help identify permission issues
		wchar_t	buf[1024];
		DWORD	buf_size = _countof(buf);
		GetUserNameEx(NameSamCompatible, buf, &buf_size);
		LOG(ELogLevel::Debug) << "Running as user: " << buf << "\n";

		// if config path is different, we may be in a different folder
		GetCurrentDirectory(_countof(cur_path), cur_path);
		LOG(ELogLevel::Debug) << "Current Directory: " << cur_path << "\n";
	}
#endif	// _WIN32


	


#if defined(_WIN32)
	{
		RECT	work_area;

		if ( SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0) )
		{
			HWND	hwnd = GetConsoleWindow();
			RECT	wnd_rect;

			GetWindowRect(hwnd, &wnd_rect);

			// relocate the console window
#if 0	// Bottom-Left
			work_area.top = work_area.bottom - (wnd_rect.bottom - wnd_rect.top);
#endif
#if 0	// Bottom-Right
			work_area.top = work_area.bottom - (wnd_rect.bottom - wnd_rect.top);
			work_area.left = work_area.right - (wnd_rect.right - wnd_rect.left);
#endif
#if 1	// Top-Left
#endif
#if 0	// Top-Right
			work_area.left = work_area.right - (wnd_rect.right - wnd_rect.left);
#endif

			//SetWindowPos(hwnd, NULL, work_area.left, work_area.top, 0, 0, SWP_NOSIZE);
			MoveWindow(hwnd, work_area.left, work_area.top, 800, 500, true);
		}
		/* intercept ctrl+c - mostly useful for debugging - don't care
		 * if we fail */
		//SetConsoleCtrlHandler((PHANDLER_ROUTINE)sig_handler, TRUE);

		std::vector<ModuleInformation*>	miv;
		uint32_t		num = 0;
		CHARSTREAMTYPE		ss;

		miv = get_loaded_modules();
		if ( miv.size() > 0 )
		{
			ss << "Outputting loaded modules:\n";

			// VS2010 brings *some* C++11 support, but not auto
#if MSVC_BEFORE_VS11
			for ( std::vector<ModuleInformation*>::iterator iter = miv.begin();
			     iter != miv.end(); iter++ )
			{
				ModuleInformation*	mi = (*iter);
#else
			for ( auto mi : miv )
			{
#endif
				ss << "\t* [" << num << "]\t" <<
					mi->name << "  [" <<
					mi->fvi.major << "." <<
					mi->fvi.minor << "." <<
					mi->fvi.revision << "." <<
					mi->fvi.build << "]\n";
				num++;

				FREE(mi);
			}
			miv.clear();

			// already has newline from last entry
			LOG(ELogLevel::Debug) << ss.str();
		}
	}
#endif



	if ( !runtime.Config()->ui.enable_terminal )
	{
#if defined(_WIN32)
		HWND	console_wnd = GetConsoleWindow();
		ShowWindow(console_wnd, SW_HIDE);
#else
		std::cout << "'show_terminal' setting ignored for this operating system\n";
#endif
	}
	

	end_time = get_ms_time();
	std::cout << "Application startup completed in " << (end_time - start_time) << "ms\n";
	LOG(ELogLevel::Info) << "Application startup completed in " << (end_time - start_time) << "ms\n";
}



void
app_stop()
{
	
	runtime.Logger()->Flush();
}



bool
parse_commandline(
	int32_t argc,
	char** argv
)
{
	char		getopt_str[] = "c:h";
	int32_t		opt = getopt(argc, argv, getopt_str);
	//app_config_writer	cfg_writer;

	while ( opt != -1 )
	{
		switch ( opt )
		{
		case 'c':
			//cfg_writer.Set_Path_Configuration(optarg);
			break;
		case 'g':
			// do nothing beyond text event generation
			//cfg_writer.Set_GenerateTextEvents();
			return true;
		case 'h':
		default:
			//display_usage(argv[0], opt);
			return false;
		}

		opt = getopt(argc, argv, getopt_str);
	}

	// nothing invalid or help, so startup can proceed
	return true;
}



END_NAMESPACE

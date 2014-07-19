
/**
 * @file	app.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
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
#	include <pwd.h>			// get user name
#endif

#include <api/char_helper.h>		// ansi/wide conversion
#include <api/utils.h>			// utility functions
#include <api/Runtime.h>		// application runtime
#include <api/Log.h>			// logging
#include <api/Terminal.h>		// output
#include <api/Allocator.h>		// memory debug log name
#include <api/Configuration.h>		// configuration
#include <api/RpcServer.h>		// RPC
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

	/* important: NO LOGGING until runtime.Config()->Load() returns.
	 * The config determines the destination path; logging before this will
	 * not work.
	 * Consider creating in user profile, then moving to config path.. */

#if defined(__linux__)
	char		curdir[1024];
	char		curpath[1024];

	// trap segmentation fault signals so we can print the call stack
	struct sigaction    sa;
	sa.sa_handler = segfault_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;

	if ( sigaction(SIGSEGV, &sa, NULL) == -1 )
		std::cerr << fg_red << "Unable to trap the SIGINT signal\n";

	if ( getcwd(curdir, sizeof(curdir)) == nullptr )
		std::cerr << fg_red << "getcwd failed - error: " << errno << "\n";

	get_current_binary_path(curpath, sizeof(curpath));
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
#elif defined(__linux__)
	{
		/* get running user (we use env to get $HOME, good for
		 * troubleshooting path issues) */
		uid_t	uid;
		passwd*	pw;
		uid = getuid();
		if (( pw = getpwuid(uid)) == nullptr )
		{
			LOG(ELogLevel::Debug) << "Running as user id: "
					      << (unsigned)uid << "\n";
		}
		else
		{
			LOG(ELogLevel::Debug) << "Running as user: "
					      << pw->pw_name << " (id="
					      << (unsigned)uid << ")\n";
		}

		LOG(ELogLevel::Debug) << "Current Directory: " << curdir << "\n";
		LOG(ELogLevel::Debug) << "Application Directory: " << curpath << "\n";
	}
#endif	// _WIN32


	// Now load the GUI module (not really for Config() - relocate it)
	runtime.Config()->LoadUI();


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


#	if !defined(_WIN64)
		/* as we target Windows 7 minimum, this functionality is 
		 * available. Can get dynamically on XP SP3 or later. Supported
		 * on 32-bit processes only */
		DEP_SYSTEM_POLICY_TYPE	cur_dep = GetSystemDEPPolicy();

		// 0=AlwaysOff, 1=AlwaysOn, 2=OptIn, 3=OptOut
		if ( cur_dep == 2 )
		{
			if ( SetProcessDEPPolicy(PROCESS_DEP_ENABLE) )
			{
				LOG(ELogLevel::Debug) << "Enabled DEP on the process\n";
			}
			else
			{
				DWORD	le = GetLastError();
				LOG(ELogLevel::Error) << "Enabling DEP on the process failed; error "
					<< le << " (" << error_code_as_string(le) << ")\n";
			}
		}
#	endif	// _WIN64


		std::vector<ModuleInformation*>	miv;
		uint32_t		num = 0;
		CHARSTREAMTYPE		ss;

		miv = get_loaded_modules();
		if ( miv.size() > 0 )
		{
			ss << "Outputting loaded modules:\n";

			for ( auto mi : miv )
			{
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
#endif	// _WIN32



	if ( !runtime.Config()->ui.enable_terminal )
	{
#if defined(_WIN32)
		HWND	console_wnd = GetConsoleWindow();
		ShowWindow(console_wnd, SW_HIDE);
#else
		std::cout << "'show_terminal' setting ignored for this operating system\n";
#endif
	}
	

	// with everything initialized, we can start the RPC server and finish
	runtime.RPC()->Startup();


	end_time = get_ms_time();
	std::cout << "Application startup completed in " << (end_time - start_time) << "ms\n";
	LOG(ELogLevel::Info) << "Application startup completed in " << (end_time - start_time) << "ms\n";
}



void
app_stop()
{
	// will block, waiting for threads to finish
	runtime.DoShutdown();

	std::cout << "Application closure and cleanup complete\n";
	LOG(ELogLevel::Info) << "Application closure and cleanup complete\n";

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

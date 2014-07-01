
/**
 * @file	Configuration.cc
 * @author	James Warren
 * @copyright	James Warren, 2013-2014
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 */



/*
#if defined(USING_JSON)
#	include <json.h>			// json-c
#	if defined(_WIN32)
#		pragma comment ( lib, "libjson.lib" )
#	endif
#endif*/
#if defined(USING_LIBCONFIG)
#	if IS_VISUAL_STUDIO
#		pragma warning ( push )
		// C4290 : C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#		pragma warning ( disable : 4290 )
#	endif
#	include <libconfig/libconfig.h++>
#	if IS_VISUAL_STUDIO
#		pragma warning ( pop )
#	endif
#	if defined(_WIN32)
#		pragma comment ( lib, "libconfig++.lib" )
#	endif
#endif

#include <stdexcept>			// std::runtime_error

#if defined(__linux__)
#	include <sys/stat.h>			// file ops
#	include <fcntl.h>				// open() options
#	include <dlfcn.h>				// dynamic library loading
#endif

#include "Configuration.h"			// prototypes, definitions
#include "Allocator.h"
#include "Terminal.h"
#include "Log.h"
#include "utils.h"




BEGIN_NAMESPACE(APP_NAMESPACE)





bool
Configuration::CreateDefault()
{
	FILE*	default_config;

	std::cout << fg_grey << "Creating default config file\n";

#if defined(_WIN32)
	if ( (default_config = _fsopen(_path.c_str(), "w", _SH_DENYWR)) == nullptr )
	{
		std::cerr << fg_red << "Could not open " << _path << " to apply the default configuration file\n";
		return false;
	}
#else
	int	fd;
	if ( (fd = open(_path.c_str(), O_WRONLY | O_CREAT, O_NOATIME | S_IRWXU | S_IRGRP | S_IROTH)) == -1 )
	{
		std::cerr << fg_red << "Could not open " << _path << " to apply the default configuration file; errno " << errno << "\n";
		return false;
	}

	if ( (default_config = fdopen(fd, "w")) == nullptr )
	{
		std::cerr << fg_red << "fdopen failed on the file descriptor for " << _path << "; errno " << errno << "\n";
		return false;
    }
#endif

	char*	config_array[] = {
#if defined(USING_LIBCONFIG)
		"log =",
		"{",
		"	path = \"app.log\";"
		"	// 1=Error,2=Warn,3=Info,4=Debug",
		"	level = 4;",
		"};",
		"ui =",
		"{",
		"	enable_terminal = 1;",
		"	command_prefix = \"/\";",
		"	library	= {",
		"		// looks for 'libui-NAME.dll' ",
		"		name : \"qt5\";",
		"	};",
		"};",
	};
#else
	};
#	error "Need a configuration library to use and store defaults"
#endif

	uint32_t	config_array_size = sizeof(config_array) / sizeof(char*);

	for ( uint32_t i = 0; i < config_array_size; i++ )
		fprintf(default_config, "%s\n", config_array[i]);

	fclose(default_config);

	return true;
}



void
Configuration::Dump() const
{
	std::stringstream	log_str;
	//uint32_t		i;

	// we want to start on a newline, logging will have the prefix data
	log_str << "\n\t==== Dumping Parsed Configuration ====\n"
		<< "\t---- Log Settings ----\n"
		<< "\t* log.path = " << log.path.data << "\n"
		<< "\t* log.level = " << log.level << "\n"
		<< "\t---- UI Settings ----\n"
		<< "\t* ui.command_prefix = " << ui.command_prefix.data << "\n"
		<< "\t* ui.library = " << ui.library.file_name.data << "\n"
		<< "\t* ui.enable_terminal = " << ui.enable_terminal << "\n";
	
	log_str << "\t#### End Settings Dump ####\n";

	LOG(ELogLevel::Force) << log_str.str().c_str();
}



void
Configuration::Load(
	const char* specific_path
)
{
	if ( specific_path != nullptr )
		_path = specific_path;

#if defined(USING_LIBCONFIG)
	libconfig::Config	cfg;

#	if defined(_WIN32)
	wchar_t		w[MAX_PATH];
	mb_to_utf8(w, _path.c_str(), _countof(w));
	if ( !path_exists(w) )
#	else
    struct stat sts;
    if ( stat(_path.c_str(), &sts) == -1 && errno == ENOENT )
#	endif
	{
		CreateDefault();
	}

	try
	{
		cfg.readFile(_path.c_str());
	}
	catch ( libconfig::FileIOException& e )
	{
		// error reading the file.
		throw;
	}
	catch ( libconfig::ParseException& e )
	{
		// error parsing the file.
		throw;
	}


	/*---------------------------------------------------------------------
	 * log
	 *--------------------------------------------------------------------*/
	{
		/* we need a little special handling, as we currently have no
		 * log file, and we need a path + level set. We therefore do
		 * these things in the required order.. */
		if ( !cfg.lookupValue("log.path", log.path.data) )
		{
			std::cerr << fg_yellow << "No log path specified; defaulting to 'app.log'\n";
			log.path = "app.log";
		}

		// we read the path first, so we know what to open
		runtime.Logger()->Open(log.path.data.c_str());

		if ( !cfg.lookupValue("log.level", log.level.data) )
		{
			log.level = 4;
			runtime.Logger()->SetLogLevel(ELogLevel::Debug);
			LOG(ELogLevel::Warn) << "No log level specified; defaulting to Debug\n";
		}
		else
		{
            switch ( log.level.data )
			{
			case 1:	runtime.Logger()->SetLogLevel(ELogLevel::Error); break;
			case 2: runtime.Logger()->SetLogLevel(ELogLevel::Warn); break;
			case 3: runtime.Logger()->SetLogLevel(ELogLevel::Info); break;
			default:
				runtime.Logger()->SetLogLevel(ELogLevel::Debug); break;
			}
			
		}

		// from here on, we can log in whatever way we desire.
	}
	/*---------------------------------------------------------------------
	 * ui
	 *--------------------------------------------------------------------*/
	{
	if ( !cfg.lookupValue("ui.library.name", ui.library.file_name.data) )
	{
		// this can still be set on the command line, so is not critical
		LOG(ELogLevel::Warn) << "No library name specified; no GUI library will be loaded!\n";
	}
	
	if ( !cfg.lookupValue("ui.enable_terminal", (int32_t&)ui.enable_terminal.data) )
	{
	}
	if ( !cfg.lookupValue("ui.command_prefix", ui.command_prefix.data) )
	{
		LOG(ELogLevel::Warn) << "No command prefix specified; using '/'\n";
		ui.command_prefix = "/";
	}
#if 0	// I was getting crashes with cmd_prefix on destruction (but not filename...)
	// It no longer appears to be an issue after I used this tmpstr 'method'
	// If it happens again, possibly last lookupValue=string issue?
	if ( !cfg.lookupValue("ui.command_prefix", tmp) )
	{
		LOG(ELogLevel::Warn) << "No command prefix specified; using '/'\n";
		ui.command_prefix = "/";
	}
	else
	{
		ui.command_prefix = tmp;
	}
#endif
	}


#else	// !LIBCONFIG

	// holds the entire config file in memory, ready to parse
	char*			buffer;
	uint32_t		alloc;
	FILE*			fp;

	if ( (fp = _fsopen(_path.c_str(), "r", _SH_DENYWR)) == nullptr )
	{
		printf("Failed to open configuration file '%s'; errno %d\n", 
		       _path.c_str(), errno);
		return;
	}

	// move to file end
	fseek(fp, 0, SEEK_END);
	// obtain file size, and amount to allocate
	alloc = ftell(fp);
	// return to start of file
	fseek(fp, 0, SEEK_SET);
	// allocate the buffer
	buffer = (char*)MALLOC(alloc);
	// copy into memory
	fread(buffer, alloc, 1, fp);
	// done with the file, close it
	fclose(fp);

	struct json_object*	json_root;
	json_root = json_tokener_parse(buffer);

	LoadUI(json_root);
#endif
}



#if 0
void
Configuration::LoadUI(
	json_object* json_root
)
{
	json_object*	json_ui;
	json_object*	json_library;
	json_object*	e;
	std::string	s;

	if ( json_object_object_get_ex(json_root, "ui", &json_ui) )
	{
		if ( json_object_object_get_ex(json_ui, "library", &json_library) )
		{
			if ( json_object_object_get_ex(json_library, "name", &e) )
			{
				s = json_object_get_string(e);
				ui.library.file_name = mbstr_to_chartypestr(s);
			}
		}

		if ( json_object_object_get_ex(json_ui, "enable_terminal", &e) )
		{
			ui.no_terminal = !json_object_get_boolean(e);
		}
		if ( json_object_object_get_ex(json_ui, "enable_gui", &e) )
		{
			// != 0 to prevent warning
			ui.enable_gui = (json_object_get_boolean(e) != 0);
		}
		if ( json_object_object_get_ex(json_ui, "command_prefix", &e) )
		{
			ui.command_prefix = (char)json_object_get_int(e);
		}
	}
#endif



void
Configuration::LoadUI()
{
	char		mb[260];
	uint32_t	func_num = 0;
	char*		func_names[] = {
		"destroy_interface",
		"process_interface",
		"spawn_interface"
	};
	int32_t		(*pfunc[sizeof(func_names)])();
	uint32_t	funcarray_size = sizeof(func_names) / sizeof(char*);

#if defined(_WIN32)

	HMODULE		module;
	wchar_t		w_file[MAX_PATH];

	str_format(mb, sizeof(mb),
		   "libui-%s.dll",
		   ui.library.file_name.data.c_str());

	mb_to_utf8(w_file, mb, _countof(w_file));


	LOG(ELogLevel::Info) << "Loading Dynamic Library '" << CHARSTRINGTYPE(w_file) << "'\n";

	// load from the current directory
	module = LoadLibrary(w_file);

	// error 126 means the libui file or one of its dependencies is missing
	if ( module == nullptr )
		throw std::runtime_error(BUILD_STRING("LoadLibrary failed - error code ", std::to_string(GetLastError())));

	for ( func_num = 0; func_num != funcarray_size; func_num++ )
	{
		if ( (pfunc[func_num] = (int32_t(__cdecl*)())GetProcAddress(module, func_names[func_num])) == nullptr )
		{
			FreeLibrary(module);

			throw std::runtime_error(BUILD_STRING("Failed to load ", mb, "; missing exported function '", func_names[--func_num], "'"));
		}
	}

	ui.library.pfunc_destroy_interface = pfunc[0];
	ui.library.pfunc_process_interface = pfunc[1];
	ui.library.pfunc_spawn_interface = pfunc[2];
	ui.library.module = module;

	LOG(ELogLevel::Debug) << "Library loaded successfully (" << ui.library.module << "). Functions:\n"
		"\t* destroy_interface = " << ui.library.pfunc_destroy_interface << "\n"
		"\t* process_interface = " << ui.library.pfunc_process_interface << "\n"
		"\t* spawn_interface = " << ui.library.pfunc_spawn_interface << "\n";

#else

    str_format(mb, sizeof(mb),
           "libui-%s.so",
           ui.library.file_name.data.c_str());

        LOG(ELogLevel::Info) << "Loading Dynamic Library '" << mb << "'\n";

        void*	lib_handle;
        char*	err;

        lib_handle = dlopen(mb, RTLD_NOW);
        if ( lib_handle == nullptr )
        {
            LOG(ELogLevel::Error) << "dlopen failed - error: " << dlerror() << "\n";
            throw std::runtime_error("Failed to load the requested GUI library!");
        }

        for ( func_num = 0; func_num != funcarray_size; func_num++ )
        {
            pfunc[func_num] = (int32_t(__cdecl*)())dlsym(lib_handle, func_names[func_num]);
            if (( err = dlerror()) != nullptr )
            {
                dlclose(lib_handle);

                LOG(ELogLevel::Error) << "Failed to load " << mb
                                      << "; dlsym() reported '" << err
                                      << "' with '" << func_names[--func_num] << "'\n";
                throw std::runtime_error(err);
            }
        }

        ui.library.pfunc_destroy_interface = pfunc[0];
        ui.library.pfunc_process_interface = pfunc[1];
        ui.library.pfunc_spawn_interface = pfunc[2];
        ui.library.module = lib_handle;

        // void* cast is needed to print addresses, not an int
        LOG(ELogLevel::Debug) << "Library loaded successfully (" << ui.library.module << "). Functions:\n"
            "\t* destroy_interface = " << (void*)ui.library.pfunc_destroy_interface << "\n"
            "\t* process_interface = " << (void*)ui.library.pfunc_process_interface << "\n"
            "\t* spawn_interface = " << (void*)ui.library.pfunc_spawn_interface << "\n";

#endif	// _WIN32

}



const char*
Configuration::Path() const
{
	return _path.c_str();
}



END_NAMESPACE

#pragma once

/**
 * @file	src/api/Configuration.h
 * @author	James Warren
 * @brief	The application configuration
 */



#include <string>
#include <map>
#include "char_helper.h"

#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#endif

// forward declarations
#if defined(USING_JSON)
struct json_object;
#endif


typedef std::map<std::string, std::string>	keyval_str;




BEGIN_NAMESPACE(APP_NAMESPACE)


/**
 *
 *
 * @class Configuration
 */
class SBI_API Configuration
{
	// we are created on the stack in Runtime::Config()
	friend class Runtime;
private:
	NO_CLASS_ASSIGNMENT(Configuration);
	NO_CLASS_COPY(Configuration);


	/** the path to the configuration file */
	std::string		_path;

	/**
	 * Attempts to create a default configuration file.
	 *
	 * Only called when an attempt to load the configuration file fails with
	 * 'file not found'.
	 *
	 * @retval true when the file is created
	 * @retval false if the file could not be created
	 */
	bool
	CreateDefault();
	

	// private constructor; we want one instance that is controlled
	Configuration()
	{
		_path = "sbi.exe.cfg";
	}

public:
	~Configuration() {}


	/**
	 * Dumps the entire configuration, as-is, to the application log file.
	 * ELogLevel::Debug does not need to be set, as it will be output directly, not
	 * checking the level.
	 */
	void
	Dump() const;


	/**
	 * Loads the core requirements of the application; enough to bring it up
	 * and let the user start doing whatever relevant tasks.
	 *
	 * If you want to load something that doesn't need to be present until
	 * after we're already running, use LoadDelayed().
	 *
	 * @param[in] specific_path Set this to a path, relative or full, of the
	 * configuration file to load. This enables a different configuration to
	 * be executed by default simply by setting it on the command line.
	 */
	void
	Load(
		const char* specific_path = nullptr
	);


	/**
	 * @todo relocate the LoadUI() function - it is not applicable in the
	 * Configuration class and should be handled somewhere in app funcs
	 */
	void
	LoadUI();


	/**
	 * Retrieves the configuration file path; pointer remains valid as long
	 * as this class exists (which as long as it's in runtime, is after
	 * main returns).
	 */
	const char*
	Path() const;


	/**
	 * Saves the current configuration to the file previously loaded.
	 */
	void
	Save();


	/*
	 * Uses the concept from here:
	 * http://stackoverflow.com/questions/5424042/class-variables-public-access-read-only-but-private-access-read-write
	 *
	 * Essentially, this class can write to these variables, but all the
	 * others can only get read access.
	 */
	template <class T>
	class proxy
	{
		friend class Configuration;
		// approved class for updating the configuration
		friend class ConfigurationWriter;
	private:
		T data;
		T operator= (const T& arg) {
			data = arg; return data;
		}
	public:
		// 0|NUL|nullptr the datatype on construction
		proxy()
		{
		}
		operator const T&() const {
			return data;
		}
		operator const T&() {
			return data;
		}
	};


	struct {
		proxy<std::string>		path;
		proxy<uint32_t>			level;
	} log;

	struct {
		proxy<bool>			search_curdir;
		proxy<keyval_str>		search_paths;
		keyval_str			get_search_paths() { return search_paths.data; }
	} interfaces;

	struct
	{
		proxy<bool>			search_curdir;
		proxy<keyval_str>		search_paths;
	} modules;

	struct {
		/* If set, all RPC communication will be done using SSL. Any
		 * interface not supporting SSL will not be able to use RPC. */
		proxy<bool>			use_ssl;
		/* If set, the server will only listen on the localhost; remote
		 * connection attempts will always fail. 
		 * When unset, the server will listen on all addresses.
		 * In either situation, a valid username and password are 
		 * mandatory in order to login to the server. */
		proxy<bool>			local_only;
		/* Key-Value mapping of allowed IPs that can connect to the RPC
		 * server, assuming it's not listening in local_only mode.
		 * Connections from IPs not in the values will be automatically
		 * denied; those that are, must then get past the credential 
		 * authentication (username + password). */
		proxy<keyval_str>		allowed_ips;
		keyval_str			get_allowed_ips() { return allowed_ips.data; }
		/* The port the server listens on; will be the RPC_DEFAULT_PORT
		 * definition if not supplied in the configuration */
		proxy<uint16_t>			port;

		struct {
			// Username credential
			proxy<std::string>		username;
			/* Password credential (plaintext); do not set sha1 if 
			 * using this! Instant password<->hash conversion 
			 * without effort.. */
			proxy<std::string>		password;
			/* Password credential (SHA-1); do not set password if 
			 * using this! Instant password<->hash conversion 
			 * without effort..
			 * This is present to cater for people who don't want 
			 * their pass visible to passers by. Brute-forcing the 
			 * unsalted hash is still trivial, so password strength 
			 * should be very high to make this worthwhile. */
			proxy<std::string>		sha1;
			// To add: certificate credential (low-priority feature)
		} auth;
	} rpc;

	struct {
		proxy<std::string>			command_prefix;
		proxy<bool>				enable_terminal;

		struct {
			/* The file suffix (without extension) of the library
			 * to load and use as the GUI (e.g. for the file called
			 * 'libui_qt_5.dll', this will be 'qt_5') */
			proxy<std::string>		file_name;
			/* The name the configuration for the UI appears under
			 * in the config file (so we can load different settings
			 * for different GUI libraries that will use the same
			 * variable names) */
			proxy<std::string>		cfg_name;

#if defined(_WIN32)
			proxy<HMODULE>			module;
#else
			proxy<void*>			module;
#endif
			/* exported functions from the GUI library - must be
			 * present if using a GUI. Assigned and loaded based on
			 * the file_name */
			int32_t	(*pfunc_destroy_interface)();
			int32_t	(*pfunc_process_interface)();
			int32_t	(*pfunc_spawn_interface)();
		} library;

	} ui;
};


END_NAMESPACE

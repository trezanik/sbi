#pragma once

/**
 * @file	Rpc.h
 * @author	James Warren
 * @brief	API RPC Server
 * @note	If this looks similar to Bitcoin et-al's RPC implementation, it
 *		is not a coincidence. I've never had to use json spirit or write
 *		a RPC client or server before, and I'd rather use something that
 *		works and adapt it then reinvent the wheel - incorrectly. 
 *		See bitcoinrpc.h for the original.
 */



#include <string>
#include <map>
#include <list>

#if defined(USING_JSON_SPIRIT_RPC)
#	include <json_spirit/json_spirit_reader_template.h>
#	include <json_spirit/json_spirit_writer_template.h>
#	include <json_spirit/json_spirit_utils.h>
#endif

#include "definitions.h"



BEGIN_NAMESPACE(APP_NAMESPACE)


// the port the server listens on, and the clients must connect to
#define RPC_PORT		50451

// This is needed because the foreach macro can't get over the comma in pair<t1, t2>
#define PAIRTYPE(t1, t2)	std::pair<t1, t2>


typedef json_spirit::Value(*rpc_function)(const json_spirit::Array& params, bool fHelp);



/**
 * Status codes returned from RPC operations that return status codes. Not all
 * RPC operations require return values, much like a sending a UDP packet and 
 * moving on.
 */
enum class ERPCStatus
{
	Ok,
	InvalidParameter,	// Supplied parameter invalid (e.g. outside min/max range)
	MissingParameter,	// Supplied parameter missing (i.e. nullptr)
	Exception,		// Exception in processing
	OutOfMemory,		// Memory allocation failed
	AccessDenied,		// No permissions
	UnknownType,		// Unrecognized type passed as parameter
	NameInUse,		// RpcCommand name is already used
	// json-rpc
	InvalidRequest,
	MethodNotFound,
	InvalidParams,
	InternalError,
	ParseError,
};



/* These definitions are the flags for a RPC Command. Requires a minimum of a 
 * 32-bit variable for storage in the class (assumed to be the case due to the
 * target platforms); the data type is just unsigned, so the available number of
 * flags depends on this.
 *
 * Reserved (API/Developer flags) are 0x00000000 through to 0x0000FFFF; anything
 * after these values can be used by an interface if desired for their own 
 * functionality.
 */
#define RPCF_DEFAULT			0x00000000	// default state; locked, not allowed in test mode
#define RPCF_UNLOCKED			0x00000001	// RPC function is unlocked, callable
#define RPCF_ALLOW_IN_TEST_MODE		0x00000002	// RPC function is allowed in test mode
#define RPCF_USER_DEFINED		0x00010000	// start of user definitions


/**
 * 
 */
class SBI_API RpcCommand
{
private:
	NO_CLASS_ASSIGNMENT(RpcCommand);
	// copy is needed for default constructor -> use in tables

protected:
public:
	// internal, unique name of the command
	std::string	name;
	// the function itself
	rpc_function	actor;
	// RPC Command type flags - see RPCF_Xxxx definitions
	unsigned	flags;
};








/**
*
*/
class SBI_API RpcTable
{
private:
	NO_CLASS_ASSIGNMENT(RpcTable);
	NO_CLASS_COPY(RpcTable);

	std::map<std::string, const RpcCommand*>	_cmd_map;

protected:
public:
	RpcTable();
	~RpcTable();


	/**
	 * Accesses a RpcCommand pointer via the associated command name
	 */
	const RpcCommand*
	operator[] (
		std::string name
	) const;


	/**
	 * Adds a new RPC function.
	 *
	 * Used by Interfaces so they can enable support for their own
	 * functionality, and with other third-party options.
	 *
	 * The one caveat is that the command name must be unique; a name
	 * conflict will be triggered if the same name is used more than once.
	 *
	 * @param[in] new_cmd The RpcCommand to add to the table
	 * @retval ERPCStatus::Ok If the function is added
	 * @return The relevant ERPCStatus error code on failure
	 */
	ERPCStatus
	AddCallableRPC(
		RpcCommand* new_cmd
	);
	

	/**
	 * Retrieves the help string for the associated command name
	 */
	std::string
	Help(
		std::string name
	) const;


	/**
	 * Execute a RPC method.
	 *
	 * @param method Method to execute
	 * @param params Array of arguments (JSON objects)
	 * @returns Result of the call.
	 * @throws an exception of json_spirit::Value if an error occurs.
	 */
	json_spirit::Value
	Execute(
		const std::string &method,
		const json_spirit::Array &params
	) const;
};



/**
 * RPC server class.
 *
 * Interfaces add RPC support for themselves by registering functions into the
 * server, which adds them to the RPC table. These must be unregistered when the
 * interface is unloaded.
 *
 * Inbuilt commands are public methods within the class (so are pretty much the
 * libapi RPC commands).
 */
class SBI_API RpcServer
{
private:
	NO_CLASS_ASSIGNMENT(RpcServer);
	NO_CLASS_COPY(RpcServer);


	RpcTable		_table;


	/** Retrieves a pointer to the RpcTable, to Add/Remove RpcCommands */
	RpcTable*
	GetRpcTable()
	{
		return &_table;
	}


	void
	TypeCheck(
		const json_spirit::Array& params,
		const std::list<json_spirit::Value_type>& expected_types,
		bool fAllowNull
	) const;


	void
	TypeCheck(
		const json_spirit::Object& obj,
		const std::map<std::string, json_spirit::Value_type>& expected_types,
		bool fAllowNull
	) const;



protected:
public:
	RpcServer();
	~RpcServer();


	


	/**
	 * Gets the number of CPU execution cores available to the environment.
	 * 
	 * Attempts to use C++11 hardware_concurrency by default; if this does
	 * not return a value, a fallback value obtained using the OS API is
	 * used.
	 */
	json_spirit::Value
	GetEnvironmentCoreCount(
		const json_spirit::Array& params,
		bool fHelp
	);


	/**
	 * Gets the details for a loaded Interface, such as path, memory address
	 * and RPC commands, amongst others.
	 */
	json_spirit::Value
	GetInterfaceInfo(
		const json_spirit::Array& params,
		bool fHelp
	);

};






END_NAMESPACE

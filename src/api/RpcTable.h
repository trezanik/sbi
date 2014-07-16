#pragma once

/**
 * @file	RpcTable.h
 * @author	James Warren
 * @brief	
 */



#include <map>
#include <string>

#if defined(USING_JSON_SPIRIT_RPC)
#	include <json_spirit/json_spirit_reader_template.h>
#	include <json_spirit/json_spirit_writer_template.h>
#	include <json_spirit/json_spirit_utils.h>
#endif

#include "definitions.h"
#include "rpc_status.h"



BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class RpcCommand;


// typedefs
typedef std::map<std::string, const RpcCommand*>	rpc_map;



/**
 *
 */
class SBI_API RpcTable
{
private:
	NO_CLASS_ASSIGNMENT(RpcTable);
	NO_CLASS_COPY(RpcTable);


	rpc_map			_cmd_map;


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
	 * @sa RemoveRpcCommand
	 * @param[in] new_cmd The RpcCommand to add to the table
	 * @retval ERpcStatus::Ok If the function is added
	 * @return The relevant ERpcStatus error code on failure
	 */
	ERpcStatus
	AddRpcCommand(
		const RpcCommand* new_cmd
	);


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


	/**
	 * Retrieves the help string for the associated command name
	 */
	std::string
	Help(
		std::string name
	) const;


	/**
	 * Removes an existing RPC function.
	 *
	 * The reverse of AddCallableRpc(). Must be called for any RPC commands
	 * that have been added, otherwise an interface unload will cause an
	 * intentional crash.
	 *
	 * API functions cannot be removed; only custom interfaces are allowed
	 * to be modified.
	 *
	 * @sa AddRpcCommand
	 * @param[in] new_cmd The RpcCommand to remove from the table
	 * @retval ERpcStatus::Ok If the function is removed
	 * @return The relevant ERpcStatus error code on failure
	 */
	ERpcStatus
	RemoveRpcCommand(
		const RpcCommand* old_cmd
	);
};




END_NAMESPACE

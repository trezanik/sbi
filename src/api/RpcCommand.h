#pragma once

/**
 * @file	RpcCommand.h
 * @author	James Warren
 * @brief	A RPC command stored in a RpcTable within RpcServer
 */



#include <string>

#if defined(USING_JSON_SPIRIT_RPC)
#	include <json_spirit/json_spirit_utils.h>

	typedef json_spirit::Value(*rpc_function)(
		const json_spirit::Array& params,
		bool fHelp
	);
#endif

#include "definitions.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



/* These definitions are the flags for a RPC Command. Requires a minimum of a
 * 32-bit variable for storage in the class (assumed to be the case due to the
 * target platforms); the data type is just unsigned, so the available number of
 * flags depends on this.
 *
 * Reserved (API/Developer flags) are 0x00000000 through to 0x0000FFFF; anything
 * after these values can be used by an interface if desired for their own
 * functionality.
 *
 * Note that certain flags, like RPCF_NO_DELETE, will be ignored if used on a
 * RpcCommand that is not owned by the API.
 */
#define RPCF_DEFAULT			0x00000000	// default state; locked, not allowed in test mode
#define RPCF_UNLOCKED			0x00000001	// RPC function is unlocked, callable
#define RPCF_ALLOW_IN_TEST_MODE		0x00000002	// RPC function is allowed in test mode
// other reserved RPCF_ flags
#define RPCF_NO_DELETE			0x0000FFFF	// RPC function cannot be removed
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



END_NAMESPACE

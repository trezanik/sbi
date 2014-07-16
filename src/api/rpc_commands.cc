
/**
 * @file	rpc_commands.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include "rpc_commands.h"
#include "RpcServer.h"
#include "Runtime.h"			// RpcServer accessor



BEGIN_NAMESPACE(APP_NAMESPACE)



json_spirit::Value
api_GetEnvironmentCoreCount(
	const json_spirit::Array& params,
	bool fHelp
)
{
	return runtime.RPC()->GetEnvironmentCoreCount(params, fHelp);
}



json_spirit::Value
api_Help(
	const json_spirit::Array& params,
	bool fHelp
)
{
	if ( fHelp || params.size() > 1 )
	{
		throw std::runtime_error(
			"help [command]\n"
			"List commands, or get help for a command."
		);
	}

	std::string	command;

	if ( params.size() > 0 )
		command = params[0].get_str();

	return runtime.RPC()->GetRpcTable()->Help(command);
}



json_spirit::Value
api_Stop(
	const json_spirit::Array& params, 
	bool fHelp
)
{
	if ( fHelp || params.size() > 0 )
	{
		throw std::runtime_error(
			"stop\n"
			"Stops the RPC server, preventing any IPC."
		);
	}

	runtime.RPC()->Shutdown();
	return "SBI RPC server stopping";
}



END_NAMESPACE

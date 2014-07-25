
/**
 * @file	src/api/RpcTable.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <set>

#include "RpcTable.h"
#include "RpcServer.h"			// RpcCommand
#include "JsonRpc.h"			// JsonRpcError
#include "rpc_commands.h"		// api_Xxx rpc functions
#include "utils.h"			// BUILD_STRING



BEGIN_NAMESPACE(APP_NAMESPACE)



static const RpcCommand ApiRpcCommands[] =
{	//  name              function             flags
	//  ----------------  -------------------  --------------------------->>
	{ "help", &api_Help, RPCF_ALLOW_IN_TEST_MODE | RPCF_UNLOCKED },
	{ "stop", &api_Stop, RPCF_ALLOW_IN_TEST_MODE | RPCF_UNLOCKED },
};



RpcTable::RpcTable()
{
	uint32_t	i;

	// add all the ApiRpcCommands to the command map
	for ( i = 0; i < (sizeof(ApiRpcCommands) / sizeof(ApiRpcCommands[0])); i++ )
	{
		const RpcCommand*	pcmd;

		pcmd = &ApiRpcCommands[i];
		_cmd_map[pcmd->name] = pcmd;
	}
}



RpcTable::~RpcTable()
{
}



const RpcCommand*
RpcTable::operator[] (
	std::string name
) const
{
	rpc_map::const_iterator		iter = _cmd_map.find(name);

	if ( iter == _cmd_map.end() )
		return nullptr;

	return (*iter).second;
}



ERpcStatus
RpcTable::AddRpcCommand(
	const RpcCommand* new_cmd
)
{
	// check for a duplicate name that would cause conflicts
	if ( (_cmd_map.find(new_cmd->name)) != _cmd_map.end() )
	{
		return ERpcStatus::NameInUse;
	}

	// name is unique; add it to the map.
	_cmd_map[new_cmd->name] = new_cmd;

	return ERpcStatus::Ok;
}



json_spirit::Value
RpcTable::Execute(
	const std::string& method,
	const json_spirit::Array& params
) const
{
	// Find method
	const RpcCommand*	pcmd = (*this)[method];

	if ( !pcmd )
	{
		throw JsonRpcError(ERpcStatus::MethodNotFound, "Method not found");
	}

#if 0	// Code Removed: we don't have a safe mode
	// Observe safe mode
	std::string	warning_str = GetWarnings("rpc");
	if ( warning_str != "" 
	    && !GetBoolArg("-disablesafemode") 
	    && !pcmd->okSafeMode )
	{
	    throw JsonRpcError(RPC_FORBIDDEN_BY_SAFE_MODE, string("Safe mode: ") + warning_str);
	}
#endif

	try
	{
		// Execute
		json_spirit::Value	result;
		{
			if ( pcmd->flags & RPCF_UNLOCKED )
			{
				result = pcmd->actor(params, false);
			}
			else
			{
				// purpose?
				//LOCK2(cs_main, pwalletMain->cs_wallet);
				result = pcmd->actor(params, false);
			}
		}
		return result;
	}
	catch ( std::exception& e )
	{
		throw JsonRpcError(ERpcStatus::Exception, e.what());
	}
}



std::string
RpcTable::Help(
	std::string command_name
) const
{
	std::string		ret;
	std::set<rpc_function>	func_set;

	for ( rpc_map::const_iterator mi = _cmd_map.begin(); mi != _cmd_map.end(); ++mi )
	{
		const RpcCommand*	pcmd = mi->second;
		std::string		method = mi->first;

		// We already filter duplicates, but these deprecated screw up the sort order
		if ( method.find("label") != std::string::npos )
			continue;
		if ( command_name != "" && method != command_name )
			continue;
		try
		{
			json_spirit::Array	params;
			rpc_function		pfn = pcmd->actor;

			if ( func_set.insert(pfn).second )
				(*pfn)(params, true);
		}
		catch ( std::exception& e )
		{
			// Help text is returned in an exception
			std::string	helpstr = std::string(e.what());

			if ( command_name == "" )
			{
				if ( helpstr.find('\n') != std::string::npos )
					helpstr = helpstr.substr(0, helpstr.find('\n'));
			}

			ret += helpstr + "\n";
		}
	}

	if ( ret == "" )
	{
		ret = BUILD_STRING(
			"help: unknown command: ",
			command_name.c_str(),
			"\n");
	}

	ret = ret.substr(0, ret.size() - 1);
	return ret;
}



ERpcStatus
RpcTable::RemoveRpcCommand(
	const RpcCommand* old_cmd
)
{
	uint32_t		i = 0;
	rpc_map::iterator	iter;

	// check to verify the command name actually exists
	if ( (iter = _cmd_map.find(old_cmd->name)) == _cmd_map.end() )
	{
		return ERpcStatus::MethodNotFound;
	}

	// validate it is not an inbuilt (API) RpcCommand
	for ( i; i < (sizeof(ApiRpcCommands) / sizeof(ApiRpcCommands[0])); i++ )
	{
		const RpcCommand*	pcmd;

		pcmd = &ApiRpcCommands[i];
		if ( pcmd->name.compare((*iter).second->name) == 0 )
		{
			// name exists in the ApiRpcCommands table, deny
			return ERpcStatus::AccessDenied;
		}
	}

	// remove the command
	_cmd_map.erase(iter);

	return ERpcStatus::Ok;
}



END_NAMESPACE

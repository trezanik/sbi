
/**
 * @file	RpcServer.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <thread>		// hardware_concurrency

#if defined(USING_JSON_SPIRIT_RPC)
#	include <boost/foreach.hpp>
#endif

#if defined(_WIN32)
#	include <Windows.h>
#endif

#include "RpcServer.h"
#include "utils.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



// special function; can't be class member as the return value is for throwing
json_spirit::Object
JSONRPCError(
	ERpcStatus err_code,
	const std::string& message
)
{
	json_spirit::Object	error;
	error.push_back(json_spirit::Pair("code", (int64_t) err_code));
	error.push_back(json_spirit::Pair("message", message));
	return error;
}





RpcServer::RpcServer()
{

}



RpcServer::~RpcServer()
{

}



json_spirit::Value
RpcServer::GetEnvironmentCoreCount(
	const json_spirit::Array& params,
	bool fHelp
)
{
	if ( fHelp || params.size() != 0 )
	{
		throw std::runtime_error(
			"GetEnvironmentCoreCount\n"
			"Returns the numer of concurrent threads (CPU cores) available"
		);
	}

	uint64_t	core_count;

	if (( core_count = std::thread::hardware_concurrency()) == 0 )
	{
#if defined(_WIN32)
		SYSTEM_INFO	si;
		GetSystemInfo(&si);
		core_count = si.dwNumberOfProcessors;
#elif defined (__linux__)
		core_count = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(BSD)
		int32_t		mib[4];
		size_t		size = sizeof(core_count);

		mib[0] = CTL_HW;
		mib[1] = HW_AVAILCPU;
		sysctl(mib, 2, &core_count, &size, nullptr, 0);
		if ( core_count < 1 )
		{
			mib[1] = HW_NCPU;
			sysctl(mib, 2, &core_count, &size, nullptr, 0);

			if ( core_count < 1 )
				core_count = 1;
		}
#else
#	error incomplete for this operating system
#endif
	}

	return core_count;
}



json_spirit::Value
RpcServer::GetInterfaceInfo(
	const json_spirit::Array& params,
	bool fHelp
)
{
	params;
	fHelp;

	/** @todo implement RPC interface info acquisition */

	return 0;
}



ERpcStatus
RpcServer::Shutdown()
{
	return ERpcStatus::Ok;
}



ERpcStatus
RpcServer::Startup()
{


	return ERpcStatus::Ok;
}



void 
RpcServer::TypeCheck(
	const json_spirit::Array& params,
	const std::list<json_spirit::Value_type>& expected_types,
	bool fAllowNull
) const
{
	uint32_t	i = 0;
	BOOST_FOREACH(json_spirit::Value_type t, expected_types)
	{
		if ( params.size() <= i )
			break;

		const json_spirit::Value& v = params[i];
		if ( !((v.type() == t) || (fAllowNull && (v.type() == json_spirit::null_type))) )
		{
			std::string	err = BUILD_STRING(
				"Expected type ",
				value_type_to_string(t),
				", got ",
				value_type_to_string(v.type())
			);
			
			throw JSONRPCError(ERpcStatus::UnknownType, err);
		}
		i++;
	}
}



void 
RpcServer::TypeCheck(
	const json_spirit::Object& o,
	const std::map<std::string, json_spirit::Value_type>& expected_types,
	bool fAllowNull
) const
{
	std::string	err;

	BOOST_FOREACH(const PAIRTYPE(std::string, json_spirit::Value_type)& t, expected_types)
	{
		const json_spirit::Value& v = find_value(o, t.first);
		if ( !fAllowNull && v.type() == json_spirit::null_type )
		{
			err = BUILD_STRING("Missing ", t.first.c_str());

			throw JSONRPCError(ERpcStatus::UnknownType, err);
		}

		if ( !((v.type() == t.second) || (fAllowNull && (v.type() == json_spirit::null_type))) )
		{
			err = BUILD_STRING(
				"Expected type ",
				value_type_to_string(t.second),
				" for ",
				t.first.c_str(),
				", got ",
				value_type_to_string(v.type())
			);
					   
			throw JSONRPCError(ERpcStatus::UnknownType, err);
		}
	}
}




END_NAMESPACE

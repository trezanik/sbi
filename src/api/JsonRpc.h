#pragma once

/**
 * @file	src/api/JsonRpc.h
 * @author	James Warren
 * @brief	Refactored bitcoin_rpc JSON RPC implementation
 */



#include <string>

#if defined(USING_JSON_SPIRIT_RPC)
#	include <json_spirit/json_spirit_utils.h>
#endif

#include "definitions.h"
#include "rpc_status.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



/**
 * Special function that would be part of the JsonRpc class, but can't be a 
 * class member as the return value is for throwing.
 *
 * @param[in] err_code ERpcStatus error code (will never be a success value..)
 * @param[in] message Associated error message
 */
json_spirit::Object
JsonRpcError(
	ERpcStatus err_code,
	const std::string& message
);



/**
 * 
 *
 * @class JsonRpc
 */
class JsonRpc
{
public:
	json_spirit::Value	id;
	std::string		method;
	json_spirit::Array	params;

	
	JsonRpc()
	{
		id = json_spirit::Value::null;
	}


	void
	ErrorReply(
		std::ostream& stream, 
		const json_spirit::Object& obj_error, 
		const json_spirit::Value& id
	);
	

	std::string
	ExecBatch(
		const json_spirit::Array& reqv
	);


	json_spirit::Object
	ExecOne(
		const json_spirit::Value& req
	);


	void
	Parse(
		const json_spirit::Value& request_val
	);


	std::string
	Reply(
		const json_spirit::Value& result,
		const json_spirit::Value& error,
		const json_spirit::Value& id
	);
	

	json_spirit::Object
	ReplyObj(
		const json_spirit::Value& result,
		const json_spirit::Value& error,
		const json_spirit::Value& id
	);


	std::string
	Request(
		const std::string& method,
		const json_spirit::Array& params,
		const json_spirit::Value& id
	);
};



END_NAMESPACE

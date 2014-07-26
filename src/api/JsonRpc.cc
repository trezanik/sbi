
/**
 * @file	src/api/JsonRpc.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#if defined(USING_JSON_SPIRIT_RPC)
#	include <json_spirit/json_spirit_writer_template.h>
#endif

#include "JsonRpc.h"
#include "RpcServer.h"		// HTTP status codes, HTTPReply
#include "Runtime.h"		// access RPC
#include "Log.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



json_spirit::Object
JsonRpcError(
	ERpcStatus err_code,
	const std::string& message
)
{
	json_spirit::Object	error;

	error.push_back(json_spirit::Pair("code", (int64_t)err_code));
	error.push_back(json_spirit::Pair("message", message));

	return error;
}



void
JsonRpc::ErrorReply(
	std::ostream& stream, 
	const json_spirit::Object& obj_error, 
	const json_spirit::Value& id
)
{
	// Send error reply from json-rpc error object
	
	uint32_t	status_code = HTTP_INTERNAL_SERVER_ERROR;
	ERpcStatus	rpcstatus = (ERpcStatus)find_value(obj_error, "code").get_int();

	if ( rpcstatus == ERpcStatus::InvalidRequest )
		status_code = HTTP_BAD_REQUEST;
	else if ( rpcstatus == ERpcStatus::MethodNotFound ) 
		status_code = HTTP_NOT_FOUND;

	std::string	reply = Reply(json_spirit::Value::null, obj_error, id);

	stream << runtime.RPC()->HTTPReply(status_code, reply, false) << std::flush;
}



std::string 
JsonRpc::ExecBatch(
	const json_spirit::Array& reqv
)
{
	json_spirit::Array	ret;

	for ( uint32_t i = 0; i < reqv.size(); i++ )
		ret.push_back(ExecOne(reqv[i]));

	return json_spirit::write_string(json_spirit::Value(ret), false) + "\n";
}



json_spirit::Object
JsonRpc::ExecOne(
	const json_spirit::Value& req
)
{
	json_spirit::Object	rpc_result;

	try
	{
		Parse(req);
	}
	catch ( json_spirit::Object& obj_error )
	{
		rpc_result = ReplyObj(json_spirit::Value::null, obj_error, id);
	}
	catch ( std::exception& e )
	{
		rpc_result = ReplyObj(json_spirit::Value::null, 
				      JsonRpcError(ERpcStatus::ParseError, e.what()), id);
	}

	return rpc_result;
}



void
JsonRpc::Parse(
	const json_spirit::Value& request_val
)
{
	// Parse request
	if ( request_val.type() != json_spirit::obj_type )
	{
		throw JsonRpcError(ERpcStatus::InvalidRequest, "Invalid Request object");
	}

	const json_spirit::Object&	request = request_val.get_obj();


	// Parse id now so errors from here on will have the id
	id = find_value(request, "id");


	// Parse method
	json_spirit::Value	methodv = find_value(request, "method");

	// validate and log method
	if ( methodv.type() == json_spirit::null_type )
	{
		throw JsonRpcError(ERpcStatus::InvalidRequest, "Missing method");
	}
	if ( methodv.type() != json_spirit::str_type )
	{
		throw JsonRpcError(ERpcStatus::InvalidRequest, "Method must be a string");
	}

	method = methodv.get_str();

	LOG(ELogLevel::Debug) << "Client executing method: " << method.c_str() << "\n";


	// Parse params
	json_spirit::Value	paramsv = find_value(request, "params");
	
	if ( paramsv.type() == json_spirit::array_type )
	{
		params = paramsv.get_array();
	}
	else if ( paramsv.type() == json_spirit::null_type )
	{
		params = json_spirit::Array();
	}
	else
	{
		throw JsonRpcError(ERpcStatus::InvalidRequest, "Params must be an array");
	}
}



std::string
JsonRpc::Reply(
	const json_spirit::Value& result,
	const json_spirit::Value& error,
	const json_spirit::Value& id
)
{
	json_spirit::Object	reply = ReplyObj(result, error, id);

	return json_spirit::write_string(json_spirit::Value(reply), false) + "\n";
}



json_spirit::Object
JsonRpc::ReplyObj(
	const json_spirit::Value& result, 
	const json_spirit::Value& error, 
	const json_spirit::Value& id
)
{
	json_spirit::Object	reply;

	if ( error.type() != json_spirit::null_type )
		reply.push_back(json_spirit::Pair("result", json_spirit::Value::null));
	else
		reply.push_back(json_spirit::Pair("result", result));

	reply.push_back(json_spirit::Pair("error", error));
	reply.push_back(json_spirit::Pair("id", id));

	return reply;
}



std::string
JsonRpc::Request(
	const std::string& method, 
	const json_spirit::Array& params, 
	const json_spirit::Value& id
)
{
	json_spirit::Object	request;

	request.push_back(json_spirit::Pair("method", method));
	request.push_back(json_spirit::Pair("params", params));
	request.push_back(json_spirit::Pair("id", id));

	return json_spirit::write_string(json_spirit::Value(request), false) + "\n";
}



END_NAMESPACE

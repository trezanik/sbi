#pragma once

/**
 * @file	rpc_status.h
 * @author	James Warren
 * @brief	RPC status codes
 */



/**
 * Status codes returned from RPC operations that return status codes. Not all
 * RPC operations require return values, much like when sending a UDP packet and
 * not concerning if it actually arrived.
 */
enum class ERpcStatus
{
	Ok,
	InvalidParameter,	// Supplied parameter invalid (e.g. outside min/max range)
	MissingParameter,	// Supplied parameter missing (i.e. nullptr)
	Exception,		// Exception in processing
	OutOfMemory,		// Memory allocation failed
	AccessDenied,		// No permissions
	UnknownType,		// Unrecognized type passed as parameter
	NameInUse,		// RpcCommand name is already used
	ThreadCreateFailed,	// Could not create the server thread
	NotListening,		// Could not listen on requested port
	// json-rpc
	InvalidRequest,
	MethodNotFound,
	InvalidParams,
	InternalError,
	ParseError,
};

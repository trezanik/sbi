#pragma once

/**
 * @file	Rpc.h
 * @author	James Warren
 * @brief	API RPC
 */





#include "definitions.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



/**
 * Status codes returned from RPC operations that return status codes. Not all
 * RPC operations require return values, much like a sending a UDP packet and 
 * moving on.
 */
enum class ERPCStatus
{
	Ok,
	InvalidParameter,
	MissingParameter,
	NotFound,
	AccessDenied
};



/**
 * RPC server class
 */
class SBI_API Rpc
{
private:
	NO_CLASS_ASSIGNMENT(Rpc);
	NO_CLASS_COPY(Rpc);

protected:
public:
	Rpc();
	~Rpc();



};






END_NAMESPACE

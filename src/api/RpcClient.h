#pragma once

/**
 * @file	RpcClient.h
 * @author	James Warren
 * @brief	RPC Client class for Interfaces to use
 */



#include <string>

#if defined(USING_JSON_SPIRIT_RPC)
#	include <json_spirit/json_spirit_utils.h>
#endif

#include "definitions.h"
#include "rpc_status.h"



class SBI_API RpcClient
{
private:
	NO_CLASS_ASSIGNMENT(RpcClient);
	NO_CLASS_COPY(RpcClient);


protected:
	

public:
	RpcClient();
	virtual ~RpcClient();


	virtual ERpcStatus
	Close();
	

	virtual ERpcStatus
	Connect(
		std::string address,
		uint16_t port
	);
};


/**
 * @file	RpcClient.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include "RpcClient.h"





RpcClient::RpcClient()
{
}



RpcClient::~RpcClient()
{
}



ERpcStatus
RpcClient::Close()
{

	return ERpcStatus::Ok;
}



ERpcStatus
RpcClient::Connect(
	std::string address,
	uint16_t port
)
{

	return ERpcStatus::Ok;
}

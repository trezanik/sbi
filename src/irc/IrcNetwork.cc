
/**
 * @file	IrcNetwork.cc
 * @author	James Warren
 * @copyright	James Warren, 2013
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#if defined(USING_BOOST_NET)
#	include <boost/asio.hpp>
#else
#	include "nethelper.h"		// DNS lookup functionality
#endif

#include <api/Runtime.h>		// IRC instance
#include <api/Terminal.h>		// console output
#include <api/utils.h>
#include "IrcNetwork.h"			// prototypes
#include "IrcEngine.h"
#include "IrcConnection.h"		// IrcConnection
#include "IrcFactory.h"
#include "IrcPool.h"




BEGIN_NAMESPACE(APP_NAMESPACE)



IrcNetwork::IrcNetwork(
	const char* group_name
)
: _group_name(group_name)
{
}



IrcNetwork::~IrcNetwork()
{
	Cleanup();
}



EIrcStatus
IrcNetwork::Cleanup()
{
	std::lock_guard<std::mutex>	lock(_mutex);

#if 0	/// @todo use updated handling
	/** @todo this is to be called when simply 'cleaning' the network, ready
	 * for another call. Consider the deletion/nulling of the connection,
	 * where to maintian state, etc., and update the below comments */
	if ( _connection != nullptr )
	{

		/* If you do not want the connection to be freed, you MUST call
		 * the cleanup function for the connection first, then set this
		 * to NULL, so it will not enter this scope on cleanup. */
		//runtime.Factory()->DeleteIrcObject(_connection);
	}
#endif

	return EIrcStatus::OK;
}



std::string
IrcNetwork::ClientHostmask() const
{
	std::lock_guard<std::mutex>	lock(_mutex);

	return _client.hostmask;
}



std::string
IrcNetwork::ClientNickname() const
{
	std::lock_guard<std::mutex>	lock(_mutex);

	return _client.nickname;
}



std::string
IrcNetwork::GetAuthService() const
{
	return _profile_config.autoident_service;
}



std::shared_ptr<IrcConnection>
IrcNetwork::GetConnection()
{
#if 0	/// @todo use updated handling
	if ( _connection == nullptr )
		return nullptr;

	return _connection;
#endif
	return nullptr;
}



std::string
IrcNetwork::GroupName() const
{
	std::lock_guard<std::mutex>	lock(_mutex);

	return _group_name;
}




std::string
IrcNetwork::Name() const
{
	std::lock_guard<std::mutex>	lock(_mutex);

	return _server.network;
}



std::string
IrcNetwork::Server() const
{
	std::lock_guard<std::mutex>	lock(_mutex);

	return _server.server;
}



std::shared_ptr<IrcConnection>
IrcNetwork::Setup(
	std::shared_ptr<config_network> network_config,
	std::shared_ptr<config_profile> profile_config
)
{
	if ( network_config == nullptr )
		goto no_network;
	if ( profile_config == nullptr )
		goto no_profile;

	/* copies are made of the xml configuration so that the physical file
	 * can be modified without affecting an active connection.
	 * Vectors are copied just by assignment, so this all looks clean! */

	_network_config.allow_invalid_cert	= network_config->allow_invalid_cert;
	_network_config.auto_connect		= network_config->auto_connect;
	_network_config.auto_exec_commands	= network_config->auto_exec_commands;
	_network_config.auto_join_channels	= network_config->auto_join_channels;
	_network_config.network_name		= network_config->network_name;
	_network_config.profile_name		= network_config->profile_name;
	_network_config.channels		= network_config->channels;
	_network_config.commands		= network_config->commands;
	_network_config.servers			= network_config->servers;

	_profile_config.auto_identify		= profile_config->auto_identify;
	_profile_config.mode			= profile_config->mode;
	_profile_config.profile_name		= profile_config->profile_name;
	_profile_config.autoident_password	= profile_config->autoident_password;
	_profile_config.autoident_service	= profile_config->autoident_service;
	_profile_config.kick_reason		= profile_config->kick_reason;
	_profile_config.part_reason		= profile_config->part_reason;
	_profile_config.quit_reason		= profile_config->quit_reason;
	_profile_config.ident			= profile_config->ident;
	_profile_config.real_name		= profile_config->real_name;
	_profile_config.nicknames		= profile_config->nicknames;

	if ( _irc_engine->CreateConnection(this->GroupName().c_str()) == EIrcStatus::OK )
	{
	}

	return nullptr;
#if 0	/// @todo use updated handling
	return _connection;
#endif

no_network:
	std::cerr << fg_red << "The supplied network configuration was a nullptr\n";
	return nullptr;
no_profile:
	std::cerr << fg_red << "The supplied profile configuration was a nullptr\n";
	return nullptr;
}



EIrcStatus
IrcNetwork::UpdateServerInfo()
{
	std::lock_guard<std::mutex>	lock(_mutex);

#if 0	/// @todo use updated handling
	std::shared_ptr<IrcConnection>	connection = _irc_engine->Pools()->GetConnection(0);

	if ( _connection == nullptr )
		goto connection_null;

	if ( !_connection->_params.host.empty() )
	{
		_server.host = _connection->_params.host;
	}
	else
	{
		_server.host.clear();
	}

	if ( !_connection->_params.ip_addr.empty() )
		_server.ip_address = _connection->_params.ip_addr;
	else
		_server.ip_address.clear();

	_server.port = _connection->_params.port;
#endif

	return EIrcStatus::OK;

connection_null:
	std::cerr << fg_red << "The connection is a nullptr\n";
	return EIrcStatus::MissingParameter;
}


END_NAMESPACE

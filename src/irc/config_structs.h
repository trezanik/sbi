#pragma once

/**
 * @file	src/irc/config_structs.h
 * @author	James Warren
 * @brief	IRC structs holding configuration
 */



#include <string>
#include <vector>
#include <memory>
#include <api/char_helper.h>



BEGIN_NAMESPACE(APP_NAMESPACE)


/**
 * Holds the configuration for an IRC profile.
 *
 * Must reside within a std::shared_ptr.
 *
 * @struct config_profile
 */
struct config_profile
{
	std::string	profile_name;
	std::string	ident;
	std::string	real_name;
	std::string	kick_reason;
	std::string	part_reason;
	std::string	quit_reason;
	bool		auto_identify;
	std::string	autoident_password;
	std::string	autoident_service;
	uint16_t	mode;			/**< usermode bitmask */
	std::vector<std::string>	nicknames;

	// deep copy; to be used within the UI when changing configuration
	config_profile operator = (config_profile* profile)
	{
		profile_name		= profile->profile_name;
		ident			= profile->ident;
		real_name		= profile->real_name;
		kick_reason		= profile->kick_reason;
		part_reason		= profile->part_reason;
		quit_reason		= profile->quit_reason;
		auto_identify		= profile->auto_identify;
		autoident_password	= profile->autoident_password;
		autoident_service	= profile->autoident_service;
		mode			= profile->mode = mode;
		for ( auto n : profile->nicknames )
			nicknames.push_back(n);
		return *this;
	}
};


/**
 * Holds the configuration for an IRC server to connect to by a connection.
 *
 * Must reside within a std::shared_ptr.
 *
 * @struct config_server
 */
struct config_server
{
	std::string	hostname;
	std::string	ip_address;
	std::string	password;
	uint16_t	port;
	bool		ssl;
};


/**
 * Holds the configuration for an IRC network. Note that the autojoin channels
 * that contain keys have it as part of the string (since this matches the send
 * format, we can use this to save complexity).
 *
 * Must reside within a std::shared_ptr.
 *
 * @struct config_network
 */
struct config_network
{
	std::string	network_name;
	std::string	profile_name;
	std::string	encoding;
	bool		allow_invalid_cert;
	bool		auto_connect;
	bool		auto_exec_commands;
	bool		auto_join_channels;
	std::vector<std::string>	channels;
	std::vector<std::string>	commands;
	// must be a shared_ptr (not unique), as we copy the entire struct over
	std::vector<std::shared_ptr<config_server>>	servers;

	// deep copy; to be used within the UI when changing configuration
	config_network operator = (config_network* network)
	{
		network_name		= network->network_name;
		profile_name		= network->profile_name;
		encoding		= network->encoding;
		allow_invalid_cert	= network->allow_invalid_cert;
		auto_connect		= network->auto_connect;
		auto_exec_commands	= network->auto_exec_commands;
		auto_join_channels	= network->auto_join_channels;
		for ( auto c : network->channels )
			channels.push_back(c);
		for ( auto c : network->commands )
			commands.push_back(c);
		for ( auto c : network->servers )
			servers.push_back(c);
		return *this;
	}
};


END_NAMESPACE

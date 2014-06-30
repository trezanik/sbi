#pragma once

/**
 * @file	IrcNetwork.h
 * @author	James Warren
 * @brief	The class for an entire IRC Network
 */



#include <mutex>
#include <api/char_helper.h>
#include "IrcObject.h"
#include "irc_status.h"
#include "config_structs.h"
#include "live_structs.h"



BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class IrcConnection;



/**
 * 
 *
 * @todo Think of a method proxy-style to prevent the need for direct variable
 * access via IrcConnection. Our existing proxy class doesn't have the ability
 * to handle structs (is more designed for single variables). May be worth just
 * having a base class where you insert a request, and the derived class returns
 * the result. Or, simply leave it as friend and save much work at the cost of
 * some imperfect design...
 *
 * @class IrcNetwork
 */
class SBI_IRC_API IrcNetwork : public IrcObject
{
	// Updates internals directly when parsing relevant data
	friend class IrcParser;
	// reads + updates internals
	friend class IrcConnection;
private:
	NO_CLASS_ASSIGNMENT(IrcNetwork);
	NO_CLASS_COPY(IrcNetwork);
	
	std::vector<uint32_t>	_cids;		/**< Connections owned by this network */

	config_network		_network_config;/**< Copied network configuration */
	config_profile		_profile_config;/**< Copied profile configuration */
	irc_client		_client;	/**< The current client configuration */
	irc_server		_server;	/**< The current server data */
	
	std::string		_name;		/**< Network name, as returned by the server */

	std::string		_group_name;	/**< A name for this network for identification */

	mutable std::mutex	_mutex;		/**< Synchronization lock; mutable to enable constness for retrieval functions */

	
public:

#if 0	// Code Removed: todo, need a working method
	struct {
		Proxy<std::string, IrcNetwork>	hostmask;
		Proxy<std::string, IrcNetwork>	nickname;
	} read_only;
#endif


	IrcNetwork(
		const char* group_name
	);
	~IrcNetwork();
	

	/**
	 *
	 *
	 */
	EIrcStatus
	Cleanup();


	/**
	 * Gets the current hostmask we're using.
	 *
	 * @return A copy of the current hostmask; this is obtained from the
	 * client config
	 */
	std::string
	ClientHostmask() const;


	/**
	 * Gets the current nickname we're using.
	 *
	 * @return A copy of the current nickname; this is obtained from the
	 * client config
	 */
	std::string
	ClientNickname() const;


	/**
	 * Retrieves a copy of the name of the network service used to 
	 * authenticate users entering their passwords.
	 *
	 * Stored in the config; is it worth just returning a copy of the entire
	 * config structs rather than using functions for each member as needed?
	 *
	 * @return A copy of the network auth service is returned, or an empty
	 * object if it's not set yet
	 */
	std::string
	GetAuthService() const;


	/**
	 * Retrieves this networks connection.
	 *
	 * @return A pointer to the connection is returned. If Setup() has not
	 * been called successfully, this will be a nullptr; otherwise, a valid
	 * pointer will be returned.
	 */
	std::shared_ptr<IrcConnection>
	GetConnection();


	/**
	 * Retrieves a copy of the name of the network as assigned by the user
	 * (this is the name that appears in the Connections dialog, with the
	 * servers as child tree items).
	 *
	 * @return A copy of the name is returned, which is assigned in the
	 * constructor
	 */
	std::string
	GroupName() const;


	/**
	 * Retrieves a copy of the name of the network, e.g. "Rizon"
	 *
	 * Set in the 005 received from the server, in 'NETWORK=xxx'.
	 *
	 * @return A copy of the network name is returned, or an empty object if
	 * it's not set yet
	 */
	std::string
	Name() const;


	/**
	 * Retrieves a copy of the networks hostname, *set by itself*
	 *
	 * This is the string contained as the prefix of all messages (i.e. 
	 * ':ircd.trezanik.org CAP * ACK :multi-prefix'). 
	 *
	 * For example, while the connection server parameters set 
	 * 'irc.rizon.net' as the host we connected to, this parameter would 
	 * show 'irc.shakeababy.net' if it was the actual server we ended at.
	 *
	 * @return A copy of the network hostname is returned, or an empty 
	 * object if it's not set yet
	 */
	std::string
	Server() const;


#if 0	// Code Removed: IrcParser has private access, this is no longer needed
	/**
	 * Sets the network name. Can only be called once; once the variable is
	 * populated, it'll be unwritable.
	 *
	 * @param name The name for the network
	 * @return Returns true on success, otherwise false
	 */
	bool
	SetNetworkName(
		const c8 *name
	);
#endif


	/**
	 * Sets up and creates the connection to use for this network.
	 *
	 * @param[in] network_config A pointer to a valid config_network struct
	 * @param[in] profile_config A pointer to a valid config_profile struct
	 * @return Returns a pre-referenced IrcConnection on success, or a
	 * nullptr on failure.
	 */
	std::shared_ptr<IrcConnection>
	Setup(
		std::shared_ptr<config_network> network_config,
		std::shared_ptr<config_profile> profile_config
	);


	/**
	 * Checks the contents of the connections connection parameters, and if
	 * the servers hostname/ip address are available, copies the data into
	 * the irc_server struct.
	 * The other fields are not touched, as they are read only when we are
	 * negotiating with the IRC server; the data is not available before
	 * then.
	 *
	 * @return Returns the relevant EIrcStatus on error
	 * @return Returns EIrcStatus::OK when the irc_server struct is in sync 
	 * with that of the connections irc_connection_params
	 */
	EIrcStatus
	UpdateServerInfo();
};


END_NAMESPACE

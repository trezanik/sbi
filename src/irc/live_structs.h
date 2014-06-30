#pragma once

/**
 * @file	live_structs.h
 * @author	James Warren
 * @brief	IRC structs holding live data, or data used for processing
 */



#include <string>
#include <vector>
#include <api/char_helper.h>


BEGIN_NAMESPACE(APP_NAMESPACE)


/**
 * The client parameters used for an individual connection. Pretty much the
 * same as the configuration struct, but this is used for the 'live' settings
 * and also on reconnection.
 *
 * This enables you to retain your saved, preferred settings, but for this one
 * instance you want a slight change. On reconnect, it will retain your small
 * changes, and not reload the defaults from the file.
 *
 * @struct irc_client
 */
struct irc_client
{
	std::string	autoauth_service;	/**< The service receiving user authentication */
	std::string	autoauth_password;	/**< The password to the receiving service */
	std::string	hostmask;		/**< The custom hostmask to use, if any */
	std::string	nickname;		/**< The current nickname to use */
	std::string	kick_reason;		/**< The default kick reason, if none other is supplied */
	std::string	part_reason;		/**< The default part reason, if none other is supplied */
	std::string	quit_reason;		/**< The default quit reason, if none other is supplied */
	std::vector<std::string>	nicknames;	/**< The list of nicknames available to use */
};


/**
 * The structure containing the dynamic server 'options' and data, as received
 *
 * @struct irc_server
 */
struct irc_server
{
	std::string	host;			/**< the hostname reported by the server */
	std::string	ip_address;		/**< the IP address of the server */
	uint16_t	port;			/**< the port of the server connection */
	std::string	chan_mode_chars;	/**< server character modes - +a, +v, +h, etc. */
	std::string	chan_mode_symbols;	/**< server symbol modes - &, @, +, etc. */
	std::string	chan_types;		/**< channel prefixes */
	std::string	supported_modes_A;	/**< server supported 'A' modes */
	std::string	supported_modes_B;	/**< server supported 'B' modes */
	std::string	supported_modes_C;	/**< server supported 'C' modes */
	std::string	supported_modes_D;	/**< server supported 'D' modes */
	std::string	network;		/**< server reported network name */
	std::string	server;			/**< the server within the network, as reported */
	uint16_t	max_len_away;		/**< maximum length of an away message */
	uint16_t	max_len_channel;	/**< maximum length of a channel name */
	uint16_t	max_len_kickmsg;	/**< maximum length of a kick message */
	uint16_t	max_len_nick;		/**< maximum length of a clients nickname */
	uint16_t	max_len_topic;		/**< maximum length of a channel topic  */
	uint16_t	max_num_modes;		/**< maximum number of modes in a single MODE command */

	/** @todo different channel prefixes have different limits */
	uint16_t	max_num_channels;	/**< The channel limit */
};


END_NAMESPACE

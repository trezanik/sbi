#pragma once

/**
 * @file	irc_structs.h
 * @author	James Warren
 * @brief	IRC structs holding extracted, or in process, data
 */


// required inclusions

#include <string>
#include <vector>

#include <api/char_helper.h>



BEGIN_NAMESPACE(APP_NAMESPACE)



/**
 * Contains a variable for every possible IRC message code in one class. Not all
 * members are valid at a time; it all depends on the code identified and then
 * processed by the parser.
 *
 * For example; receiving a private message structured as follows:
 *
 @verbatim
 :more!qwebirc@4FDB4E94.IP PRIVMSG #chan_name :i manually activated your account
 @endverbatim
 *
 * results in the following populated members:
 *
 * variable             | value
 * -------------------: | :----------------:
 * instigator.nickname  | more
 * instigator.ident     | qwebirc
 * instigator.hostmask  | 4FDB4E94.IP
 * channel_name         | \#chan_name
 * message              | i manually activated your account
 *
 * All the other members are left in an unset state and should not be used. The
 * way the application is structured means that the activity can remain within
 * an IrcConnection, with a reference to the object being returned to whatever
 * listener is active at the time. You have the until your own OnXxx function
 * returns to extract any data; then when control has returned from the 
 * notification handlers, the parser reactivates, going through the
 * next data in the queue (which then causes new activity details to be set).
 * Explaining this isn't easy, so it's best to check the flow diagrams!
 *
 * The alternative to this is having a struct for each code type, where they are
 * then populated in the same way. This may be easier to understand and document
 * in hindsight, and can reside within a union to save on memory. The major 
 * drawback is having SO many extra structs (1 for all 999 codes, plus the non
 * numerics - you want to do that?).
 *
 * @struct irc_activity
 */
struct irc_activity
{
	/** The channel name this activity resides within */
	std::string			channel_name;

	/** The details of the sender, or instigator of the action (a duplicate
	 * of ircbuf_sender, but we are trying to limit header inclusions) */
	struct {
		std::string		nickname;
		std::string		ident;
		std::string		hostmask;
	} instigator;

	/** A single affected nickname (more than one will be vectorized) */
	std::string			nickname;

	/** Holds NOTICE, PRIVMSG, KICK, KILL, PART, QUIT, etc. messages */
	std::string			message;
	
	/** Additional data ( CAP's ACK/NAK, ... ) */
	std::string			data;

	/** User, channel, server modes */
	std::vector<std::string>	modes;
};


/**
 * Used to store the three 'components' of an IRC data message; the sender,
 * the IRC code, and the data associated with it. Key to the parsers work.
 *
 * @struct ircbuf_data
 */
struct ircbuf_data
{
	std::string	sender;		/**< The sender of the data */
	std::string	code;		/**< The IRC message code of the data */
	std::string	data;		/**< The actual data itself, of the message */
};


/**
 * Used to store the three 'components' of an IRC sender - the nickname, ident
 * and hostmask. Frequently the sender is the server itself, which has no
 * ident or 'nickname'. We interpret it as the nickname however, to maintain
 * consistency.
 *
 * @struct ircbuf_sender
 */
struct ircbuf_sender
{
	std::string	nickname;	/**< The senders nickname/entire server string */
	std::string	ident;		/**< The users ident (empty if server) */
	std::string	hostmask;	/**< The users hostmask (empty if server) */
};


/**
 * Additional structure required to parse modes. Each mode received in a message
 * is split into multiple structures, and then sent through processing. Doing
 * it this way allows plugins to get any data they're monitoring with minimal
 * hassle and virtually no work to do.
 *
 * @struct mode_data
 */
struct mode_data
{
	bool		is_enabled;	/**< enabling mode (+) or disabling (-) */
	bool		has_data;	/**< whether there should be data with this mode */
	char		mode;		/**< the mode itself (m, v, q, o, etc.) */
	std::string	data;		/**< data associated with the mode - can be a nick, hostmask, etc. */
};


/**
 * The struct used for dynamically updating channel and user modes on receipt
 *
 * @struct mode_update
 */
struct mode_update
{
	bool		erase_existing;	/**< If true, erase the existing modes */
	uint16_t	to_add;		/**< Modes to add */
	uint16_t	to_remove;	/**< Modes to remove */
};


/**
 *
 * @struct user_modes_change
 */
struct user_modes_change
{
	uint16_t		before;
	uint16_t		after;
	const mode_update*	mu;
};


END_NAMESPACE

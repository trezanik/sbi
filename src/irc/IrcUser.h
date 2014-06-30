#pragma once

/**
 * @file	IrcUser.h
 * @author	James Warren
 * @brief	Functionality for creating and working with IrcUser objects
 */



#include <mutex>
#include <memory>
#include <api/char_helper.h>
#include "IrcObject.h"
#include "irc_status.h"



BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class IrcChannel;
struct mode_update;



/**
 * An IRC user, used within all IRC channels
 *
 * @class IrcUser
 */
class SBI_IRC_API IrcUser : public IrcObject
{
	// Updates internals directly when parsing relevant data
	friend class IrcParser;
private:
	NO_CLASS_ASSIGNMENT(IrcUser);
	NO_CLASS_COPY(IrcUser);

	uint16_t		_flags;		/**< User flags */
	uint16_t		_modes;		/**< The users modes in the channel */
	std::string		_nickname;	/**< The users nickname on the server */
	std::string		_ident;		/**< The users ident */
	std::string		_hostmask;	/**< The users hostmask */

	/** The channel owning this user. Being weak enables the channel to be
	 * deleted while this/other users are still being accessed. */
	std::weak_ptr<IrcChannel>	_owner;

	/** Synchronization lock; mutable to enable constness for retrieval functions */
	mutable std::recursive_mutex	_mutex;		

public:

	/**
	 * Since we're created by a factory, our debugging macros get in the
	 * way of being able to use the default nullptr operations (needed only
	 * on servers that don't support supplying the extra host info at the
	 * same time). Have retained their default nullptr states for reference.
	 */
	IrcUser(
		std::shared_ptr<IrcChannel> channel,
		const char* nickname,
		const char* ident = nullptr,
		const char* hostmask = nullptr
	);
	~IrcUser();


	/**
	 * Frees the memory, if allocated, for the class members.
	 */
	EIrcStatus
	Cleanup();


	/**
	 * 
	 */
	std::string
	Hostmask() const;
	

	/**
	 * 
	 */
	std::string
	Ident() const;


	/**
	 * 
	 */
	std::string
	Nickname() const;


	/**
	 * 
	 */
	std::shared_ptr<IrcChannel>
	Owner();


	/**
	 * Updates the specified user object with the supplied details; NULL values
	 * will not replace any existing data.
	 *
	 * @param new_nickname (Optional) The new nickname to set
	 * @param new_ident (Optional) The new ident to set
	 * @param new_hostmask (Optional) The new hostmask to set
	 * @param new_modes (Optional) The new user modes to set
	 * @return Returns true if the specified user is updated successfully with the
	 * new details, otherwise false
	 */
	EIrcStatus
	Update(
		const char* new_nickname,
		const char* new_ident,
		const char* new_hostmask,
		const mode_update* new_modes
	);
};



END_NAMESPACE

#pragma once

/**
 * @file	IrcChannel.h
 * @author	James Warren
 * @brief	Functionality for creating and maintaining an IRC channel
 */



#include <set>		// std::set
#include <memory>	// std::unique_ptr,std::shared_ptr
#include <mutex>	// std::mutex

#include <api/char_helper.h>
#include "IrcObject.h"
#include "irc_status.h"



BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class IrcConnection;
class IrcUser;
struct mode_update;



/**
 *
 *
 * @class IrcChannel
 */
class SBI_IRC_API IrcChannel : public IrcObject
{
	// Updates internals directly when parsing relevant data
	friend class IrcParser;
private:
	NO_CLASS_ASSIGNMENT(IrcChannel);
	NO_CLASS_COPY(IrcChannel);

	uint32_t		_flags;		/**< Channel flags */
	uint16_t		_limit;		/**< User limit (max 65535 users in a channel seems fair/accurate?) */
	std::string		_key;		/**< Channel password, aka 'Key' */
	std::string		_name;		/**< Channel name, including any prefixes */
	std::string		_topic;		/**< Channel topic (will contain colour codes) */
	std::weak_ptr<IrcConnection>	_owner;	/**< Pointer to the owning connection */

	/** Active channel userlist */
	std::set<std::string>	_userlist;
	/** Channel userlist for receiving 353 NAMES entries */
	std::set<IrcUser*>	_nameslist;

	/** Synchronization lock; mutable to enable constness for retrieval functions */
	mutable std::recursive_mutex	_mutex;


	/**
	 * Deletes all the IrcUser objects in the names list.
	 *
	 * The protection mutex is recursive, so we can call this safely from
	 * other class functions so long as they're on the same thread.
	 */
	EIrcStatus
	EraseNameslist();


	/**
	 * Deletes all the IrcUser objects in the active userlist.
	 *
	 * The protection mutex is recursive, so we can call this safely from
	 * other class functions so long as they're on the same thread.
	 */
	EIrcStatus
	EraseUserlist();

public:

	/**
	 * Constructs the channel. This is the only opportunity to set the name
	 * of it (why would you change it afterwards?).
	 *
	 * Should only be created (and therefore called) from the IrcParser.
	 *
	 * @param[in] connection The IrcConnection this channel was created for
	 * @param[in] channel_name The name of the channel
	 * for
	 */
	IrcChannel(
		std::shared_ptr<IrcConnection> connection,
		const char* channel_name
	);
	~IrcChannel();


	/**
	 * Adds the supplied user details into a new IrcUser class which is
	 * added to the receiving 353 NAMES list.
	 *
	 * Identical to that of AddUser excepting the list the user is
	 * added to, and not calling a UI update
	 *
	 * @param name The users nickname
	 * @param ident The users ident
	 * @param hostmask The users hostmask
	 * @param modes The users channel modes
	 * @return true if the user is created and added to the list
	 * @return false on an invalid parameter or addition failure
	 */
	EIrcStatus
	AddNamesUser(
		const char* name,
		const char* ident,
		const char* hostmask,
		const mode_update *modes
	);


	/**
	 *
	 */
	EIrcStatus
	Cleanup();


	/**
	 * Unlinks the supplied user from the userlist, if not already done
	 * (usually via ReleaseUser), and decrements its reference count.
	 *
	 * Note this function does not get called on the userlist or names list
	 * cleanups, as they unlink and release the user directly.
	 *
	 * @param[in] user The user to delete
	 * @return Returns true if the user is unlinked and released
	 * @returns false is returned if the user is a nullptr, or an integer
	 * underflow is triggered on the linked list.
	 */
	EIrcStatus
	DeleteUser(
		std::shared_ptr<IrcUser> user
	);


	/**
	 * Gets the IrcUser object from the specified connection, whose name
	 * matches nickname.
	 *
	 * Will not generate an error if the specified user is not found, as
	 * this can be used for a simple search; raise your own error if this is
	 * undesired.
	 *
	 * @warning Increments the reference counter for the user; you must
	 * call Dereference on it when finished using it, or pass it to a
	 * function that does!
	 *
	 * @param[in] nickname The name of the user to locate
	 * @return If the user is not found, a nullptr is returned.
	 * @return A pointer to the IrcUser called nickname.
	 */
	std::shared_ptr<IrcUser>
	GetUser(
		const char* nickname
	);


	/**
	 *
	 */
	bool
	IsActive() const;


	/**
	 * Looks up the IrcUser with a nickname of nickname, and checks if it
	 * is 'authorized' - its mode equal to or greater than that of mode.
	 *
	 * @param nickname The nickname to lookup and check
	 * @param mode The mode to check
	 * @return true if the nickname was found and has sufficient
	 * access.
	 * @return false if the nickname isn't found, mode does not exist, or
	 * the user simply doesn't have the specified mode.
	 */
	bool
	IsAuthorized(
		const char* nickname,
		const uint8_t mode
	) const;


	/**
	 * Checks if the IrcUser object user has its mode equal to or greater
	 * than that of mode.
	 *
	 * @param user The IrcUser object to check
	 * @param mode The mode to check
	 * @return true if the user object has sufficient access.
	 * @return false if the mode does not exist, or the user simply doesn't
	 * have the specified mode.
	 */
	bool
	IsAuthorized(
		const std::shared_ptr<IrcUser> user,
		const uint8_t mode
	) const;


	/**
	 * Retrieves a copy.
	 */
	std::string
	Name() const;


	/**
	 *
	 */
	uint32_t
	NumberOfUsers() const
	{
		return _userlist.size();
	}


	/**
	 *
	 */
	std::shared_ptr<IrcConnection>
	Owner();


	/**
	 *
	 */
	EIrcStatus
	PopulateUserlist();


	/**
	 *
	 */
	std::string
	Topic() const;


	/**
	 * Sets the channel topic
	 *
	 * @param topic The new topic, complete with colour codes, special chars
	 */
	EIrcStatus
	UpdateTopic(
		const char* topic
	);


	/**
	 * should probably return the pool addresses rather than the strings..
	 */
	std::set<std::string>&
	Users()
	{
		return _userlist;
	}
};


END_NAMESPACE


/**
 * @file	IrcChannel.cc
 * @author	James Warren
 * @copyright	James Warren, 2013
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 */



#include <assert.h>			// assertions
#include <algorithm>			// std::find
#include <string.h>			// strcmp

#include "IrcChannel.h"			// prototypes
#include "IrcUser.h"			// IrcUser
#include "IrcConnection.h"		// IrcConnection
#include "IrcEngine.h"			// IrcEngine
#include "IrcFactory.h"
#include "IrcPool.h"
#include "irc_channel_modes.h"		// channel flags
#include <api/Runtime.h>		// IRC instance
#include <api/Terminal.h>		// console output
#include <api/utils.h>			// string functions




BEGIN_NAMESPACE(APP_NAMESPACE)



IrcChannel::IrcChannel(
	std::shared_ptr<IrcConnection> connection,
	const char* channel_name
) : _flags(0), _limit(0), _owner(connection)
{
	// userlist, nameslist, etc., start empty

	_name = channel_name;
}



IrcChannel::~IrcChannel()
{
	Cleanup();
}



EIrcStatus
IrcChannel::AddNamesUser(
	const char* name,
	const char* ident,
	const char* hostmask,
	const mode_update* modes
)
{
	IrcUser*	user;

	if ( name == nullptr )
		goto no_name;
	if ( ident == nullptr )
		goto no_ident;
	if ( hostmask == nullptr )
		goto no_hostmask;

	{
		std::lock_guard<std::recursive_mutex>	lock(_mutex);

		_irc_engine->CreateUser(Owner()->Id(), Name().c_str(), name, ident, hostmask);

		if ( modes != nullptr )
		{
			// since we have the modes, update now
			/** @todo we must have a better method than this, this 
			 * is awful. could always get CreateXx to return object,
			 * shouldn't affect lifetimes */
			_irc_engine->Pools()->GetUser(Owner()->Id(), Name().c_str(), name)->Update(nullptr, nullptr, nullptr, modes);
		}

		_nameslist.insert(user);

		// no notification, this is a temporary holder
	}

	return EIrcStatus::OK;

no_name:
	std::cerr << fg_red << "The supplied nickname was a nullptr\n";
	return EIrcStatus::MissingParameter;
no_ident:
	std::cerr << fg_red << "The supplied ident was a nullptr\n";
	return EIrcStatus::MissingParameter;
no_hostmask:
	std::cerr << fg_red << "The supplied hostmask was a nullptr\n";
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcChannel::Cleanup()
{
	EIrcStatus	retval;

	std::lock_guard<std::recursive_mutex>	lock(_mutex);

	if (( retval = EraseNameslist()) != EIrcStatus::OK )
		return retval;
	if (( retval = EraseUserlist()) != EIrcStatus::OK )
		return retval;

	return EIrcStatus::OK;
}



EIrcStatus
IrcChannel::DeleteUser(
	std::shared_ptr<IrcUser> user
)
{
	if ( user == nullptr )
		goto no_user;

	return _irc_engine->Pools()->IrcUsers()->FreeObject(user) ?
		EIrcStatus::OK : EIrcStatus::ObjectFreeError;
	
no_user:
	std::cerr << fg_red << "The supplied user was a nullptr\n";
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcChannel::EraseNameslist()
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);

	for ( auto u : _nameslist )
	{
		_irc_engine->Pools()->IrcUsers()->FreeObject(
			GetUser(u->Nickname().c_str())
		);
	}
	_nameslist.clear();

	return EIrcStatus::OK;
}



EIrcStatus
IrcChannel::EraseUserlist()
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);

	_userlist.clear();

	return EIrcStatus::OK;
}



std::shared_ptr<IrcUser>
IrcChannel::GetUser(
	const char* nickname
)
{
	return _irc_engine->Pools()->GetUser(Owner()->Id(), _name.c_str(), nickname);
}



bool
IrcChannel::IsActive() const
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	return (_flags & CHANFLAG_ACTIVE);
}



std::string
IrcChannel::Name() const
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	return _name;
}



std::shared_ptr<IrcConnection>
IrcChannel::Owner()
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);

	return _owner.lock();
}



EIrcStatus
IrcChannel::PopulateUserlist()
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);

	// wipe out the existing userlist
	EraseUserlist();
	// replace it with the pre-built names list
	for ( auto n : _nameslist )
	{
		_userlist.insert(_userlist.begin(), n->Nickname());
	}

	return EIrcStatus::OK;
}



std::string
IrcChannel::Topic() const
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	return _topic;
}



EIrcStatus
IrcChannel::UpdateTopic(
	const char* topic
)
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);

	// we allow setting a null topic
	_topic = topic;

	return EIrcStatus::OK;
}



END_NAMESPACE

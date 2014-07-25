
/**
 * @file	src/irc/IrcUser.cc
 * @author	James Warren
 * @copyright	James Warren, 2013
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <cassert>			// assertions

#include <api/utils.h>
#include "IrcUser.h"			// prototypes
#include "irc_structs.h"		// mode_update struct
#include "irc_user_modes.h"		// IRC user modes
#include "IrcChannel.h"			// our owner



BEGIN_NAMESPACE(APP_NAMESPACE)


IrcUser::IrcUser(
	std::shared_ptr<IrcChannel> channel,
	const char* nickname,
	const char* ident,
	const char* hostmask
) : _flags(0), _modes(0), _owner(channel)
{
	Update(nickname, ident, hostmask, nullptr);
}



IrcUser::~IrcUser()
{
	Cleanup();
}



EIrcStatus
IrcUser::Cleanup()
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);

	/* don't really need to do this manually, but it makes me feel all warm
	 * and fuzzy inside */
	_nickname.clear();
	_ident.clear();
	_hostmask.clear();

	return EIrcStatus::OK;
}



std::string
IrcUser::Hostmask() const
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	return _hostmask;
}



std::string
IrcUser::Ident() const
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	return _ident;
}



std::string
IrcUser::Nickname() const
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	return _nickname;
}



std::shared_ptr<IrcChannel>
IrcUser::Owner()
{
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	return _owner.lock();
}



EIrcStatus
IrcUser::Update(
	const char* new_nickname,
	const char* new_ident,
	const char* new_hostmask,
	const mode_update* new_modes
)
{
	/* just use one lock rather than 4 separate ones; these will all finish
	 * in an infathomable amount of time anyway */
	std::lock_guard<std::recursive_mutex>	lock(_mutex);

	if ( new_nickname != nullptr )
		_nickname = new_nickname;
	
	if ( new_ident != nullptr )
		_ident = new_ident;
	
	if ( new_hostmask != nullptr )
		_hostmask = new_hostmask;
	
	if ( new_modes != nullptr )
	{
		if ( new_modes->erase_existing )
			_modes = UM_None;
		if ( new_modes->to_add != UM_None )
			_modes |= new_modes->to_add;
		if ( new_modes->to_remove != UM_None )
			_modes &= ~new_modes->to_remove;
	}

	return EIrcStatus::OK;
}


END_NAMESPACE

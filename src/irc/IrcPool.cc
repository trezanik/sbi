
/**
 * @file	src/irc/Pool.cc
 * @author	James Warren
 * @copyright	James Warren, 2013-2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */


#include "IrcPool.h"		// prototypes


BEGIN_NAMESPACE(APP_NAMESPACE)


IrcPool::IrcPool()
{
}



IrcPool::~IrcPool()
{
}



std::shared_ptr<IrcChannel>
IrcPool::GetChannel(
	const uint32_t connection_id,
	const char* channel_name
)
{
	if ( connection_id == 0 )
		return nullptr;
	if ( channel_name == nullptr )
		return nullptr;

	for ( auto c : _channels.Allocated() )
	{
		if ( c->Name().compare(channel_name) == 0 )
		{
			if ( c->Owner()->Id() == connection_id )
			{
				return c;
			}
		}
	}

	return nullptr;
}



std::shared_ptr<IrcConnection>
IrcPool::GetConnection(
	const uint32_t connection_id
)
{
	if ( connection_id == 0 )
		return nullptr;

	for ( auto c : _connections.Allocated() )
	{
		if ( c->Id() == connection_id )
		{
			return c;
		}
	}

	return nullptr;
}



std::shared_ptr<IrcNetwork>
IrcPool::GetNetwork(
	const char* name
)
{
	if ( name == nullptr )
		return nullptr;

	for ( auto n : _networks.Allocated() )
	{
		if ( n->Name().compare(name) == 0 )
		{
			return n;
		}
	}

	return nullptr;
}



std::shared_ptr<IrcUser>
IrcPool::GetUser(
	const uint32_t connection_id,
	const char* channel_name,
	const char* nickname
)
{
	if ( connection_id == 0 )
		return nullptr;
	if ( channel_name == nullptr )
		return nullptr;
	if ( nickname == nullptr )
		return nullptr;

	for ( auto u : _users.Allocated() )
	{
		if ( u->Nickname().compare(nickname) == 0 )
		{
			if ( u->Owner()->Name().compare(channel_name) == 0 )
			{
				if ( u->Owner()->Owner()->Id() == connection_id )
				{
					return u;
				}
			}
		}
	}

	return nullptr;
}



END_NAMESPACE

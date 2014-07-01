
/**
 * @file	IrcEngine.cc
 * @author	James Warren
 * @copyright	James Warren, 2013
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */


#include <iostream>			// std::cerr
#include <algorithm>			// std::find
#include <cassert>

#include <api/Terminal.h>
#include "IrcEngine.h"			// prototypes
#include "IrcListener.h"
#include "IrcConnection.h"
#include "IrcNetwork.h"
#include "IrcParser.h"
#include "IrcPool.h"			// object pool
#include "irc_structs.h"		// irc_activity (reference in IrcListener.h)




BEGIN_NAMESPACE(APP_NAMESPACE)


IrcEngine::IrcEngine()
{
	_ircobject_factory.reset(new IrcFactory(this));
}



IrcEngine::~IrcEngine()
{
	_ircobject_factory.release();
}



void
IrcEngine::AttachListener(
	IrcListener* listener
)
{
	assert(listener != nullptr);

	_listeners.insert(listener);
}



EIrcStatus
IrcEngine::CreateChannel(
	const uint32_t connection_id,
	const char* name
)
{
	return Factory()->CreateIrcChannel(
		Pools()->GetConnection(connection_id),
		name
	) == nullptr ? EIrcStatus::ObjectAddError : EIrcStatus::OK;
}



EIrcStatus
IrcEngine::CreateConnection(
	const char* network
)
{
	return Factory()->CreateIrcConnection(
		Pools()->GetNetwork(network)
	) == nullptr ? EIrcStatus::ObjectAddError : EIrcStatus::OK;
}



EIrcStatus
IrcEngine::CreateNetwork(
	const char* name
)
{
	return Factory()->CreateIrcNetwork(
		name
	) == nullptr ? EIrcStatus::ObjectAddError : EIrcStatus::OK;
}



EIrcStatus
IrcEngine::CreateUser(
	const uint32_t connection_id,
	const char* channel,
	const char* name,
	const char* ident,
	const char* hostmask
)
{
	return Factory()->CreateIrcUser(
		Pools()->GetChannel(connection_id, channel), 
		name, ident, hostmask
	) == nullptr ? EIrcStatus::ObjectAddError : EIrcStatus::OK;
}



void
IrcEngine::DetachListener(
	IrcListener* listener
)
{
	assert(listener != nullptr);

	_listeners.erase(_listeners.find(listener));
}



IrcFactory*
IrcEngine::Factory() const
{
	return _ircobject_factory.get();
}



void
IrcEngine::NotifyListeners(
	E_IRC_LISTENER_NOTIFICATION event_type,
	std::shared_ptr<IrcConnection> connection
) const
{
	for ( auto l : _listeners )
	{
		switch ( event_type )
		{
		case LN_NewData:	l->OnData(connection); break;
		case LN_001:		l->On001(connection, connection->GetActivity()); break;
		case LN_002:		l->On002(connection, connection->GetActivity()); break;
		case LN_003:		l->On003(connection, connection->GetActivity()); break;
		case LN_004:		l->On004(connection, connection->GetActivity()); break;
		case LN_005:		l->On005(connection, connection->GetActivity()); break;
		case LN_331:		l->On331(connection, connection->GetActivity()); break;
		case LN_332:		l->On332(connection, connection->GetActivity()); break;
		case LN_353:		l->On353(connection, connection->GetActivity()); break;
		case LN_366:		l->On366(connection, connection->GetActivity()); break;
		case LN_ConnectionReady:	/* what to do? */break;
		case LN_Cap:		l->OnCap(connection, connection->GetActivity()); break;
		case LN_Invite:		l->OnInvite(connection, connection->GetActivity()); break;
		case LN_Join:		l->OnJoin(connection, connection->GetActivity()); break;
		case LN_Kick:		l->OnKick(connection, connection->GetActivity()); break;
		case LN_Kill:		l->OnKill(connection, connection->GetActivity()); break;
		case LN_Mode:		l->OnMode(connection, connection->GetActivity()); break;
		case LN_Nick:		l->OnNick(connection, connection->GetActivity()); break;
		case LN_Notice:		l->OnNotice(connection, connection->GetActivity()); break;
		case LN_Part:		l->OnPart(connection, connection->GetActivity()); break;
		case LN_Privmsg:	l->OnPrivmsg(connection, connection->GetActivity()); break;
		case LN_Quit:		l->OnQuit(connection, connection->GetActivity()); break;
		case LN_Topic:		l->OnTopic(connection, connection->GetActivity()); break;
		case LN_SentInvite:	l->OnOurInvite(connection, connection->GetActivity()); break;
		case LN_WeJoined:	l->OnOurJoin(connection, connection->GetActivity()); break;
		case LN_WeKicked:	l->OnOurKick(connection, connection->GetActivity()); break;
		case LN_GotKicked:	l->OnOurKicked(connection, connection->GetActivity()); break;
		case LN_GotNickChanged:	l->OnOurNick(connection, connection->GetActivity()); break;
		case LN_SentPrivmsg:	l->OnOurPrivmsg(connection, connection->GetActivity()); break;
		case LN_GotUserMode:	l->OnOurServerMode(connection, connection->GetActivity()); break;
		case LN_GotChannelMode:	l->OnOurMode(connection, connection->GetActivity()); break;
		case LN_GotKilled:	l->OnOurKilled(connection, connection->GetActivity()); break;
		case LN_WeParted:	l->OnOurPart(connection, connection->GetActivity()); break;
		case LN_WeQuit:		l->OnOurQuit(connection, connection->GetActivity()); break;
		default:
			std::cerr << fg_red << "Unhandled event type received (" << event_type << ")";
			break;
		}
	}
}



IrcParser*
IrcEngine::Parser() const
{
	static IrcParser	parser;
	return &parser;
}




IrcPool*
IrcEngine::Pools() const
{
	static IrcPool	pools;
	return &pools;
}



END_NAMESPACE

 
/**
 * @file	src/irc/IrcFactory.cc
 * @author	James Warren
 * @copyright	James Warren, 2013
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <api/Runtime.h>
#include <api/Log.h>
#include <api/Terminal.h>
#include "IrcFactory.h"			// prototypes
#include "IrcObject.h"
#include "IrcChannel.h"
#include "IrcConnection.h"
#include "IrcNetwork.h"
#include "IrcUser.h"
#include "IrcEngine.h"
#include "IrcPool.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



IrcFactory::IrcFactory(
	IrcEngine* engine
)
: _engine(engine)
{
}



IrcFactory::~IrcFactory()
{
}



/* The code within all the CreateXxx functions are the same, just dealing with
 * different types and their different construction requirements. The comments
 * specified in CreateIrcChannel are pertinent to the others too. */



std::shared_ptr<IrcChannel>
IrcFactory::CreateIrcChannel(
	std::shared_ptr<IrcConnection> parent_connection,
	const char* channel_name
)
{
	// obtain an IrcChannel from the object pool
	std::shared_ptr<IrcChannel>	channel = _engine->Pools()->IrcChannels()->GetObject(IRCPOOL_GET_ARGS);
	// as we're dealing with a shared_ptr, we need the raw type for the next step
	IrcChannel*			c = channel.get();

	// placement new with constructor arguments, using raw type
	c = ::new (c) IrcChannel(parent_connection, channel_name);
	
	// return the shared_ptr, internal object constructed
	return channel;
}



std::shared_ptr<IrcConnection>
IrcFactory::CreateIrcConnection(
	std::shared_ptr<IrcNetwork> parent_network
)
{
	std::shared_ptr<IrcConnection>	connection = _engine->Pools()->IrcConnections()->GetObject(IRCPOOL_GET_ARGS);
	IrcConnection*			c = connection.get();

	c = ::new (c) IrcConnection(parent_network);

	return connection;
}



std::shared_ptr<IrcNetwork>
IrcFactory::CreateIrcNetwork(
	const char* group_name
)
{
	std::shared_ptr<IrcNetwork>	network = _engine->Pools()->IrcNetworks()->GetObject(IRCPOOL_GET_ARGS);
	IrcNetwork*			n = network.get();

	n = ::new (n) IrcNetwork(group_name);

	return network;
}



std::shared_ptr<IrcUser>
IrcFactory::CreateIrcUser(
	std::shared_ptr<IrcChannel> channel,
	const char* nickname,
	const char* ident,
	const char* hostmask
)
{
	std::shared_ptr<IrcUser>	user = _engine->Pools()->IrcUsers()->GetObject(IRCPOOL_GET_ARGS);
	IrcUser*			u = user.get();

	u = ::new (u) IrcUser(channel, nickname, ident, hostmask);

	return user;
}



END_NAMESPACE
#pragma once

/**
 * @file	src/irc/IrcFactory.h
 * @author	James Warren
 * @brief	IRC Object Factory
 */



#include <memory>
#include <api/definitions.h>


BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class IrcUser;
class IrcChannel;
class IrcConnection;
class IrcNetwork;
class IrcEngine;
class IrcObject;



/**
 * 
 *
 * Plugins should have no need to create any of the objects we supply, so all
 * the rules regarding referencing and deletion functions should only apply to
 * our own internal code.
 *
 * Our memory debugging macros make this classes function declarations look
 * peculiar at best, definitely violating our own style guidelines - but it's 
 * the only way to get the functionality we desire without hardcoding, or adding
 * huge duplicated 'if' preprocessor blocks.
 *
 * @class IrcFactory
 */
class SBI_IRC_API IrcFactory
{
	// we are created by the IrcEngine
	friend class IrcEngine;
private:
	NO_CLASS_ASSIGNMENT(IrcFactory);
	NO_CLASS_COPY(IrcFactory);

	// private constructor; we want one instance that is controlled
	IrcFactory(
		IrcEngine* engine
	);


	/** A pointer to the IRC engine; saves having to access it through the
	 * runtime constantly. Owns this class, so will always exist as long
	 * as this factory does. */
	IrcEngine*	_engine;

public:
	~IrcFactory();


	// copied from the class constructors 


	/**
	 * Creates and constructs a new IrcChannel.
	 *
	 * The object is actually returned from a memory pool, and constructed
	 * via placement new.
	 *
	 * The function arguments match those of the normal constructor. If
	 * memory debugging is enabled for the build, the debug info is also 
	 * supplied in at the same time.
	 *
	 * @warning
	 * Do not use standard deletion functions/macros to free the object; it
	 * must be passed into DeleteIrcObject(), which handles the freeing of
	 * the memory directly from its pool.
	 *
	 * @warning
	 * The object is referenced before it is returned, so do not call it
	 * manually. In turn, when you want to delete it, do not dereference it;
	 * just call DeleteIrcObject()
	 *
	 * @sa Pool::Get()
	 * @sa DeleteIrcObject()
	 * @sa IrcChannel()
	 */
	std::shared_ptr<IrcChannel>
	CreateIrcChannel(
		std::shared_ptr<IrcConnection> parent_connection,
		const char* channel_name
	);


	/**
	 * Creates and constructs a new IrcConnection.
	 *
	 * The object is actually returned from a memory pool, and constructed
	 * via placement new.
	 *
	 * The function arguments match those of the normal constructor. If
	 * memory debugging is enabled for the build, the debug info is also 
	 * supplied in at the same time.
	 *
	 * @warning
	 * Do not use standard deletion functions/macros to free the object; it
	 * must be passed into DeleteIrcObject(), which handles the freeing of
	 * the memory directly from its pool.
	 *
	 * @warning
	 * The object is referenced before it is returned, so do not call it
	 * manually. In turn, when you want to delete it, do not dereference it;
	 * just call DeleteIrcObject()
	 *
	 * @sa Pool::Get()
	 * @sa DeleteIrcObject()
	 * @sa IrcConnection()
	 */
	std::shared_ptr<IrcConnection>
	CreateIrcConnection(
		std::shared_ptr<IrcNetwork> parent_network
	);


	/**
	 * Creates and constructs a new IrcNetwork.
	 *
	 * The object is actually returned from a memory pool, and constructed
	 * via placement new.
	 *
	 * The function arguments match those of the normal constructor. If
	 * memory debugging is enabled for the build, the debug info is also 
	 * supplied in at the same time.
	 *
	 * @warning
	 * Do not use standard deletion functions/macros to free the object; it
	 * must be passed into DeleteIrcObject(), which handles the freeing of
	 * the memory directly from its pool.
	 *
	 * @warning
	 * The object is referenced before it is returned, so do not call it
	 * manually. In turn, when you want to delete it, do not dereference it;
	 * just call DeleteIrcObject()
	 *
	 * @sa Pool::Get()
	 * @sa DeleteIrcObject()
	 * @sa IrcNetwork()
	 */
	std::shared_ptr<IrcNetwork>
	CreateIrcNetwork(
		const char* group_name
	);


	/**
	 * Creates and constructs a new IrcUser.
	 *
	 * The object is actually returned from a memory pool, and constructed
	 * via placement new.
	 *
	 * The function arguments match those of the normal constructor. If
	 * memory debugging is enabled for the build, the debug info is also 
	 * supplied in at the same time.
	 *
	 * @warning
	 * Do not use standard deletion functions/macros to free the object; it
	 * must be passed into DeleteIrcObject(), which handles the freeing of
	 * the memory directly from its pool.
	 *
	 * @warning
	 * The object is referenced before it is returned, so do not call it
	 * manually. In turn, when you want to delete it, do not dereference it;
	 * just call DeleteIrcObject()
	 *
	 * @sa Pool::Get()
	 * @sa DeleteIrcObject()
	 * @sa IrcUser()
	 */
	std::shared_ptr<IrcUser>
	CreateIrcUser(
		std::shared_ptr<IrcChannel> channel,
		const char* nickname,
		const char* ident,
		const char* hostmask
	);
};



END_NAMESPACE

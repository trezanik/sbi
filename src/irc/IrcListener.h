#pragma once

/**
 * @file	IrcListener.h
 * @author	James Warren
 * @brief	Observer design pattern for IRC events
 */



#include <string>
#include <vector>
#include <api/definitions.h>
#include "irc_structs.h"		// prevent unused variable warning


BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class IrcConnection;
struct mode_data;
struct user_modes_change;



/**
 * Unique types for every IRC message we support. The standard codes, 1-999, are
 * used directly, with custom interpretation for text-based messages. Positions
 * are not guaranteed to remain the same, so never use codes directly (outside 
 * of the 1-999).
 *
 * If adding a new code, put it here, add a handler function in the IrcListener
 * class below, and add a call to IrcEngine::NotifyListeners, so it can actually
 * be received by classes deriving from IrcListener.
 *
 * @sa IrcEngine::NotifyListeners
 * @enum E_IRC_LISTENER_NOTIFICATION
 */
enum E_IRC_LISTENER_NOTIFICATION
{
	LN_NewData = 0,			/**< Data received, not yet parsed */
	// real IRC codes (0-999) : add more as we enable features
	LN_001,
	LN_002,
	LN_003,
	LN_004,
	LN_005,
	LN_331 = 331,
	LN_332 = 332,
	LN_353 = 353,
	LN_366 = 366,
	// client updates
	LN_ConnectionReady = 1000,	/**< Internal processing of new connection complete */
	LN_Cap,
	LN_Invite,
	LN_Join,
	LN_Kick,
	LN_Kill,
	LN_Mode,
	LN_Nick,
	LN_Notice,
	LN_Part,
	LN_Privmsg,
	LN_Quit,
	LN_Topic,
	// client send/target
	LN_SentInvite,			/**< We sent an invite */
	LN_WeJoined,			/**< We joined a channel */
	LN_WeKicked,			/**< We kicked another user */
	LN_GotKicked,			/**< We got kicked */
	LN_GotNickChanged,		/**< Our nickname was changed */
	LN_SentPrivmsg,			/**< We sent a PRIVMSG */
	LN_GotUserMode,			/**< Our usermodes were modified */
	LN_GotChannelMode,		/**< Our modes in a channel were modified */
	LN_GotKilled,			/**< We were killed */
	LN_WeParted,			/**< We left the channel */
	LN_WeQuit,			/**< We quit from the server */
	// placeholder for invalids
	LN_MAX
};



/**
 * Derive a class from this to handle all IRC events you choose. Since there are
 * so many event types, this one class handles them all, and provides a blank
 * set of functionality by default (i.e. the class is not abstract).
 *
 * The design choices are purely down to the vast amount of event types there,
 * with small variances in importance.
 *
 * This enables you to only overload the functions/events you want.
 *
 * This class resides within the application runtime, and receives all events
 * as brought in by the IrcParser. The IrcConnection also enables the pre-parse
 * hook, calling into this class before received data enters the parser.
 *
 * IrcChannel and its fellow classes are not listeners, as the IrcParser is
 * responsible for updating their internals. It would also increase the vtable
 * size, and add extra layers of work to do, just for the case of not having the
 * parser with full private access, and updating _while it has the data ready to
 * go_!!
 *
 * @todo
 * These functions can probably all be const, implement if good to go
 *
 * @class IrcListener
 */
class SBI_IRC_API IrcListener
{
	/** The engine calls our methods; nothing else is permitted. */
	friend class IrcEngine;
private:
	NO_CLASS_ASSIGNMENT(IrcListener);
	NO_CLASS_COPY(IrcListener);

protected:

/*-----------------------------------------------------------------------------
 * Pre-Parse hook
 *----------------------------------------------------------------------------*/

	virtual void
	OnData(
		std::shared_ptr<IrcConnection> connection
	)
	{ connection; }


/*-----------------------------------------------------------------------------
 * Normal notifications (our client didn't initiate, or not 'directed' at us)
 *----------------------------------------------------------------------------*/

	/**
	 * Our connection receives a 001
	 *
	 */
	virtual void
	On001(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Our connection receives a 001
	 *
	 */
	virtual void
	On002(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Our connection receives a 001
	 *
	 */
	virtual void
	On003(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Our connection receives a 001
	 *
	 */
	virtual void
	On004(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Our connection receives a 005
	 *
	 */
	virtual void
	On005(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Our connection receives a 331
	 *
	 */
	virtual void
	On331(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Our connection receives a 332
	 *
	 */
	virtual void
	On332(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Our connection receives a 353
	 *
	 */
	virtual void
	On353(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Our connection receives a 366
	 *
	 */
	virtual void
	On366(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Received a response to a CAP request
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnCap(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Another user invited us to a channel
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnInvite(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Another user joined the channel
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnJoin(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * A user kicked another user in the channel
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnKick(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Another user was killed from the server
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnKill(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 *
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnMode(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Another user changed their nickname
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnNick(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * Another user sent a notice that we were recipient to
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnNotice(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 *
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnPart(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 *
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnPrivmsg(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnQuit(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 *
	 * @param[in] connection The IrcConnection this event occurred on
	 * @param[in] activity The connections irc_activity, for easier access
	 */
	virtual void
	OnTopic(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }


/*-----------------------------------------------------------------------------
 * Client notifications (we initiated, or targets only us)
 *----------------------------------------------------------------------------*/

	/**
	 * We invited someone to a channel
	 * @sa OnInvite
	 */
	virtual void
	OnOurInvite(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We joined a channel
	 * @sa OnJoin
	 */
	virtual void
	OnOurJoin(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We kicked another user
	 * @sa OnKick
	 */
	virtual void
	OnOurKick(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We were kicked from a channel
	 * @sa OnKick
	 */
	virtual void
	OnOurKicked(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We were killed from the server
	 * @sa OnKill
	 */
	virtual void
	OnOurKilled(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We had a mode update in a channel
	 * @sa OnMode
	 */
	virtual void
	OnOurMode(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We had a NICK change
	 * @sa OnNick
	 */
	virtual void
	OnOurNick(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We sent a notice
	 * @sa OnNotice
	 */
	virtual void
	OnOurNotice(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We parted from a channel
	 * @sa OnPart
	 */
	virtual void
	OnOurPart(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We sent a privmsg
	 * @sa OnPrivmsg
	 */
	virtual void
	OnOurPrivmsg(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We quit from a connection
	 * @sa OnQuit
	 */
	virtual void
	OnOurQuit(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We had a mode update on the server
	 * @sa OnMode
	 */
	virtual void
	OnOurServerMode(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

	/**
	 * We updated a topic in a channel
	 * @sa OnTopic
	 */
	virtual void
	OnOurTopic(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	)
	{ connection; activity; }

public:

	// public, no-operation constructor/destructor
	IrcListener()
	{
	}

	~IrcListener()
	{
	}


	/**
	 *
	 */
	void
	Notify(
		E_IRC_LISTENER_NOTIFICATION event_type,
		std::shared_ptr<IrcConnection> connection
	);
};



END_NAMESPACE

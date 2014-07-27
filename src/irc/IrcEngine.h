#pragma once

/**
 * @file	src/irc/IrcEngine.h
 * @author	James Warren
 * @brief	The class powering the IRC functionality for the application
 */



#include <set>
#include <memory>

// mandatory inclusion so spawn_interface() can be a friend
#include <api/interface.h>

/* screw forward declaring the enum - it causes more build problems than it's
 * worth, and IrcListener isn't that bloated anyway. */
#include "IrcListener.h"

#include "IrcFactory.h"
#include "irc_status.h"



BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class IrcObject;
class IrcConnection;
class IrcNetwork;
class IrcChannel;
class IrcParser;
class IrcPool;
class IrcGui;





/** Module name accessor for the runtime */
#define IRC_MODULE_NAME			"libirc"

/** Shorthand version of module accessor for cleaner looking code */
#define IRC_RUNTIME_MODULE		IrcEngine* zxcvbnm = static_cast<IrcEngine*>(runtime.GetObjectFromModule(IRC_MODULE_NAME));zxcvbnm

/** Same purpose as IRC_RUNTIME_MODULE, but localized to this project, as we
 * can access the variable without touching the runtime.
 * Most IrcObjects have the _irc_engine available already, but this is for
 * those that don't. */
#define IRC_ENGINE			(static_cast<IrcEngine*>(instance(nullptr)))


/** Max size for an IRC message buffer; 512, + terminating nul */
#define MAX_BUF_IRC_MSG_CRLF		513

/** Max size for an IRC message buffer without the CRLF */
#define MAX_BUF_IRC_MSG			(MAX_BUF_IRC_MSG_CRLF - 2)

/** Max length for an IRC message */
#define MAX_LEN_IRC_MSG_CRLF		(MAX_BUF_IRC_MSG_CRLF - 1)

/** Max length for an IRC message without the CRLF */
#define MAX_LEN_IRC_MSG			(MAX_BUF_IRC_MSG_CRLF - 3)

/** Maximum length of a X.509 attribute; @warning the rfc does NOT specify this! */
#define X509_MAX_ATTRIBUTE_LENGTH	256



/**
 * An enumeration for all the variable connection states that a connection can
 * be in - naturally these are different from the socket TCP/IP states.
 *
 * @enum E_CONNECTION_STATE
 */
enum E_CONNECTION_STATE
{
	CS_Unknown = 0x00,		/**< Default connection state (invalid) */
	CS_Disconnected = 0x01,		/**< Connection has become disconnected */
	CS_Disconnecting = 0x02,	/**< In the process of disconnecting */
	CS_Connecting = 0x04,		/**< In the process of connecting */
	CS_Active = 0x08,		/**< Connected and established */
	CS_InitSent = 0x10,		/**< Init (nick, user) sent to server */
	CS_RecvNames = 0x80,		/**< Currently receiving a 353 NAMES */
};



/**
 *
 *
 *
 * @class IrcEngine
 */
class SBI_IRC_API IrcEngine
{
	// only spawn_interface() is allowed to create this class
	friend int ::spawn_interface();
	/* only IrcConnection and IrcParser can execute NotifyListeners(), as
	 * they are the classes that determine fresh data. Just promise not to
	 * touch the other private variables..! */
	friend class IrcConnection;
	friend class IrcParser;
private:
	NO_CLASS_ASSIGNMENT(IrcEngine);
	NO_CLASS_COPY(IrcEngine);


	/** The listeners receive all data on every connection; the first member
	 * is always the parser, while subsequent members are usually the user
	 * interface(s) and any plugins present */
	std::set<IrcListener*>		_listeners;

#if 0
	/** Keeps knowledge of every IRC object created */
	std::vector<IrcObject*>		_objects;
	/** Connections tracking */
	std::vector<IrcConnection*>	_connections;
	/** Networks tracking */
	std::vector<IrcNetwork*>	_networks;
#endif

	/** The 'active' IrcConnection */
	std::string			_active_connection;
	/** The 'active' IrcChannel */
	std::string			_active_channel;

	/** IrcObject creation factory */
	std::unique_ptr<IrcFactory>	_ircobject_factory;



	// private constructor; we want one instance that is controlled
	IrcEngine();


	/**
	 * Retrieves a pointer to the IRC Object Factory, used for creating new
	 * channels, users, connections, etc. - they are all allocated via a
	 * pool for efficiency purposes.
	 *
	 * Private as only the engine itself can be authorized to create new
	 * IrcObjects.
	 *
	 * @warning
	 * Always access through this function, and not the raw pointer, as the
	 * factory is not created/constructed until this has been called for the
	 * first time.
	 *
	 * @retval IrcFactory*
	 */
	IrcFactory*
	Factory() const;


	/**
	 * Called within IrcConnection whenever it successfully adds data to its
	 * receive queue; the event_type will be LN_NewData.
	 *
	 * Also called from the IrcParser whenever a supported event is parsed
	 * out; a '001' results in LN_001, another user joining results in
	 * LN_Join, and so forth.
	 *
	 * This informs all the _listeners by triggering their own notification
	 * handlers, ready for optional processing.
	 *
	 * @sa AttachListener(), DetachListener()
	 * @param[in] event_type The event that triggered this notification
	 * @param[in] connection The IrcConnection this event occurred in
	 */
	void
	NotifyListeners(
		EIrcListenerNotification event_type,
		std::shared_ptr<IrcConnection> connection
	) const;


public:
	~IrcEngine();


	/**
	 * Retrieves the 'active' IrcChannel (the one the user has requested
	 * to set focus to, or the only one present on the connection).
	 */
	std::shared_ptr<IrcChannel>
	ActiveChannel();


	/**
	 * Retrieves the 'active' IrcConnection (the one the user has requested
	 * to set focus to, or the only one present on creation/closure).
	 */
	std::shared_ptr<IrcConnection>
	ActiveConnection();


	/**
	 * Attaches an IrcListener to receive notifications of NotifyData() -
	 * it must be detached when it is finished.
	 *
	 * Never fails.
	 *
	 * @sa DetachListener(), NotifyData()
	 * @param[in] listener The IrcListener to add
	 */
	void
	AttachListener(
		IrcListener* listener
	);

#if 0
	/**
	 * Retrieves the vector for all known IrcConnection objects; the list is
	 * a copy, and makes no guarantee as to the lifetime of each connection.
	 *
	 * @return Returns a vector of #####
	 */
	std::vector<std::string>
	Connections() const;
#endif

	EIrcStatus
	CreateChannel(
		const uint32_t connection_id,
		const char* name
	);


	EIrcStatus
	CreateConnection(
		const char* network
	);
	

	EIrcStatus
	CreateNetwork(
		const char* name
	);
	

	EIrcStatus
	CreateUser(
		const uint32_t connection_id,
		const char* channel,
		const char* name,
		const char* ident,
		const char* hostmask
	);


	/**
	 * Detaches an IrcListener previously attached. Once removed, the object
	 * will not longer receive notifications of new data on connections.
	 *
	 * @sa AttachListener(), NotifyData()
	 * @param[in] listener The IrcListener to remove
	 */
	void
	DetachListener(
		IrcListener* listener
	);



#if 0
	/**
	 * Retrieves the vector for all known IrcNetwork objects; the list is
	 * a copy, and makes no guarantee as to the lifetime of each connection.
	 *
	 * @return Returns a vector of IrcNetwork pointers
	 */
	std::vector<IrcNetwork*>
	Networks() const;


	/**
	 * @warning
	 * Called within the destructor for IrcObject - do not call manually.
	 *
	 * @sa NotifyNewObject()
	 * @param object The Object deleted
	 */
	void
	NotifyDeleteObject(
		IrcObject* object
	);

	/* I don't like these, and they need replacing, hence they are not
	 * documented. */
	void
	NotifyDeleteConnection(
		IrcConnection* connection
	);
	void
	NotifyDeleteNetwork(
		IrcNetwork* network
	);
	void
	NotifyNewConnection(
		IrcConnection* connection
	);
	void
	NotifyNewNetwork(
		IrcNetwork* network
	);


	/**
	 * Enables the engine to keep track and iterate through all the created
	 * objects.
	 *
	 * @warning
	 * Called within the constructor for IrcObject - do not call manually.
	 *
	 * @sa NotifyDeleteObject()
	 * @param object The Object created
	 */
	void
	NotifyNewObject(
		IrcObject* object
	);
#endif

	/**
	 * Retrieves the IrcParser. Never fails - created on the stack as a
	 * static variable.
	 *
	 * @retval A pointer to the IrcParser
	 */
	IrcParser*
	Parser() const;


	/**
	 * Gets the IrcObject pool allocator. Never fails - created on the stack
	 * as a static variable.
	 *
	 * @retval A pointer to the IrcPool
	 */
	IrcPool*
	Pools() const;


	/**
	 * Accesses the UI functionality exposed by this interface.
	 *
	 * Never fails - created on the stack as a static variable.
	 *
	 * @retval A pointer to the IrcGui
	 */
	IrcGui*
	UI() const;
};



END_NAMESPACE

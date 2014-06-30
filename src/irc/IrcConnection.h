#pragma once

/**
 * @file	IrcConnection.h
 * @author	James Warren
 * @brief	The class for an individual IRC connection to a server
 */



#include <queue>
#include <mutex>
#include <set>
#include <memory>

#if defined(_WIN32)
#	include <WS2tcpip.h>		// DWORD, HANDLE
#endif
#if defined(__linux__) || defined(BSD)
#	include <pthread.h>
#endif
#if defined(USING_OPENSSL_NET)
#	include <openssl/bio.h>
#elif defined(USING_BOOST_NET)
#	include <boost/asio/ip/tcp.hpp>
#endif

#include <api/char_helper.h>
#include <api/Runtime.h>
#include "IrcObject.h"
#include "nethelper.h"
#include "irc_structs.h"
#include "irc_status.h"




BEGIN_NAMESPACE(APP_NAMESPACE)

// forward declarations
class IrcChannel;
class IrcNetwork;
struct config_server;



/**
 * This structure holds variables extracted from the xml configuration, ready
 * to be used in a connection attempt. This saves needing to process xml on
 * connection (and duplication), needing it only once per connection. They also
 * receive dynamic data applicable to a pre-connection, like name resolution.
 *
 * @struct irc_connection_params
 */
struct irc_connection_params
{
	std::string	data;		/**< Holder for lookup information */
	std::string	conn_str;	/**< OpenSSL connection string 'host:port' */
	std::string	host;		/**< The hostname of the remote server */
	std::string	ip_addr;	/**< The ip address of the remote server */
	ip_address	ip;		/**< An  ip_address structure, populated through inet_ntop */
	sockaddr_union	sa;		/**< Holds the reverse lookup info */
	uint16_t	port;		/**< The port to connect to */
	bool		use_ssl;	/**< Whether or not to use SSL to connect */
	bool		allow_invalid_cert;	/**< If use_ssl, whether it allows an invalid certificate */
};



/**
 *
 *
 *
 * @class IrcConnection
 */
class SBI_IRC_API IrcConnection : public IrcObject
{
	// Updates internals directly when parsing relevant data
	friend class IrcParser;
	// reads + updates internals
	friend class IrcNetwork;
	// calls ExecEstablishConnection
	//friend bool Runtime::CreateThread(E_THREAD_TYPE, void*, const char*);
private:
	NO_CLASS_ASSIGNMENT(IrcConnection);
	NO_CLASS_COPY(IrcConnection);

	uint32_t	_state;		/**< flag-based connection state */
	time_t		_last_data;	/**< The time data was last received */
	time_t		_lag_sent;	/**< The time 'LAG' was sent */
	uint64_t	_bytes_recv;	/**< stats tracking - bytes received */
	uint64_t	_bytes_sent;	/**< stats tracking - bytes sent */

	/** Synchronization lock; mutable to enable constness for retrieval functions */
	mutable std::mutex		_mutex;

	/** The connections unprocessed receive queue */
	std::queue<std::string>		_recv_queue;
	/** The connections unprocessed send queue */
	std::queue<std::string>		_send_queue;

	/** The connections channel list */
	std::set<std::string>		_channel_list;

#if defined(USING_OPENSSL_NET)
	std::unique_ptr<BIO>		_socket;	/**< The raw socket, in an OpenSSL struct */
	std::unique_ptr<SSL_CTX>	_ssl_context;	/**< OpenSSL's context */
	std::unique_ptr<SSL>		_ssl;		/**< OpenSSL's state and data */
#elif defined(USING_BOOST_NET)
	boost::asio::ip::tcp::socket*			_socket;	/**<  */
	boost::asio::io_service*			_io_service;	/**<  */
	boost::asio::ip::tcp::resolver::iterator*	_endpoint_iterator;	/**<  */
#endif

	std::weak_ptr<IrcNetwork>	_owner;		/**< the network we reside in */

	std::vector<std::string>	_cap_ack;	/**< Servers confirmed CAP support */
	std::vector<std::string>	_cap_nak;	/**< Servers confirmed CAP no support */
	std::vector<std::string>	_cap_ls;	/**< Servers listed CAP support */

	irc_activity			_activity;	/**< The last parsed activity */
	irc_connection_params		_params;	/**< The parameters used for the last server connection */
	uint32_t			_id;		/**< Unique id */

#if defined(_WIN32)
	uint32_t	_thread;	/**< ID of the connection preservation thread */
#elif defined(__linux__) || defined(BSD)
	pthread_t	_thread;	/**< The connection preservation thread */
#endif


	/**
	 * Adds the supplied data to the connections receiving queue, ready to be
	 * picked up aned processed by the parsing thread.
	 *
	 * @param data The raw text to have received
	 * @return Returns true if the data is added to the pending queue, otherwise
	 * false is returned
	 */
	EIrcStatus
	AddToRecvQueue(
		const char* data
	);


	/**
	 * Adds the supplied data to the connections send queue, ready for sending
	 * to the server. The queue is processed by the dedicated thread automatically,
	 * keeping within the bounds of flood protection.
	 *
	 * The trailing carriage return and linefeed are added BY this function,
	 * so there is no need to supply it as part of the data.
	 *
	 * @param data The raw text to send
	 * @retval retval true if the data is added to the pending queue
	 * @retval false on any failure; no data added to the pending queue
	 */
	EIrcStatus
	AddToSendQueue(
		const char* data
	);


	/**
	 * Wipes out the linked list containing the IRC channels.
	 *
	 * Only expected to be executed when the destructor or Cleanup() are
	 * called.
	 */
	EIrcStatus
	EraseChannelList();


	/**
	 * The thread function that constantly receives data and adds it to the
	 * receiving queue. Automatically sends data (initial connection, PING
	 * and PONGs) where applicable.
	 *
	 * Only called through ExecEstablishConnection() as part of a new thread.
	 *
	 * @return Always returns 0.
	 * @sa ExecEstablishConnection, Runtime::CreateThread
	 */
	uint32_t
	EstablishConnection();


	/**
	 * Used for creating a thread for the EstablishConnection() function.
	 * Since we reside in a C++ class, this is necessary to be able to have
	 * it called via a new thread.
	 *
	 * @warning
	 * Do not call manually; the internal functions and other classes will
	 * do all necessary preparation, and call Runtime::CreateThread, which
	 * in turn will execute this function. As a result, no safety checks are
	 * performed.
	 *
	 * Runtime::CreateThread is a friend in order to call this function.
	 *
	 * @param[in] thisptr A pointer to the IrcConnection class
	 * @return Always returns 0. Whatever happens in the called function
	 * stays unique and separate.
	 * @sa Runtime::CreateThread, IrcParser::ExecParser
	 */
#if defined(_WIN32)
	static uint32_t __stdcall
#elif defined(__linux__) || defined(BSD)
	static void*
#endif
	ExecEstablishConnection(
		void* thisptr
	);


	/**
	 * Sends data across the wire through the socket, without doing any form
	 * of flood protection - the data is sent immediately.
	 *
	 * Called by AddToSendQueue after flood protection has been taken into
	 * account.
	 *
	 * @param[in] data_format The format-string of the data to send
	 * @param[in] ... Variable number of parameters as determined within  data_format
	 * @return The return value is that of BIO_puts. Error handling is already
	 * processed when the function returns, if necessary.
	 */
	EIrcStatus
	SendBypass(
		const char* data_format,
		...
	);

public:

#if 0	// Code Removed: consider this style method for variable access instead of friends
	struct {
		Proxy<uint64_t*, IrcConnection>	bytes_recv;
		Proxy<uint64_t*, IrcConnection>	bytes_sent;
	} read_only;
#endif


	IrcConnection(
		std::shared_ptr<IrcNetwork> network
	);
	~IrcConnection();


	/**
	 * Allocates a new IrcChannel, identified as channel_name, linking
	 * it into the connections channel list.
	 *
	 * To retrieve it, call GetChannel()
	 *
	 * @param channel_name The channel name to add to the channel list
	 * @return If the channel is added to the connection list, the function returns
	 * true; otherwise false
	 */
	EIrcStatus
	AddChannel(
		 const char* channel_name
	);


	/**
	 * Sends a request to the connected server to choose the next nickname in the
	 * users profile.
	 *
	 * @return If no more nicknames are available, EC_NoMoreNicks is returned, but
	 * the counter is reset (so it can be called continuously again). EC_Success is
	 * returned when the nick is changed.
	 */
	EIrcStatus
	AutoChangeNick();


	/**
	 * Cleans up the connection, freeing any allocated memory, closing any sockets
	 * and shutting down SSL, if in usage.
	 *
	 * This will not destroy the connection object itself, allowing it to be reused
	 * for another connection (i.e. reconnecting to the same server)
	 */
	EIrcStatus
	Cleanup();


	/**
	* @todo fixme
	 * Physically connects to the server set within the connection. This is done
	 * within , which includes performing a DNS lookup
	 * and populating the sockaddr structures. This function makes the first real
	 * outbound call, and on success, creates a thread to parse any data received.
	 *
	 * @note
	 * Currently hardcoded to use OpenSSL for a connection without any
	 * preprocessor guards - to change if we want to support other SSL
	 * implementations. Until then, this will cause a compile error if not
	 * USING_OPENSSL
	 *
	 * @return If an outbound connection is successfully made, and a thread created
	 * to process it, the function returns 0.
	 * @return Any failure causes the function to return -1
	 * @return If the connection is already active, the function returns 1
	 */
	EIrcStatus
	ConnectToServer();


#if 0	// Code Removed: now handled by Runtime::CreateThread
	/**
	 * Creates the thread used to establish & maintain a connection to the
	 * IRC server.
	 *
	 * Should only be called within UI input, after a successful call to
	 * ConnectToServer(), otherwise the internal state will certainly fail
	 * hard.
	 *
	 * @return Returns true on success, otherwise false.
	 */
	bool
	CreateThread();
#endif


	/**
	 * Deletes the supplied channel. If it is still a part of the
	 * channel list, it will be unlinked first.
	 *
	 * @sa GetChannel(), RemoveChannel()
	 * @param[in] channel The IrcChannel to delete
	 * @return Returns true if the channel is deleted; otherwise false
	 */
	EIrcStatus
	DeleteChannel(
		const char* channel_name
	);


	/**
	 * Gets the irc_activity struct for this connection.
	 *
	 * Only called within IrcEngine as a means of getting the struct to pass
	 * to the listeners.
	 * As such, the lifetime of the returned reference is 'guaranteed' to be
	 * sufficient for as long as the listener functions are being run, and
	 * is also the only point at which the data within the struct should be
	 * accurate for usage.
	 *
	 * @return A reference to the irc_activity is returned
	 */
	irc_activity&
	GetActivity()
	{ return _activity; }


	/**
	 * Gets the IrcChannel object from the specified connection, whose name
	 * matches channel_name.
	 *
	 * Will not generate an error if the specified channel is not found, as
	 * this can be used for a simple search; raise your own error if this is
	 * undesired.
	 *
	 * @warning Increments the reference counter for the channel; you must
	 * call Dereference on it when finished using it!
	 *
	 * @param[in] channel_name The name of the channel to locate
	 * @return If the channel is not found, a nullptr is returned, otherwise
	 * it is a pointer to the IrcChannel called channel_name
	 */
	std::shared_ptr<IrcChannel>
	GetChannel(
		const char* channel_name
	);


	/**
	 * Retrieves a copy of the current nickname in usage for the network
	 * connection.
	 *
	 * @return The copied string is returned, so the memory will not be
	 * invalidated if the nickname changes while this is still held
	 */
	std::string
	GetCurrentNickname() const;


	/**
	 * Retrieves the group name of the network, as set by the user.
	 *
	 * @sa IrcNetwork::GroupName
	 * @return A copy of the group network name
	 */
	std::string
	GroupName() const;


	/**
	 * 
	 */
	uint32_t
	Id() const
	{
		return _id;
	}


	/**
	 * Determines if the connection is currently pending (i.e. in the state
	 * between connecting to the server, and being actually connected to it.
	 *
	 * @return Returns true if the connection is in the 'connected' state,
	 * otherwise returns false
	 */
	bool
	IsActive();


	/**
	 * Determines if the connection is currently pending (i.e. in the state
	 * between connecting to the server, and being actually connected to it.
	 *
	 * @return Returns true if the connection is in the 'connecting' state,
	 * otherwise returns false
	 */
	bool
	IsConnecting();


	/**
	 * Determines if the connection is currently in a complete disconnected
	 * state - so is ready for a reconnect call, if needed.
	 *
	 * @return Returns true if the connection is in the 'disconnected' state,
	 * otherwise returns false
	 */
	bool
	IsDisconnected();


	/**
	 * Determines if the connection is currently pending (i.e. in the state
	 * between connecting to the server, and being actually connected to it)
	 *
	 * @return Returns true if the connection is in the 'disconnecting' state,
	 * otherwise returns false
	 */
	bool
	IsDisconnecting();


	/**
	 * Retrieves the name of the network, as reported within its ISUPPORT
	 * response.
	 *
	 * @sa IrcNetwork::Name
	 * @return A copy of the server network name; if ISUPPORT (005) has not
	 * yet being received & processed, the string will be empty.
	 */
	std::string
	NetworkName() const;


	/**
	 * Retrieves the pointer for the IrcNetwork that owns this connection.
	 *
	 * @warning Increments the reference counter for the network; you must
	 * call Dereference on it when finished using it!
	 *
	 * @retval A pointer to an IrcNetwork
	 */
	std::shared_ptr<IrcNetwork>
	Owner();


	/**
	 * Sends a message to the server to set our 'AWAY' state.
	 *
	 * @param[in] message The message to report back
	 */
	EIrcStatus
	SendAway(
		const char* message = nullptr
	);

	/**
	 * Sends a message to the server to clear a previously set 'AWAY' state.
	 */
	EIrcStatus
	SendBack();

	/**
	 * The contents of message are surrounded [prefix+suffix] by the CTCP
	 * codes (\\001), and sent off via a PRIVMSG to the supplied target.
	 */
	EIrcStatus
	SendCTCP(
		const char* target,
		const char* message
	);

	/**
	 *
	 */
	EIrcStatus
	SendCTCPNotice(
		const char* destination,
		const char* message
	);

	/**
	 * Sends the client standard initialization (nick and user, plus any
	 * capabilities with newer servers).
	 * Some servers WE have to trigger this, as they wait for our first data
	 * whereas most others simply start the 439 straight away.
	 */
	EIrcStatus
	SendInit();

	/**
	 *
	 */
	EIrcStatus
	SendInvite(
		const char* channel_name,
		const char* nickname
	);

	/**
	 *
	 */
	EIrcStatus
	SendJoin(
		const char* channel_name,
		const char* channel_key = nullptr

	);

	/**
	 *
	 */
	EIrcStatus
	SendKick(
		const char* channel_name,
		const char* nickname,
		const char* msg = nullptr
	);

	/**
	 *
	 */
	EIrcStatus
	SendMode(
		const char* target,
		const char* mode
	);

	/**
	 *
	 */
	EIrcStatus
	SendNick(
		const char* nickname
	);

	/**
	 *
	 */
	EIrcStatus
	SendIdentify(
		const char* service,
		const char* pass
	);

	/**
	 *
	 */
	EIrcStatus
	SendNotice(
		const char* target,
		const char* message
	);

	/**
	 *
	 */
	EIrcStatus
	SendPart(
		const char* channel_name,
		const char* msg = nullptr
	);

	/**
	 *
	 */
	EIrcStatus
	SendPrivmsg(
		const char* target,
		const char* privmsg
	);

	/**
	 *
	 */
	EIrcStatus
	SendRaw(
		const char* data
	);

	/**
	 *
	 */
	EIrcStatus
	SendQuit(
		const char* msg = nullptr
	);

	/**
	 *
	 */
	EIrcStatus
	SendTopic(
		const char* channel_name,
		const char* message = nullptr
	);

	/**
	 *
	 */
	EIrcStatus
	SendUser(
		const char* username,
		const uint16_t mode = 8,
		const char* realname = nullptr
	);


	/**
	 * Activates the supplied channel as the currently active IrcChannel
	 * in the connection. If no channels exist, and AddChannel() adds a
	 * new channel, this is automatically called to make it active.
	 *
	 * Dereferences any channel that was previously set to active, before
	 * referencing the supplied one, and setting it active.
	 *
	 * @param[in] channel The channel name to set as active
	 */
	EIrcStatus
	SetActiveChannel(
		const char* channel_name
	);


	/**
	 * Creates an irc_connection struct, ready for a call to connect_to_server.
	 * This also performs the DNS lookup, sockaddr creation, etc. This should be
	 * the only outbound data at this stage; we won't touch the specified server
	 * until the actual connect function is called.
	 *
	 * Populates the classes parameters struct, required before calling
	 * EstablishConnection
	 *
	 * The OpenSSL library will also be used to create the BIO (socket) in
	 * preparation for usage before returning.
	 *
	 * If using SSL, this creates the SSL struct and its context too.
	 *
	 * @param[in] network A pointer to this connections network; obtained from a prior
	 * call to ???
	 * @param[in] server_config A pointer to the XML element for the server to connect to
	 * @return On success, a pointer to the dynamically allocated struct is
	 * returned; this must be freed when no longer in usage.
	 * @return Any failure results in the function returning false
	 */
	EIrcStatus
	Setup(
		std::shared_ptr<IrcNetwork> network,
		std::shared_ptr<config_server> server_config
	);

};


END_NAMESPACE

#pragma once

/**
 * @file	src/api/RpcServer.h
 * @author	James Warren
 * @brief	API RPC Server
 * @note	If this looks similar to Bitcoin et-al's RPC implementation, it
 *		is not a coincidence. I've never had to use json spirit or write
 *		a RPC client or server before, and I'd rather use something that
 *		works and adapt it then reinvent the wheel - incorrectly. 
 *		See bitcoinrpc.[h|cpp] for the originals.
 */



#include <string>
#include <map>
#include <list>

#if defined(USING_JSON_SPIRIT_RPC)
#	include <json_spirit/json_spirit_utils.h>
	/* anything using API will need to link to boost if using the normal
	 * header (boost/asio/io_service.hpp) and not the forward declaration */
	namespace boost { 
		namespace asio { 
			class io_service; 
		}
	}
#endif
#if defined(USING_BOOST_NET)
	/* anything using API will need to link to boost if using the normal
	 * header (boost/asio/ip/address.hpp) and not the forward declaration */
	namespace boost {
		namespace asio {
			namespace ip {
				class address;
			}
		}
	}
#endif

#include "definitions.h"
#include "RpcTable.h"		// RpcServer variable, rpc_map typedef
#include "RpcCommand.h"
#include "rpc_status.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



// HTTP status codes returned from the server
#define HTTP_OK				200
#define HTTP_BAD_REQUEST		400
#define HTTP_UNAUTHORIZED		401
#define HTTP_FORBIDDEN			403
#define HTTP_NOT_FOUND			404
#define HTTP_INTERNAL_SERVER_ERROR	500

// our maximum content-length in HTTP
#define HTTP_MAX_CONTENT_LENGTH		8000

/** the port the server listens on, and the clients must connect to
 * @note If modifying, be sure to update the three entries in Configuration.cc 
 * with this changed value, as they do not utilize this definition!
 */
#define RPC_PORT		50451

// This is needed because the foreach macro can't get over the comma in pair<t1, t2>
#define PAIRTYPE(t1, t2)	std::pair<t1, t2>


// forward declarations
class AcceptedConnection;
class RpcServer;




/**
 * RPC handler thread parameters
 */
struct SBI_API rpch_params
{
	RpcServer*	thisptr;

	std::shared_ptr<AcceptedConnection>	connection;

#if defined(_WIN32)
	uintptr_t	thread_handle;
	uint32_t	thread_id;
#else
	pthread_t	thread;
#endif

};


/**
 * RPC server thread parameters
 */
struct SBI_API rpcs_params
{
	RpcServer*	thisptr;

#if defined(_WIN32)
	uintptr_t	thread_handle;
	uint32_t	thread_id;
#else
	pthread_t	thread;
#endif

};




/**
 * RPC server class.
 *
 * Interfaces add RPC support for themselves by registering functions into the
 * server, which adds them to the RPC table. These must be unregistered when the
 * interface is unloaded.
 *
 * Inbuilt commands are public methods within the class (so are pretty much the
 * libapi RPC commands).
 */
class SBI_API RpcServer
{
private:
	NO_CLASS_ASSIGNMENT(RpcServer);
	NO_CLASS_COPY(RpcServer);


	RpcTable		_table;

	/**
	 * Flag to trigger the shutdown of the RPC server. Will continue running
	 * (assuming it started up ok) until this is set via Shutdown().
	 */
	bool			_shutdown;

	/**
	 * Passed into the ServerThread for creation parameters. Used by the
	 * Startup() and Shutdown() functions.
	 */
	rpcs_params		_server_params;

#if defined(USING_JSON_SPIRIT_RPC)
	/**
	 * Runs in the RpcServerThread. In order to signal it to stop, it must 
	 * be callable from another thread, so we retain it in this class.
	 *
	 * Is a pointer to get around needing to provide boost paths and linkage
	 * to projects for this single variable.
	 */
	std::unique_ptr<boost::asio::io_service>	_io_service;
#endif

	/**
	 * Stores the base64 encoded 'username:password' credentials used to
	 * authenticate with the server, as read by the configuration file.
	 */
	std::string		_rpc_auth;



	/**
	 * Returns if the client is authorized, comparing the supplied
	 * credentials with those set in the configuration.
	 *
	 * Uses basic authentication; for reference:
	 * http://en.wikipedia.org/wiki/Basic_access_authentication
	 *
	 * @param[in] headers HTTP headers, as received and mapped
	 */
	bool
	IsAuthorizedHTTP(
		std::map<std::string, std::string>& headers
	);



	/**
	 * Reads http status code
	 */
	uint32_t
	ReadHTTPStatus(
		std::basic_istream<char>& stream, 
		int& proto
	);


	/**
	 * Returns length of content, negative on error
	 */
	int32_t 
	ReadHTTPHeader(
		std::basic_istream<char>& stream, 
		std::map<std::string, std::string>& headers
	);


	/**
	 * Returns HTTP status code
	 *
	 * @param[in] stream
	 * @param[out] headers
	 * @param[out] message
	 */
	uint32_t
	ReadHTTP(
		std::basic_istream<char>& stream, 
		std::map<std::string, std::string>& headers,
		std::string& message
	);



	/**
	 * The thread function created for every RPC client. Performs the tasks
	 * as required, and exits as soon as the work is done. Only created when
	 * the client address is allowed access, but before HTTP authorization;
	 * which this function performs.
	 */
	ERpcStatus
	RpcHandlerThread(
		rpch_params* tp
	);


	/**
	 * Enters an endless loop, processing RPC requests.
	 *
	 * Do not call directly; must be executed via ExecServerThread, which 
	 * needs to be the function passed into a new thread creation.
	 *
	 * @sa ExecServerThread
	 * @param[in] tp A pointer to a rpcs_params struct.
	 * @return
	 */
	ERpcStatus
	RpcServerThread(
		rpcs_params* tp
	);


	

	void
	TypeCheck(
		const json_spirit::Array& params,
		const std::list<json_spirit::Value_type>& expected_types,
		bool allow_null
	) const;


	void
	TypeCheck(
		const json_spirit::Object& obj,
		const std::map<std::string, json_spirit::Value_type>& expected_types,
		bool allow_null
	) const;



protected:
public:
	RpcServer();
	~RpcServer();



	/**
	 * Executes the RpcHandlerThread function.
	 *
	 * This is needed, and static, so that it can be the recipient to a new
	 * thread creation, as it's part of a class.
	 *
	 * @sa RpcHandlerThread
	 * @param[in] params A pointer to populated rpch_params cast void
	 * @return Returns the value returned by RpcHandlerThread, as an uint32_t
	 */
#if defined(_WIN32)
	static uint32_t
	__stdcall
#else
	static void*
#endif
	ExecRpcHandlerThread(
		void* params
	);



	/**
	 * Executes the RpcServerThread function.
	 *
	 * This is needed, and static, so that it can be the recipient to a new
	 * thread creation, as it's part of a class.
	 *
	 * @sa RpcServerThread
	 * @param[in] params A pointer to populated rpcs_params cast void
	 * @return Returns the value returned by RpcServerThread, as an uint32_t
	 */
#if defined(_WIN32)
	static uint32_t
	__stdcall
#else
	static void*
#endif
	ExecRpcServerThread(
		void* params
	);


	/**
	 * Gets the number of CPU execution cores available to the environment.
	 * 
	 * Attempts to use C++11 hardware_concurrency by default; if this does
	 * not return a value, a fallback value obtained using the OS API is
	 * used.
	 */
	json_spirit::Value
	GetEnvironmentCoreCount(
		const json_spirit::Array& params,
		bool fHelp
	);


	/**
	 * Gets the details for a loaded Interface, such as path, memory address
	 * and RPC commands, amongst others.
	 */
	json_spirit::Value
	GetInterfaceInfo(
		const json_spirit::Array& params,
		bool fHelp
	);


	/**
	 * Retrieves a pointer to the RpcTable, to Add/Remove/Execute
	 * RpcCommands. Never fails.
	 */
	RpcTable*
	GetRpcTable()
	{
		return &_table;
	}


	/**
	 * Generates a HTTP response based on the input parameters.
	 *
	 * @param[in] status_code The HTTP status code (200, 403, 404, etc.)
	 * @param[in] msg The body of the reply (everything after content-length)
	 * @param[in] keepalive Enable/disable connection keep-alive
	 */
	std::string
	HTTPReply(
		int status_code,
		const std::string& msg,
		bool keepalive
	);


	/**
	 * Checks the supplied IP address against the configuration list of
	 * allowed remote IPs.
	 *
	 * @note
	 * If the configuration sets local_only, this always returns false,
	 * regardless of if the IP is in the allow list.
	 *
	 * @params[in] client_addr A pointer to the address. Only a pointer to
	 * get round needing to include the boost headers, meaning everything
	 * using the API would also need to link to the boost library.
	 * @retval true if the client is allowed or is the loopback address
	 * @retval false if the client is not allowed
	 */
	bool
	IsClientAllowed(
		boost::asio::ip::address* client_addr
	);


	/**
	 * Shuts down the RPC server, preventing any IPC. Once done, can only be
	 * restarted by the API.
	 *
	 * RPC threads running tasks will cease processing once the task is
	 * finished, so this function could potentially take some time to return
	 * and should be catered for.
	 *
	 * Should only normally be needed when closing the application.
	 *
	 * @sa Startup
	 * @retval ERpcStatus::Ok if the server was running, and this call then
	 * stopped the server.
	 * @retval ERpcStatus::IsShutdown if the server was already shutdown
	 */
	ERpcStatus
	Shutdown();


	/**
	 * Starts the RPC server thread, enabling IPC.
	 *
	 * Only normally executed when starting up the application.
	 *
	 * @sa Shutdown
	 */
	ERpcStatus
	Startup();
};






END_NAMESPACE

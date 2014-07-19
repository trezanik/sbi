#pragma once

/**
 * @file	RpcServer.h
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
	/* anything using API will need to link to boost if using the 'normal'
	 * headers and not this forward declaration */
	namespace boost { namespace asio { class io_service; }}
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

// the port the server listens on, and the clients must connect to
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

	/** flag to trigger the shutdown of the RPC server. Will continue
	 * running (assuming it started up ok) until this is set via Shutdown().
	 */
	bool			_shutdown;

	/**
	 * Passed into the ServerThread for creation parameters. Used by the
	 * Startup() and Shutdown() functions.
	 */
	rpcs_params		_server_params;

#if defined(USING_JSON_SPIRIT_RPC)
	/**
	 * Runs in the ServerThread. In order to signal it to stop, it must be
	 * callable from another thread, so we retain it in this class.
	 *
	 * Is a pointer to get around needing to provide boost paths and linkage
	 * to projects for this single variable.
	 */
	std::unique_ptr<boost::asio::io_service>	_io_service;
#endif


	/**
	 *
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
	ServerThread(
		rpcs_params* tp
	);


	

	void
	TypeCheck(
		const json_spirit::Array& params,
		const std::list<json_spirit::Value_type>& expected_types,
		bool fAllowNull
	) const;


	void
	TypeCheck(
		const json_spirit::Object& obj,
		const std::map<std::string, json_spirit::Value_type>& expected_types,
		bool fAllowNull
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
	 * Executes the ServerThread function.
	 *
	 * This is needed, and static, so that it can be the recipient to a new
	 * thread creation, as it's part of a class.
	 *
	 * @sa ServerThread
	 * @param[in] params A pointer to populated rpcs_params cast void
	 * @return Returns the value returned by ServerThread, as an uint32_t
	 */
#if defined(_WIN32)
	static uint32_t
	__stdcall
#else
	static void*
#endif
	ExecServerThread(
		void* params
	);


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

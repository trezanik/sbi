
/**
 * @file	RpcServer.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <thread>		// hardware_concurrency

#if defined(USING_JSON_SPIRIT_RPC)
#	include <boost/foreach.hpp>
#	include <boost/algorithm/string.hpp>
#	include "JsonRpc.h"
#endif
#if defined(USING_BOOST_NET)
	// ironically requires OpenSSL for the ssl functionality anyway
#	if IS_VISUAL_STUDIO
#		pragma comment ( lib, "libeay32.lib" )
#		pragma comment ( lib, "ssleay32.lib" )
#	endif
#	include <boost/asio.hpp>
#	include <boost/asio/ssl.hpp>
#	include <boost/iostreams/concepts.hpp>
#	include <boost/iostreams/stream.hpp>
	/* I don't like obscuring namespace use, which is why you'll rarely see
	 * 'using namespace' in my code - but the boost variables are hidden so
	 * deep it prevents code clarity. These namespace aliases aim to improve
	 * this situation, at least slightly. */
	namespace boost_ip = boost::asio::ip;
	namespace boost_ssl = boost::asio::ssl;
#endif

#if defined(_WIN32)
#	include <Windows.h>
#	include <process.h>
#else
#	include <pthread.h>
#endif

#include "Allocator.h"
#include "RpcServer.h"
#include "Runtime.h"
#include "Log.h"
#include "Terminal.h"
#include "utils.h"
#include "version.h"



BEGIN_NAMESPACE(APP_NAMESPACE)




//
// IOStream device that speaks SSL but can also speak non-SSL
//
class SSLIOStreamDevice : public boost::iostreams::device<boost::iostreams::bidirectional> 
{
public:
	SSLIOStreamDevice(
		boost_ssl::stream<typename boost_ip::tcp::socket> &stream, 
		bool use_ssl
	) : _stream(stream)
	{
		_use_ssl = _need_handshake = use_ssl;
	}

	void
	handshake(
		boost_ssl::stream_base::handshake_type role
	)
	{
		if ( !_need_handshake )
			return;
		_need_handshake = false;
		_stream.handshake(role);
	}
	std::streamsize
	read(
		char* s,
		std::streamsize n
	)
	{
		handshake(boost_ssl::stream_base::server); // HTTPS servers read first
		return _use_ssl ?
			_stream.read_some(boost::asio::buffer(s, n)) :
			_stream.next_layer().read_some(boost::asio::buffer(s, n));
	}
	std::streamsize
	write(
		const char* s,
		std::streamsize n
	)
	{
		handshake(boost_ssl::stream_base::client); // HTTPS clients write first
		return _use_ssl ?
			boost::asio::write(_stream, boost::asio::buffer(s, n)) :
			boost::asio::write(_stream.next_layer(), boost::asio::buffer(s, n));
	}
	bool 
	connect(
		const std::string& server,
		const std::string& port
	)
	{
		boost_ip::tcp::resolver			resolver(_stream.get_io_service());
		boost_ip::tcp::resolver::query		query(server.c_str(), port.c_str());
		boost_ip::tcp::resolver::iterator	endpoint_iterator = resolver.resolve(query);
		boost_ip::tcp::resolver::iterator	end;
		boost::system::error_code		error = boost::asio::error::host_not_found;
		while ( error && endpoint_iterator != end )
		{
			_stream.lowest_layer().close();
			_stream.lowest_layer().connect(*endpoint_iterator++, error);
		}
		if ( error )
			return false;
		return true;
	}

private:
	NO_CLASS_ASSIGNMENT(SSLIOStreamDevice);

	bool	_need_handshake;
	bool	_use_ssl;
	boost_ssl::stream<typename boost_ip::tcp::socket>&	_stream;
};



class AcceptedConnection
{
public:
	AcceptedConnection(
		boost::asio::io_service& io_service,
		boost_ssl::context &context,
		bool use_ssl
	)
	: ssl_stream(io_service, context),
	  _dev(ssl_stream, use_ssl),
	  _stream(_dev)
	{
	}

	std::iostream&
	stream()
	{
		return _stream;
	}

	std::string
	peer_address_to_string() const
	{
		return peer.address().to_string();
	}

	void
	close()
	{
		_stream.close();
	}

	boost_ip::tcp::endpoint				peer;
	boost_ssl::stream<boost_ip::tcp::socket>	ssl_stream;

private:
	SSLIOStreamDevice				_dev;
	boost::iostreams::stream<SSLIOStreamDevice>	_stream;
};



// Forward declaration required for RPCListen
static void
RPCAcceptHandler(
	std::shared_ptr<boost::asio::basic_socket_acceptor<boost_ip::tcp>> acceptor,
	boost_ssl::context& context,
	bool use_ssl,
	std::shared_ptr<AcceptedConnection> conn,
	const boost::system::error_code& error
);

/**
 * Sets up I/O resources to accept and handle a new connection.
 */
static void 
RPCListen(
	std::shared_ptr<boost::asio::basic_socket_acceptor<boost_ip::tcp>> acceptor,
	boost_ssl::context& context,
	const bool use_ssl
)
{
	// Accept connection
	std::shared_ptr<AcceptedConnection> conn;
	
	conn.reset(new AcceptedConnection(acceptor->get_io_service(), context, use_ssl));

	acceptor->async_accept(
		conn->ssl_stream.lowest_layer(),
		conn->peer,
		boost::bind(
			&RPCAcceptHandler,
			acceptor,
			boost::ref(context),
			use_ssl,
			conn,
			boost::asio::placeholders::error
		)
	);
}

bool
ClientAllowed(
	boost_ip::address client_addr
)
{
	/* ensure that IPv4-compatible and IPv4-mapped IPv6 addresses are 
	 * treated as IPv4 addresses */
	if ( client_addr.is_v6()
	    && ( client_addr.to_v6().is_v4_compatible()
	    || client_addr.to_v6().is_v4_mapped())
	)
	{
		return ClientAllowed(client_addr.to_v6().to_v4());
	}

	if ( client_addr == boost_ip::address_v4::loopback()
	    || client_addr == boost_ip::address_v6::loopback()
	    // Check whether IPv4client_addres match 127.0.0.0/8 (loopback subnet)
	    || (client_addr.is_v4() && (client_addr.to_v4().to_ulong() & 0xff000000) == 0x7f000000) )
	{
		return true;
	}

	const std::string	address_str = client_addr.to_string();
	/* to implement:
	const std::vector<std::string>& allow_vect = mapMultiArgs["-rpcallowip"];
	
	BOOST_FOREACH(std::string allow_str, allow_vect)
	{
		if ( WildcardMatch(address_str, allow_str) )
			return true;
	}

	return false;
	*/
	return true;
}

/**
 * Accept and handle an incoming connection.
 */
void
RPCAcceptHandler(
	std::shared_ptr<boost::asio::basic_socket_acceptor<boost_ip::tcp>> acceptor,
	boost_ssl::context& context,
	const bool use_ssl,
	std::shared_ptr<AcceptedConnection> conn,
	const boost::system::error_code& error
)
{
	// Immediately start accepting new connections, except when we're cancelled or our socket is closed.
	if ( error != boost::asio::error::operation_aborted && acceptor->is_open() )
		RPCListen(acceptor, context, use_ssl);
	
	rpch_params	params;

	/** @todo Actually handle errors in RpcAcceptHandler */
	if ( error )
	{
		
	}
	else if ( conn && !ClientAllowed(conn->peer.address()) )
	{
		if ( !use_ssl )
		{
			// Only send a 403 if we're not using SSL to prevent a DoS during the SSL handshake.
			conn->stream() << runtime.RPC()->HTTPReply(HTTP_FORBIDDEN, "", false) << std::flush;
		}

		LOG(ELogLevel::Info) << "Client was denied access.\n";
		return;
	}

	params.thisptr		= runtime.RPC();
	params.connection	= conn;

#if defined(_WIN32)
	params.thread_handle	= _beginthreadex(nullptr, 0, 
		runtime.RPC()->ExecRpcHandlerThread,
		&params, 
		CREATE_SUSPENDED,
		&params.thread_id);

	if ( params.thread_handle == 0 )
	{
		LOG(ELogLevel::Error) << "Failed to create the RPC Handler thread\n";
		return;
	}
	
	ResumeThread((HANDLE)params.thread_handle);

#else
	int32_t		err;
	pthread_attr_t	attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	err = pthread_create(&params.thread,
			     &attr,
			     runtime.RPC()->ExecRpcHandlerThread,
			     &params);

	if ( err != 0 )
	{
		LOG(ELogLevel::Error) << "Failed to create the RPC Handler thread; error "
					 << err << "\n";
		return;
	}
#endif	// _WIN32

	/* wait for the thread to reset this member; we don't want to exit scope
	 * before the thread has a chance to acquire the params contents */
	while ( params.thisptr != nullptr )
		SLEEP_MILLISECONDS(9);
}


std::string
rfc1123_time()
{
	char		buffer[64];
	std::string	locale(setlocale(LC_TIME, NULL));
	std::string	rfctime;
	
	setlocale(LC_TIME, "C"); // we want POSIX (aka "C") weekday/month strings
	rfctime = get_current_time_format(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S +0000");
	setlocale(LC_TIME, locale.c_str());
	
	return rfctime;
}










RpcServer::RpcServer()
{
}



RpcServer::~RpcServer()
{
}



#if defined(_WIN32)
	uint32_t
	__stdcall
#else
	void*
#endif
RpcServer::ExecRpcHandlerThread(
	void* params
)
{
	rpch_params*	tp = reinterpret_cast<rpch_params*>(params);

#if defined(_WIN32)
	return (uint32_t)tp->thisptr->RpcHandlerThread(tp);
#else
	tp->thisptr->RpcHandlerThread(tp);
	return nullptr;
#endif
}




#if defined(_WIN32)
	uint32_t
	__stdcall
#else
	void*
#endif
RpcServer::ExecRpcServerThread(
	void* params
)
{
	rpcs_params*	tp = reinterpret_cast<rpcs_params*>(params);

#if defined(_WIN32)
	return (uint32_t)tp->thisptr->RpcServerThread(tp);
#else
	tp->thisptr->RpcServerThread(tp);
	return nullptr;
#endif
}



std::string
RpcServer::HTTPReply(
	int status_code,
	const std::string& msg,
	bool keepalive
)
{
	std::stringstream	ss;
	std::string		retstr;
	std::string		version = APPLICATION_VERSION_STR;

	if ( status_code == HTTP_UNAUTHORIZED )
	{
		std::string	resp_401 =
			"<!DOCTYPE html>\r\n"
			"<html>\r\n"
			"<head>\r\n"
			"<title>Error</title>\r\n"
			"<meta http-equiv='Content-Type' content='text/html; charset=ISO-8859-1'>\r\n"
			"</head>\r\n"
			"<body><h1>401 Unauthorized.</h1></body>\r\n"
			"</html>\r\n"
			;
		ss << "HTTP/1.0 401 Authorization Required\r\n"
			<< "Date: " << rfc1123_time().c_str() << "\r\n"
			<< "Server: sbi-json-rpc/" << version.c_str() << "\r\n"
			<< "WWW-Authenticate: Basic realm=\"jsonrpc\"\r\n"
			<< "Content-Type: text/html\r\n"
			<< "Content-Length: " << resp_401.size() << "\r\n"
			<< "\r\n"
			<< resp_401.c_str();
	}
	else
	{
		char*		status_str;
		const char*	conn_type = keepalive ? "keep-alive" : "close";

		switch ( status_code )
		{
		case HTTP_OK:			status_str = "OK"; break;
		case HTTP_BAD_REQUEST:		status_str = "Bad Request"; break;
		case HTTP_FORBIDDEN:		status_str = "Forbidden"; break;
		case HTTP_NOT_FOUND:		status_str = "Not Found"; break;
		case HTTP_INTERNAL_SERVER_ERROR:status_str = "Internal Server Error"; break;
		default:
			status_str = "";
			break;
		}

		ss << "HTTP/1.1 " << status_code << " " << status_str << "\r\n"
			<< "Date: " << rfc1123_time().c_str() << "\r\n"
			<< "Connection: " << conn_type << "\r\n"
			<< "Content-Length: " << msg.size() << "\r\n"
			<< "Content-Type: application/json\r\n"
			<< "Server: sbi-json-rpc/" << version.c_str() << "\r\n"
			<< "\r\n"
			<< msg.c_str();
	}

	retstr = ss.str();
	return retstr;
}



json_spirit::Value
RpcServer::GetEnvironmentCoreCount(
	const json_spirit::Array& params,
	bool fHelp
)
{
	if ( fHelp || params.size() != 0 )
	{
		throw std::runtime_error(
			"GetEnvironmentCoreCount\n"
			"Returns the numer of concurrent threads (CPU cores) available"
		);
	}

	uint64_t	core_count;

	// use C++11 function, fallback to operating system independent
	if (( core_count = std::thread::hardware_concurrency()) == 0 )
	{
#if defined(_WIN32)
		SYSTEM_INFO	si;
		GetSystemInfo(&si);
		core_count = si.dwNumberOfProcessors;
#elif defined (__linux__)
		core_count = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(BSD)
		int32_t		mib[4];
		size_t		size = sizeof(core_count);

		mib[0] = CTL_HW;
		mib[1] = HW_AVAILCPU;
		sysctl(mib, 2, &core_count, &size, nullptr, 0);
		if ( core_count < 1 )
		{
			// if HW_AVAILCPU fails, fallback to HW_NCPU
			mib[1] = HW_NCPU;
			sysctl(mib, 2, &core_count, &size, nullptr, 0);

			if ( core_count < 1 )
				core_count = 1;
		}
#else
#	error incomplete for this operating system
#endif
	}

	return core_count;
}



json_spirit::Value
RpcServer::GetInterfaceInfo(
	const json_spirit::Array& params,
	bool fHelp
)
{
	params;
	fHelp;

	/** @todo implement RPC interface info acquisition */

	return 0;
}



uint32_t
RpcServer::ReadHTTPStatus(
	std::basic_istream<char>& stream,
	int& proto
)
{
	const char*	ver;
	std::string	str;
	std::vector<std::string>	words;

	getline(stream, str);
	boost::split(words, str, boost::is_any_of(std::string(" ")));
	
	if ( words.size() < 2 )
		return HTTP_INTERNAL_SERVER_ERROR;
	
	proto = 0;
	ver = strstr(str.c_str(), "HTTP/1.");
	
	if ( ver != nullptr )
	{
		proto = std::stoi(ver + 7);
	}

	// "POST / HTTP/1.1" will have no status (should be server only anyway..)
	// naturally causes exception here:
	//return std::stoi(words[1].c_str());
	return 200;
}



int32_t
RpcServer::ReadHTTPHeader(
	std::basic_istream<char>& stream,
	std::map<std::string, std::string>& headers
)
{
	int32_t	len = 0;

	for ( ;; )
	{
		std::string	str;
		std::getline(stream, str);
		if ( str.empty() || str == "\r" )
			break;
		std::string::size_type	colon = str.find(":");
		if ( colon != std::string::npos )
		{
			std::string	header = str.substr(0, colon);
			boost::trim(header);
			boost::to_lower(header);
			std::string	value = str.substr(colon + 1);
			boost::trim(value);
			headers[header] = value;

			if ( header == "content-length" )
				len = std::stoi(value.c_str());
		}
	}

	return len;
}



uint32_t
RpcServer::ReadHTTP(
	std::basic_istream<char>& stream,
	std::map<std::string, std::string>& headers,
	std::string& message
)
{
	headers.clear();
	message = "";

	// Read status
	int32_t		proto = 0;
	uint32_t	status = ReadHTTPStatus(stream, proto);

	// Read header
	int32_t		len = ReadHTTPHeader(stream, headers);

	if ( len < 0 || len > HTTP_MAX_CONTENT_LENGTH )
		return HTTP_INTERNAL_SERVER_ERROR;

	// if len = 0?

	// Read message
	if ( len > 0 )
	{
		std::vector<char>	vch(len);
		stream.read(&vch[0], len);
		message = std::string(vch.begin(), vch.end());
	}

	std::string	conn_header = headers["connection"];

	if ( (conn_header != "close") && (conn_header != "keep-alive") )
	{
		if ( proto >= 1 )
			headers["connection"] = "keep-alive";
		else
			headers["connection"] = "close";
	}

	return status;
}



bool
RpcServer::HTTPAuthorized(
	std::map<std::string,
	std::string>& headers
)
{
	bool		ret = false;
	int		i;
	std::string	auth = headers["authorization"];

	if ( auth.substr(0, 6) != "Basic " )
		return ret;

	std::string	user_pass64 = auth.substr(6);

	boost::trim(user_pass64);

	char*		user_pass = base64(user_pass64.c_str(), user_pass64.length()+1, &i);

	// temporary until we actually add assigning rpc auth in the config
	_rpc_auth = user_pass;
	ret = timing_resistant_equal(std::string(user_pass), _rpc_auth);

	// all done with the encoded value; release the memory
	FREE(user_pass);

	return ret;
}



ERpcStatus
RpcServer::RpcHandlerThread(
	rpch_params* tparam
)
{
	/* Remember, this is a class thread function - using 'this' will not
	 * work, which is why we supply the 'thisptr' as an input parameter. */

	// input pointer won't live forever, copy the contents
	RpcServer*			thisptr;
	std::shared_ptr<thread_info>	ti;
	std::shared_ptr<AcceptedConnection>	conn;
	{
		thisptr			= tparam->thisptr;
		conn			= tparam->connection;

		ti.reset(new thread_info);
#if defined(_WIN32)
		ti->thread		= tparam->thread_id;
		ti->thread_handle	= tparam->thread_handle;
#else
		ti->thread		= tparam->thread;
#endif
		strlcpy(ti->called_by_function, __func__, sizeof(ti->called_by_function));
		rename_thread("rpchandler");
		runtime.AddManualThread(ti);
		
		// let the caller know we're done
		tparam->thisptr = nullptr;

		assert(thisptr != nullptr);
		assert(ti != nullptr);
		assert(conn != nullptr);
	}

	bool	stop = false;

	// loop until we're shutting down or processing is complete
	for ( ;; )
	{
		if ( thisptr->_shutdown || stop )
		{
			conn->close();
			break;
		}

		std::map<std::string,std::string>	headers;
		std::string				request;

		ReadHTTP(conn->stream(), headers, request);

		if ( headers.count("authorization") == 0 )
		{
			conn->stream() << HTTPReply(HTTP_UNAUTHORIZED, "", false) << std::flush;
		}
		if ( !HTTPAuthorized(headers) )
		{
			std::cerr << "Incorrect password attempt from %s\n" << conn->peer_address_to_string().c_str();
			SLEEP_MILLISECONDS(250);

			conn->stream() << HTTPReply(HTTP_UNAUTHORIZED, "", false) << std::flush;
			break;
		}
		if ( headers["connection"] == "close" )
			stop = true;

		JsonRpc		jrpc;

		try
		{
			// parse request
			json_spirit::Value	val_req;
			std::string		reply;

			if ( !read_string(request, val_req) )
			{
				throw JsonRpcError(ERpcStatus::ParseError, "Parse error");
			}

			if ( val_req.type() == json_spirit::obj_type )
			{
				jrpc.Parse(val_req);

				json_spirit::Value	result = _table.Execute(jrpc.method, jrpc.params);

				// send reply
				reply = jrpc.Reply(result, json_spirit::Value::null, jrpc.id);
			}
			else if ( val_req.type() == json_spirit::array_type )
			{
				// array of requests
				reply = jrpc.ExecBatch(val_req.get_array());
			}
			else
			{
				throw JsonRpcError(ERpcStatus::ParseError, "Top-level object parse error");
			}

			conn->stream() << HTTPReply(HTTP_OK, reply, !stop) << std::flush;
		}
		catch ( json_spirit::Object& obj_error )
		{
			jrpc.ErrorReply(conn->stream(), obj_error, jrpc.id);
			break;
		}
		catch ( std::exception& e )
		{
			jrpc.ErrorReply(conn->stream(), 
					JsonRpcError(ERpcStatus::ParseError, e.what()),
					jrpc.id);
			break;
		}
	}

	conn.reset();

	runtime.ThreadStopping(ti->thread, __func__);
	return ERpcStatus::Ok;
}



ERpcStatus
RpcServer::RpcServerThread(
	rpcs_params* tparam
)
{
	/* Remember, this is a class thread function - using 'this' will not
	 * work, which is why we supply the 'thisptr' as an input parameter. */

	/* input pointer will live forever, but to remain similar to the other
	 * client threads, we'll still copy the contents */
	RpcServer*			thisptr;
	std::shared_ptr<thread_info>	ti;
	{
		thisptr			= tparam->thisptr;

		ti.reset(new thread_info);
#if defined(_WIN32)
		ti->thread		= tparam->thread_id;
		ti->thread_handle	= tparam->thread_handle;
#else
		ti->thread		= tparam->thread;
#endif
		strlcpy(ti->called_by_function, __func__, sizeof(ti->called_by_function));
		rename_thread("rpcserver");
		runtime.AddManualThread(ti);

		// caller can proceed, we've done our log
		tparam->thisptr = nullptr;
	}



	// server exec


	/* pointer exists as long as the server exists, perfectly safe; to avoid
	 * spamming .get() everywhere below, barely legible as it is */
	boost::asio::io_service&	io_service = *thisptr->_io_service.get();
	std::string			errstr;
	bool				is_listening = false;
	const bool			loopback = true;
	const bool			use_ssl = false;
	boost_ip::address		bind_addr = loopback ?
		boost_ip::address_v6::loopback() :
		boost_ip::address_v6::any();
	boost_ssl::context		context(io_service, boost_ssl::context::sslv23);
	boost_ip::tcp::endpoint		endpoint(bind_addr, RPC_PORT);
	boost::system::error_code	v6_only_error;
	std::shared_ptr<boost_ip::tcp::acceptor>	acceptor(
		new boost_ip::tcp::acceptor(io_service)
	);
	

	try
	{
		acceptor->open(endpoint.protocol());
		acceptor->set_option(boost_ip::tcp::acceptor::reuse_address(true));
		// try setting dual ipv6/v4
		acceptor->set_option(boost_ip::v6_only(loopback), v6_only_error);
		acceptor->bind(endpoint);
		acceptor->listen(boost::asio::socket_base::max_connections);

		RPCListen(acceptor, context, use_ssl);
		is_listening = true;
	}
	catch ( boost::system::system_error& e )
	{
		errstr = BUILD_STRING(
			"Error setting up RPC port ", 
			std::to_string(RPC_PORT).c_str(),
			" for listening on IPv6; falling back to IPv4: ",
			e.what()
		);
		LOG(ELogLevel::Error) << errstr.c_str() << "\n";
	}

	if ( !is_listening || loopback || v6_only_error )
	{
		try
		{
			bind_addr = loopback ?
				boost_ip::address_v4::loopback() :
				boost_ip::address_v4::any();
			endpoint.address(bind_addr);

			acceptor.reset(new boost_ip::tcp::acceptor(io_service));
			acceptor->open(endpoint.protocol());
			acceptor->set_option(boost_ip::tcp::acceptor::reuse_address(true));
			acceptor->bind(endpoint);
			acceptor->listen(boost::asio::socket_base::max_connections);

			RPCListen(acceptor, context, use_ssl);
			is_listening = true;
		}
		catch ( boost::system::system_error& e )
		{
			errstr = BUILD_STRING(
				"Error setting up RPC port ",
				std::to_string(RPC_PORT).c_str(),
				" for listening on IPv4: ",
				e.what()
			);
			LOG(ELogLevel::Error) << errstr.c_str() << "\n";
		}
	}

	// if we're not listening on a port at all, return failure
	if ( !is_listening )
	{
		LOG(ELogLevel::Error) << "Not listening on any port; startup failure\n";
		runtime.ThreadStopping(ti->thread, __func__);
		return ERpcStatus::NotListening;
	}

	// otherwise, enter the server loop
	while ( !thisptr->_shutdown )
	{
		// blocks until signalled
		io_service.run_one();
	}

	// RPC server shutting down; close acceptors and end the thread
	acceptor->close();

	runtime.ThreadStopping(ti->thread, __func__);
	return ERpcStatus::Ok;
}



ERpcStatus
RpcServer::Shutdown()
{
	if ( _shutdown )
	{
		// is already shutdown, can't repeat (_io_service is a nullptr!)
		return ERpcStatus::IsShutdown;
	}

	_shutdown = true;
	_io_service->stop();

	// wait for other threads to finish
#if defined(_WIN32)

	

	/* 5 seconds to wait for the thread before we just bail. Don't do error
	 * handling here, the thread is either deadlocked already, or simply
	 * doesn't exist. */
	WaitForSingleObject((HANDLE)_server_params.thread_handle, 5000);

#else
#endif

	_io_service.release();

	LOG(ELogLevel::Info) << "RPC Server has finished shutdown operations.\n";
	return ERpcStatus::Ok;
}



ERpcStatus
RpcServer::Startup()
{
	// set here in case of a prior call to Shutdown()
	_shutdown = false;
	memset(&_server_params, 0, sizeof(_server_params));
	_io_service.reset(new boost::asio::io_service);


	/** @todo Support socks|proxy|tor (also applies to IRC, Twitter, etc.).
	 * We will NEVER use UPNP. Don't even ask. */

	_server_params.thisptr = this;

#if defined(_WIN32)
	_server_params.thread_handle = _beginthreadex(
		nullptr, 0,
		ExecRpcServerThread,
		(void*)&_server_params, CREATE_SUSPENDED,
		&_server_params.thread_id
	);
	if ( _server_params.thread_handle == -1 )
	{
		std::cerr << fg_red << "_beginthreadex failed\n";
		LOG(ELogLevel::Error) << "_beginthreadex failed\n";
		return ERpcStatus::ThreadCreateFailed;
	}

	ResumeThread((HANDLE)_server_params.thread_handle);

#else
	int32_t		err;
	pthread_attr_t	attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	err = pthread_create(&_server_params.thread,
			     &attr,
			     ExecRpcServerThread,
			     &_server_params);

	if ( err != 0 )
	{
		LOG(ELogLevel::Error) << "Failed to create the RPC Server thread; error "
					 << err << "\n";
		return ERpcStatus::ThreadCreateFailed;
	}
#endif

	/* while _server_params lifetime is with the class and there's no need
	 * to wait for the thread, the thread will make a log entry that we want
	 * to report before the app_exec startup log goes through. End result is
	 * that looks just like all the other thread startups. */
	while ( _server_params.thisptr != nullptr )
		SLEEP_MILLISECONDS(4);

	return ERpcStatus::Ok;
}



void 
RpcServer::TypeCheck(
	const json_spirit::Array& params,
	const std::list<json_spirit::Value_type>& expected_types,
	bool allow_null
) const
{
	uint32_t	i = 0;
	BOOST_FOREACH(json_spirit::Value_type t, expected_types)
	{
		if ( params.size() <= i )
			break;

		const json_spirit::Value& v = params[i];
		if ( !((v.type() == t) || (allow_null && (v.type() == json_spirit::null_type))) )
		{
			std::string	err = BUILD_STRING(
				"Expected type ",
				value_type_to_string(t).c_str(),
				", got ",
				value_type_to_string(v.type()).c_str()
			);
			
			throw JsonRpcError(ERpcStatus::UnknownType, err);
		}
		i++;
	}
}



void 
RpcServer::TypeCheck(
	const json_spirit::Object& o,
	const std::map<std::string, json_spirit::Value_type>& expected_types,
	bool allow_null
) const
{
	std::string	err;

	BOOST_FOREACH(const PAIRTYPE(std::string, json_spirit::Value_type)& t, expected_types)
	{
		const json_spirit::Value& v = find_value(o, t.first);
		if ( !allow_null && v.type() == json_spirit::null_type )
		{
			err = BUILD_STRING("Missing ", t.first.c_str());

			throw JsonRpcError(ERpcStatus::UnknownType, err);
		}

		if ( !((v.type() == t.second) || (allow_null && (v.type() == json_spirit::null_type))) )
		{
			err = BUILD_STRING(
				"Expected type ",
				value_type_to_string(t.second).c_str(),
				" for ",
				t.first.c_str(),
				", got ",
				value_type_to_string(v.type()).c_str()
			);
					   
			throw JsonRpcError(ERpcStatus::UnknownType, err);
		}
	}
}




END_NAMESPACE

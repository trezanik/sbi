
/**
 * @file	RpcServer.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <thread>		// hardware_concurrency

#if defined(USING_JSON_SPIRIT_RPC)
#	include <boost/foreach.hpp>
#endif
#if defined(USING_BOOST_NET)
#	include <boost/asio.hpp>
#	include <boost/asio/ssl.hpp>
#	include <boost/iostreams/concepts.hpp>
#	include <boost/iostreams/stream.hpp>
// ironically requires OpenSSL for the ssl functionality anyway
#endif

#if defined(_WIN32)
#	include <Windows.h>
#	include <process.h>
#endif

#include "RpcServer.h"
#include "Runtime.h"
#include "Log.h"
#include "Terminal.h"
#include "utils.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



// special function; can't be class member as the return value is for throwing
json_spirit::Object
JSONRPCError(
	ERpcStatus err_code,
	const std::string& message
)
{
	json_spirit::Object	error;
	error.push_back(json_spirit::Pair("code", (int64_t) err_code));
	error.push_back(json_spirit::Pair("message", message));
	return error;
}




//
// IOStream device that speaks SSL but can also speak non-SSL
//
template <typename Protocol>
class SSLIOStreamDevice : public boost::iostreams::device<boost::iostreams::bidirectional> 
{
public:
	SSLIOStreamDevice(
		boost::asio::ssl::stream<typename Protocol::socket> &streamIn, 
		bool fUseSSLIn
	) : stream(streamIn)
	{
		fUseSSL = fUseSSLIn;
		fNeedHandshake = fUseSSLIn;
	}

	void
	handshake(
		boost::asio::ssl::stream_base::handshake_type role
	)
	{
		if ( !fNeedHandshake )
			return;
		fNeedHandshake = false;
		stream.handshake(role);
	}
	std::streamsize
	read(
		char* s,
		std::streamsize n
	)
	{
		handshake(boost::asio::ssl::stream_base::server); // HTTPS servers read first
		return fUseSSL ?
			stream.read_some(boost::asio::buffer(s, n)) :
			stream.next_layer().read_some(boost::asio::buffer(s, n));
	}
	std::streamsize
	write(
		const char* s,
		std::streamsize n
	)
	{
		handshake(boost::asio::ssl::stream_base::client); // HTTPS clients write first
		return fUseSSL ?
			boost::asio::write(stream, boost::asio::buffer(s, n)) :
			boost::asio::write(stream.next_layer(), boost::asio::buffer(s, n));
	}
	bool 
	connect(
		const std::string& server,
		const std::string& port
	)
	{
		ip::tcp::resolver resolver(stream.get_io_service());
		ip::tcp::resolver::query query(server.c_str(), port.c_str());
		ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		ip::tcp::resolver::iterator end;
		boost::system::error_code error = boost::asio::error::host_not_found;
		while ( error && endpoint_iterator != end )
		{
			stream.lowest_layer().close();
			stream.lowest_layer().connect(*endpoint_iterator++, error);
		}
		if ( error )
			return false;
		return true;
	}

private:
	NO_CLASS_ASSIGNMENT(SSLIOStreamDevice);

	bool fNeedHandshake;
	bool fUseSSL;
	boost::asio::ssl::stream<typename Protocol::socket>& stream;
};

class AcceptedConnection
{
public:
	virtual ~AcceptedConnection()
	{
	}

	virtual std::iostream&
	stream() = 0;
	
	virtual std::string
	peer_address_to_string() const = 0;
	
	virtual void
	close() = 0;
};

template <typename Protocol>
class AcceptedConnectionImpl : public AcceptedConnection
{
public:
	AcceptedConnectionImpl(
		boost::asio::io_service& io_service,
		boost::asio::ssl::context &context,
		bool use_ssl
	)
	: ssl_stream(io_service, context),
	  _dev(ssl_stream, use_ssl),
	  _stream(_dev)
	{
	}

	std::iostream&
	stream() override
	{
		return _stream;
	}

	std::string
	peer_address_to_string() const override
	{
		return peer.address().to_string();
	}

	void
	close() override
	{
		_stream.close();
	}

	typename Protocol::endpoint				peer;
	boost::asio::ssl::stream<typename Protocol::socket>	ssl_stream;

private:
	SSLIOStreamDevice<Protocol>				_dev;
	boost::iostreams::stream<SSLIOStreamDevice<Protocol>>	_stream;
};

// Forward declaration required for RPCListen
template <typename Protocol, typename SocketAcceptorService>
static void
RPCAcceptHandler(
	std::shared_ptr<boost::asio::basic_socket_acceptor<Protocol, SocketAcceptorService>> acceptor,
	boost::asio::ssl::context& context,
	bool use_ssl,
	AcceptedConnection* conn,
	const boost::system::error_code& error
);

/**
 * Sets up I/O resources to accept and handle a new connection.
 */
template <typename Protocol, typename SocketAcceptorService>
static void 
RPCListen(
	std::shared_ptr<boost::asio::basic_socket_acceptor<Protocol, SocketAcceptorService>> acceptor,
	boost::asio::ssl::context& context,
	const bool use_ssl
)
{
	// Accept connection
	AcceptedConnectionImpl<Protocol>* conn = new AcceptedConnectionImpl<Protocol>(acceptor->get_io_service(), context, use_ssl);

	acceptor->async_accept(
		conn->ssl_stream.lowest_layer(),
		conn->peer,
		boost::bind(&RPCAcceptHandler<Protocol, SocketAcceptorService>,
		acceptor,
		boost::ref(context),
		use_ssl,
		conn,
		boost::asio::placeholders::error)
	);
}

/**
* Accept and handle incoming connection.
*/
template <typename Protocol, typename SocketAcceptorService>
static void
RPCAcceptHandler(
	std::shared_ptr<boost::asio::basic_socket_acceptor<Protocol, SocketAcceptorService>> acceptor,
	boost::asio::ssl::context& context,
	const bool use_ssl,
	AcceptedConnection* conn,
	const boost::system::error_code& error
)
{
	//vnThreadsRunning[THREAD_RPCLISTENER]++;

	// Immediately start accepting new connections, except when we're cancelled or our socket is closed.
	if ( error != boost::asio::error::operation_aborted && acceptor->is_open() )
	    RPCListen(acceptor, context, use_ssl);

	AcceptedConnectionImpl<boost::asio::ip::tcp>* tcp_conn = dynamic_cast< AcceptedConnectionImpl<boost::asioip::tcp>* >(conn);

	/** @todo Actually handle errors in RpcAcceptHandler */
	if ( error )
	{
		delete conn;
	}

	// Restrict callers by IP.  It is important to
	// do this before starting client thread, to filter out
	// certain DoS and misbehaving clients.
	else if ( tcp_conn && !ClientAllowed(tcp_conn->peer.address()) )
	{
		// Only send a 403 if we're not using SSL to prevent a DoS during the SSL handshake.
		if ( !use_ssl )
			conn->stream() << HTTPReply(HTTP_FORBIDDEN, "", false) << std::flush;
		delete conn;
	}

	// start HTTP client thread
	// SpawnServerClientThread(conn)
	else if ( !NewThread(ThreadRPCServer3, conn) )
	{
		printf("Failed to create RPC server client thread\n");
		delete conn;
	}

	//vnThreadsRunning[THREAD_RPCLISTENER]--;
}






RpcServer::RpcServer()
{

}



RpcServer::~RpcServer()
{

}



uint32_t
#if defined(_WIN32)
__stdcall
#endif
RpcServer::ExecServerThread(
	void* params
)
{
	rpcs_params*	tp = reinterpret_cast<rpcs_params*>(params);

	return (uint32_t)tp->thisptr->ServerThread(tp);
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



ERpcStatus
RpcServer::ServerThread(
	rpcs_params* tparam
)
{
	/* Remember, this is a class thread function - using 'this' will not
	 * work, which is why we supply the 'thisptr' as an input parameter. */

	// input pointer won't live forever, copy the contents
	RpcServer*	thisptr = tparam->thisptr;
	std::shared_ptr<thread_info>	ti;
	ti->called_by_function	= __func__;
	ti->thread		= tparam->thread_id;;
#if defined(_WIN32)
	ti->thread_handle	= tparam->thread_handle;
#endif
	runtime.AddManualThread(ti);
	// let the caller know we're done
	tparam->thisptr = nullptr;

	
	


	// needs to be shared object (put through thisptr)
	bool shutdown = true;

	// server exec
	const bool	loopback = true;
	const bool	use_ssl = false;
	boost::asio::io_service		io_service;
	boost::asio::ssl::context	context(io_service, boost::asio::ssl::context::sslv23);
	boost::asio::ip::address	bind_addr = loopback ?
		boost::asio::ip::address_v6::loopback() :
		boost::asio::ip::address_v6::any();
	boost::asio::ip::tcp::endpoint	endpoint(bind_addr, RPC_PORT);
	boost::system::error_code	v6_only_error;
	std::shared_ptr<boost::asio::ip::tcp::acceptor>	acceptor(
		new boost::asio::ip::tcp::acceptor(io_service)
	);
	bool		is_listening = false;
	std::string	errstr;

	try
	{
		acceptor->open(endpoint.protocol());
		acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
		// try setting dual ipv6/v4
		acceptor->set_option(boost::asio::ip::v6_only(loopback), v6_only_error);
		acceptor->bind(endpoint);
		acceptor->listen(boost::asio::socket_base::max_connections);

		RPCListen(acceptor, context, use_ssl);
		is_listening = true;
	}
	catch ( boost::system::system_error& e )
	{
		errstr = BUILD_STRING(
			"Error setting up RPC port ", 
			std::to_string(RPC_PORT),
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
				boost::asio::ip::address_v4::loopback() :
				boost::asio::ip::address_v4::any();
			endpoint.address(bind_addr);

			acceptor.reset(new boost::asio::ip::tcp::acceptor(io_service));
			acceptor->open(endpoint.protocol());
			acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
			acceptor->bind(endpoint);
			acceptor->listen(boost::asio::socket_base::max_connections);

			RPCListen(acceptor, context, use_ssl);
			is_listening = true;
		}
		catch ( boost::system::system_error& e )
		{
			errstr = BUILD_STRING(
				"Error setting up RPC port ",
				std::to_string(RPC_PORT),
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
	while ( !shutdown )
	{
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
	return ERpcStatus::Ok;
}



ERpcStatus
RpcServer::Startup()
{

	/** @todo Support socks|proxy|tor (also applies to IRC, Twitter, etc.).
	 * We will NEVER use UPNP. Don't even ask. */

	rpcs_params	tparam;

	tparam.thisptr = this;

#if defined(_WIN32)
	tparam.thread_handle = _beginthreadex(
		nullptr, 0,
		ExecServerThread,
		(void*)&tparam, CREATE_SUSPENDED,
		&tparam.thread_id
	);
	if ( tparam.thread_handle == -1 )
	{
		std::cerr << fg_red << "_beginthreadex failed\n";
		LOG(ELogLevel::Error) << "_beginthreadex failed\n";
		return ERpcStatus::ThreadCreateFailed;
	}

	ResumeThread((HANDLE)tparam.thread_handle);

	/* wait for the thread to reset this member; we don't want to exit scope
	 * before the thread has a chance to acquire the params contents */
	while ( tparam.thisptr != nullptr )
		SLEEP_MILLISECONDS(21);

#else
	pthread_create();
#endif

	return ERpcStatus::Ok;
}



void 
RpcServer::TypeCheck(
	const json_spirit::Array& params,
	const std::list<json_spirit::Value_type>& expected_types,
	bool fAllowNull
) const
{
	uint32_t	i = 0;
	BOOST_FOREACH(json_spirit::Value_type t, expected_types)
	{
		if ( params.size() <= i )
			break;

		const json_spirit::Value& v = params[i];
		if ( !((v.type() == t) || (fAllowNull && (v.type() == json_spirit::null_type))) )
		{
			std::string	err = BUILD_STRING(
				"Expected type ",
				value_type_to_string(t),
				", got ",
				value_type_to_string(v.type())
			);
			
			throw JSONRPCError(ERpcStatus::UnknownType, err);
		}
		i++;
	}
}



void 
RpcServer::TypeCheck(
	const json_spirit::Object& o,
	const std::map<std::string, json_spirit::Value_type>& expected_types,
	bool fAllowNull
) const
{
	std::string	err;

	BOOST_FOREACH(const PAIRTYPE(std::string, json_spirit::Value_type)& t, expected_types)
	{
		const json_spirit::Value& v = find_value(o, t.first);
		if ( !fAllowNull && v.type() == json_spirit::null_type )
		{
			err = BUILD_STRING("Missing ", t.first.c_str());

			throw JSONRPCError(ERpcStatus::UnknownType, err);
		}

		if ( !((v.type() == t.second) || (fAllowNull && (v.type() == json_spirit::null_type))) )
		{
			err = BUILD_STRING(
				"Expected type ",
				value_type_to_string(t.second),
				" for ",
				t.first.c_str(),
				", got ",
				value_type_to_string(v.type())
			);
					   
			throw JSONRPCError(ERpcStatus::UnknownType, err);
		}
	}
}




END_NAMESPACE

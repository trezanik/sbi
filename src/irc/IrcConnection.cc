
/**
 * @file	IrcConnection.cc
 * @author	James Warren
 * @copyright	James Warren, 2013-2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <assert.h>			// assertions

#if defined(_WIN32)
#	include <WS2tcpip.h>		// Winsock needed before openssl
#	include <process.h>		// _beginthreadex
#elif defined(__linux__)
#	include <signal.h>		// signals
#	include <bits/sigthread.h>	// pthread_kill
#	include <pthread.h>		// pthread creation
#	include <unistd.h>		// usleep
#	include <netdb.h>		// NI_MAXHOST
#	include <stdarg.h>		// variable args
#endif

#if defined(USING_OPENSSL_NET)
#	include <openssl/bio.h>		// openssl i/o socket
#	include <openssl/crypto.h>	// openssl crypto functions
#	include <openssl/err.h>		// openssl error codes/strings
#	include <openssl/pem.h>		// openssl requirement
#	include <openssl/ssl.h>		// openssl requirement
#	include <openssl/x509.h>	// openssl cert functions
#endif

// for performing DNS lookups, can use Poco libraries or our own code
#if defined(USING_BOOST_NET)
#	include <boost/asio/ip/tcp.hpp>
	using namespace boost::asio;
#else
#	include "nethelper.h"		// DNS lookup functionality
#endif

#include <api/utils.h>			// utility functions
#include <api/Terminal.h>		// console output
#include <api/Log.h>
#include <api/Runtime.h>
#include "IrcConnection.h"		// prototypes
#include "IrcEngine.h"
#include "IrcNetwork.h"
#include "IrcChannel.h"
#include "IrcListener.h"
#include "IrcParser.h"
#include "IrcPool.h"
#include "IrcFactory.h"
#include "config_structs.h"




BEGIN_NAMESPACE(APP_NAMESPACE)



IrcConnection::IrcConnection(
	std::shared_ptr<IrcNetwork> network
) : _owner(network)
{
#if defined(USING_OPENSSL_NET)
	_socket = nullptr;
	_ssl = nullptr;
	_ssl_context = nullptr;
#endif
	_bytes_recv = 0;
	_bytes_sent = 0;
	_state = CS_Disconnected;

	_last_data = 0;
	_lag_sent = 0;

	_thread = 0;

	// and the parameters

#if !defined(USING_BOOST_NET)
	memset(&_params.ip, 0, sizeof(_params.ip));
	memset(&_params.sa, 0, sizeof(_params.sa));
#endif
}



IrcConnection::~IrcConnection()
{
	Cleanup();
}



EIrcStatus
IrcConnection::AddChannel(
	const char* channel_name
)
{
	if ( channel_name == nullptr )
		goto no_name;

	{
		std::lock_guard<std::mutex>	lock(_mutex);
		
		if ( _irc_engine->Factory()->CreateIrcChannel(
			_irc_engine->Pools()->GetConnection(_id), channel_name) 
			!= nullptr )
		{
			_channel_list.insert(channel_name);
		}
	}

	return EIrcStatus::OK;

no_name:
	std::cerr << fg_red << "The supplied channel name was a nullptr\n";
	return EIrcStatus::InvalidParameter;
}



EIrcStatus
IrcConnection::AddToRecvQueue(
	const char* data
)
{
	uint32_t	length = 0;

	if ( data == nullptr )
		goto no_data;

	length = strlen(data);

	/* if truncation occurs, the received message is longer than the known 
	 * IRC RFC limit, and is therefore invalid. We have already stripped the
	 * CR-LF so do not include that in the size check */
	if ( length >= MAX_BUF_IRC_MSG )
		goto data_too_long;
	if ( length < 2 )
		goto data_too_short;

	{
		std::lock_guard<std::mutex>	lock(_mutex);

		// append the new data to the receiving queue as a copy
		_recv_queue.push(data);
	}

	// Debug log
	LOG(ELogLevel::Debug) << "Recv on " << this << ": " << data << "\n";

	return EIrcStatus::OK;

no_data:
	std::cerr << fg_red << "The supplied data was a nullptr\n";
	return EIrcStatus::InvalidParameter;
data_too_long:
	std::cerr << fg_red << "The supplied data exceeded the maximum buffer size for an IRC message\n";
	return EIrcStatus::InvalidData;
data_too_short:
	std::cerr << fg_red << "The supplied data was too short for a valid IRC message\n";
	return EIrcStatus::InvalidData;
}



EIrcStatus
IrcConnection::AddToSendQueue(
	const char* data
)
{
	/** @todo existing functionality is temporary - flood protection! */

	SendBypass("%s\r\n", data);

	return EIrcStatus::OK;
}



EIrcStatus
IrcConnection::AutoChangeNick()
{
	config_profile*	profile;
	std::string	next_nick;
	bool		use = false;
	std::shared_ptr<IrcNetwork>	network = _owner.lock();

	if ( network == nullptr )
		goto no_parent;

	profile = &network->_profile_config;

	for ( auto n : profile->nicknames )
	{
		if ( use )
		{
			next_nick = n;
			break;
		}

		if ( n.compare(network->ClientNickname()) == 0 )
		{
			use = true;
		}
	}

	if ( next_nick.empty() )
	{
		// if use is true, then the current nick was the final one
		if ( use )
			goto no_more_nicks;
		// otherwise, we have a non-profile nick; so use the first again
		next_nick = profile->nicknames[0];
	}

	network->_client.nickname = next_nick;
	SendNick(next_nick.c_str());

	return EIrcStatus::OK;

no_parent:
	std::cerr << fg_red << "The supplied connections parent network was a nullptr\n";
	return EIrcStatus::NoOwner;
no_more_nicks:
	std::cerr << fg_red << "There are no more nicknames left to try\n";
	return EIrcStatus::NoMoreNicks;
}



EIrcStatus
IrcConnection::Cleanup()
{
	// the order of cleanup here should be the most stable/suitable

	if ( _state & CS_Active )
	{
		/* we can't use _owner->_client.quit_reason.c_str() here,
		 * as the parent network has already been deleted if we're closing
		 * the application. Yes, we can copy it; or move the structs into
		 * this class; until we're fully aware of all the intricate bits,
		 * yes send a quit without a message. Not that important... */
		SendQuit();
		_state = CS_Disconnecting;
	}

	/* we can safely wipe out the channel list while we're waiting for the
	 * server to close the connection and the thread to return */
	EraseChannelList();


#if 0	// Code Removed: now handled by Runtime
#if defined(_WIN32)
	if ( _thread != nullptr && _thread != INVALID_HANDLE_VALUE )
	{
		DWORD	exit_code = 0;

		/* give the thread 1 second to close */
		WaitForSingleObject(_thread, 1000);

		if ( !GetExitCodeThread(_thread, &exit_code) || exit_code == STILL_ACTIVE )
		{
			/* tried to let the thread go peacefully - kill it */
			if ( !TerminateThread(_thread, EXIT_FAILURE) )
			{
				/* termination failed - we're very likely to crash if the
				 * thread tries to resume at the blocking BIO_read, since
				 * we're about to free the ssl & class data.. */
				std::cerr << fg_red << "Failed to terminate the connection thread; Win32 error " << GetLastError();
			}
		}

		/* we want nothing to do with the thread anymore; we've tried to kill
		 * it (even if it succeeded) and so we can close the handle, and the
		 * system should cleanup all the associated resources for it when it is
		 * able to do so. */
		CloseHandle(_thread);
		_thread = nullptr;
		_thread_id = 0;
	}
#elif defined(__linux__) || defined(BSD)

	/* this cleanup code has been copied and used in the IrcParser and
	 * IrcAntiTimeout classes; modifications should be duplicated there;
	 * either that or we call a function that does it....recommended! */

	if ( _thread != 0 )
	{
		s32		ret;
		timespec	wait_time;

		wait_time.tv_sec = 1;

		/* wait 1 second for the thread to finish up */
		ret = pthread_timedjoin_np(_thread, nullptr, &wait_time);

		if ( ret == ETIMEDOUT )
		{
			/* tried to let the thread go peacefully - stop it */
			pthread_cancel(_thread);
			/* wait again */
			ret = pthread_timedjoin_np(_thread, nullptr, &wait_time);

			if ( ret == ETIMEDOUT )
			{
				std::cout << fg_red << "Thread " << _thread << " has been forcibly killed after failing to finish on request\n";

				/* second timeout, even after cancelling. Just
				 * kill it and live with any resource leaks */
				pthread_kill(_thread, SIGKILL);
			}
			else if ( ret != 0 )
			{
				std::cout << fg_red << "Received errno " << ret << " after waiting for thread " << _thread << " to finish\n";
			}
		}

		_thread = 0;
	}
#endif
#endif	// Code Removed

#if defined(USING_OPENSSL_NET)
	/* the EstablishConnection thread will still be running (assuming we
	 * actually connected) at this stage; and since it's blocking on the BIO
	 * the only way we can sync it is to delete it. As such, perform the
	 * deletion, and **wait** for the thread to finish before returning from
	 * this function. */
	if ( _socket != nullptr )
	{
		if ( _ssl != nullptr )
		{
			SSL_shutdown(_ssl.release());
			SSL_CTX_free(_ssl_context.release());
			// frees the connection->socket too, no need to reset
			SSL_free(_ssl.release());
			_ssl = nullptr;
			_ssl_context = nullptr;
		}
		else
		{
			BIO_reset(_socket.get());
			BIO_free(_socket.release());
		}

		_socket = nullptr;
	}
#endif	// USING_OPENSSL_NET

	runtime.WaitThenKillThread(_thread);

	// theoretically possible for a queue to add entries inbetween above

	while ( !_send_queue.empty() )
		_send_queue.pop();
	while ( !_recv_queue.empty() )
		_recv_queue.pop();

	_last_data = 0;
	_lag_sent = 0;
	_state = CS_Disconnected;

	return EIrcStatus::OK;
}



#if defined(USING_OPENSSL_NET)

int32_t
IrcConnection::ConnectToServer()
{
	/* SSL related only; attribute lengths are unspecified (min/max), so we
	 * make a best-guess attempt - 256 characters should be sufficient.. */
	char	subject[X509_MAX_ATTRIBUTE_LENGTH];
	char	issuer[X509_MAX_ATTRIBUTE_LENGTH];
	char	not_before[X509_MAX_ATTRIBUTE_LENGTH];
	char	not_after[X509_MAX_ATTRIBUTE_LENGTH];
	char	digest[32];
	char	fp[EVP_MAX_MD_SIZE * 3];
	BIO*	bio = nullptr;
	time_t	curtime = time(nullptr);
	bool	expired = false;
	bool	not_yet_valid = false;
	bool	no_fingerprint = false;


	enum E_DIGEST_TYPE
	{
		DT_None = 0,
		DT_MD5,
		DT_SHA1,
		DT_SHA256,
		DT_SHA512
	} eDT;

	if ( _owner == nullptr )
		goto no_parent;

	if ( IsActive() )
		goto already_connected;

	/* If the connection fails or is cancelled, the socket must be freed! */

	if ( BIO_do_connect(_socket.get()) <= 0 )
	{
		if ( _ssl != nullptr )
			goto openssl_ssl_connect_failed;
		else
			goto openssl_connect_failed;
	}

	/* connection success, means the port requested is listening;
	 * unset disconnected, set connecting */
	_state &= ~CS_Disconnected;
	_state |= CS_Connecting;

	/* if using SSL, grab the remote certificate and validate it; for a
	 * more standards-normal approach, use the OCSP */
	if ( _ssl != nullptr )
	{
		X509*	remote_cert = SSL_get_peer_certificate(_ssl.get());
		int64_t	res;
		uint8_t	md[EVP_MAX_MD_SIZE];
		uint32_t	n;

		if ( remote_cert == nullptr )
			goto openssl_no_cert;

		subject[0] = '\0';
		issuer[0] = '\0';
		not_before[0] = '\0';
		not_after[0] = '\0';
		digest[0] = '\0';
		fp[0] = '\0';

		X509_NAME_oneline(X509_get_subject_name(remote_cert), subject, sizeof(subject));
		X509_NAME_oneline(X509_get_issuer_name(remote_cert), issuer, sizeof(issuer));

		bio = BIO_new(BIO_s_mem());

		if ( ASN1_TIME_print(bio, X509_get_notAfter(remote_cert)) )
		{
			if (( n = BIO_read(bio, not_after, sizeof(not_after))) > 0 )
				not_after[n] = '\0';
		}
		BIO_reset(bio);/** @todo causes computed value but unused - confirm if still needed */
		if ( ASN1_TIME_print(bio, X509_get_notBefore(remote_cert)) )
		{
			if (( n = BIO_read(bio, not_before, sizeof(not_before))) > 0 )
				not_before[n] = '\0';
		}

		if ( X509_cmp_time(X509_get_notAfter(remote_cert), &curtime) < 0 )
			expired = true;
		else if ( X509_cmp_time(X509_get_notBefore(remote_cert), &curtime) > 0 )
			not_yet_valid = true;

		BIO_free(bio);

		// largest known first, work our way down
		if ( X509_digest(remote_cert, EVP_sha512(), md, &n) )
		{
			eDT = DT_SHA512;
			strlcpy(digest, "SHA-512", sizeof(digest));
		}
		else if ( X509_digest(remote_cert, EVP_sha256(), md, &n) )
		{
			eDT = DT_SHA256;
			strlcpy(digest, "SHA-256", sizeof(digest));
		}
		else if ( X509_digest(remote_cert, EVP_sha1(), md, &n) )
		{
			eDT = DT_SHA1;
			strlcpy(digest, "SHA-1", sizeof(digest));
		}
		else if ( X509_digest(remote_cert, EVP_md5(), md, &n) )
		{
			eDT = DT_MD5;
			strlcpy(digest, "MD5", sizeof(digest));
		}
		else
		{
			// no fingerprint, or is an unimplemented hash
			eDT = DT_None;
			no_fingerprint = true;
		}

		/* if we found a digest and its length is less than what we have
		 * available for the fingerprint, convert it to a human-readable
		 * string */
		if ( eDT != DT_None && n < sizeof(fp) )
		{
			char	hex[] = "0123456789abcdef";
			uint32_t	i;

			// convert to human-readable string

			for ( i = 0; i < n; i++ )
			{
				fp[i*3 + 0] = hex[(md[i] >> 4) & 0xF];
				fp[i*3 + 1] = hex[(md[i] >> 0) & 0xF];
				fp[i*3 + 2] = i == n - 1 ? '\0' : ':';
			}
		}

		if (( res = SSL_get_verify_result(_ssl.get())) != X509_V_OK )
		{
			// always generate an error on an invalid certificate
			std::cerr << fg_red << "The certificate received from " <<
				_owner->_server.host << " [" <<
				_owner->_server.ip_address << "] was invalid\n"
				"\tSubject: " << subject << "\n"
				"\tIssuer: " << issuer << "\n"
				"\tStart Date: " << not_before << "\n"
				"\tExpires: " << not_after << "\n"
				"\tFingerprint: " << fp << "\n"
				"\tError: " << X509_verify_cert_error_string((long)res) << "\n";

			// if we allow invalid certs, proceed; otherwise, break

			if ( !_params.allow_invalid_cert )
				goto openssl_invalid_cert;

			std::cout << fg_grey << "Ignoring invalid certificate\n";
		}
	}

	// now ready to negotiate with the server (assuming it is an ircd)
	return 0;

	/** @todo generate irc_activity's from here, so errors can be output in
	 * the gui and not restricted to the cli (which can optionally be hidden
	 * , thereby leaving no cause of error) */
no_parent:
	std::cerr << fg_red << "The supplied connections parent network was a nullptr\n";
	return -1;
already_connected:
	std::cerr << fg_red << "Already connected!\n";
	return 1;
openssl_connect_failed:
	std::cerr << fg_red << "OpenSSL connect failed\n";
	ERR_print_errors_cb(&openssl_err_callback, NULL);
	BIO_free(_socket.release());
	return -1;
openssl_ssl_connect_failed:
	std::cerr << fg_red << "OpenSSL connect failed\n";
	ERR_print_errors_cb(&openssl_err_callback, NULL);
	goto openssl_cleanup;
openssl_no_cert:
	std::cerr << fg_red << "No certificate was received from the remote host\n";
	goto openssl_cleanup;
openssl_invalid_cert:
	std::cerr << fg_red << "The application is configured to disallow invalid certificates\n";
	goto openssl_cleanup;
openssl_cleanup:
	SSL_shutdown(_ssl.release());
	SSL_CTX_free(_ssl_context.release());
	SSL_free(_ssl.release());
	return -1;
}


#elif defined(USING_BOOST_NET)

EIrcStatus
IrcConnection::ConnectToServer()
{
	return EIrcStatus::OK;
}

#else


int32_t
IrcConnection::ConnectToServer()
{
#	error No implementation exists for this function with current project settings
}

#endif	// USING_OPENSSL



#if 0	// Code Removed: now handled by Runtime::CreateThread
bool
IrcConnection::CreateThread()
{
#if defined(_WIN32)
	if ( _thread != INVALID_HANDLE_VALUE && _thread != nullptr )
		goto exists;

	_thread = (HANDLE)_beginthreadex(nullptr, 0,
		ExecEstablishConnection, this,
		CREATE_SUSPENDED, (uintptr_t*)&_thread_id);

	if ( _thread == nullptr )
		goto creation_failure;

	ResumeThread(_thread);
	return true;

exists:
	std::cerr << fg_red << "The thread appears to already exist!\n";
	return false;
creation_failure:
	std::cerr << fg_red << "Thread creation failure; errno " << errno << "\n";
	return false;
#else
	int32_t		err;
	pthread_attr_t	attr;

	if ( _thread != 0 )
		goto exists;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	err = pthread_create(&_thread, &attr, ExecEstablishConnection, this);

	if ( err != 0 )
		goto creation_failure;

	return true;

exists:
	return false;
creation_failure:
	return false;
#endif
}
#endif	// Code Removed



EIrcStatus
IrcConnection::DeleteChannel(
	const char* channel_name
)
{
	bool	found = false;

	if ( channel_name == nullptr )
		goto no_channel;

	{
		std::lock_guard<std::mutex>	lock(_mutex);

		for ( auto c : _channel_list )
		{
			if ( c == channel_name )
			{
				found = true;
				_irc_engine->Pools()->IrcChannels()->FreeObject(
					_irc_engine->Pools()->GetChannel(_id, channel_name)
				);
				_channel_list.erase(channel_name);
				// must break out of the loop, iterator invalid
				break;
			}
		}
	}

	if ( !found )
		goto not_found;

	return EIrcStatus::OK;

no_channel:
	std::cerr << fg_red << "The supplied channel was a nullptr\n";
	return EIrcStatus::InvalidParameter;
not_found:
	std::cerr << fg_red << "The supplied channel (" << channel_name << ") was not found in the channel list\n";
	return EIrcStatus::ObjectNotFound;
}



EIrcStatus
IrcConnection::EraseChannelList()
{
	std::lock_guard<std::mutex>	lock(_mutex);

	// all channels should be at reference count 1 here
	for ( auto c : _channel_list )
	{
		_irc_engine->Pools()->IrcChannels()->FreeObject(GetChannel(c.c_str()));
	}

	_channel_list.clear();

	return EIrcStatus::OK;
}



uint32_t
IrcConnection::EstablishConnection()
{
	char	buffer[MAX_BUF_IRC_MSG_CRLF];
	char	store_buffer[MAX_BUF_IRC_MSG_CRLF] = { '\0' };
	char	delim[] = "\n";
	char*	last = nullptr;
	char*	p = nullptr;
	int32_t		buffer_read = 0;
	uint32_t	len;
	uint32_t	max_len = sizeof(buffer) - 1;

	if ( &_owner == nullptr )
		goto invalid_parent;

	// ConnectToServer must have opened the connection
	if ( !IsConnecting() )
		goto invalid_state;


	// store the thread id so the class can close it cleanly
#if defined(_WIN32)
	_thread = GetCurrentThreadId();
#else
	_thread = pthread_self();
#endif


	try
	{

	/*
	 * Different servers have different first responses. For example, Rizon
	 * brings back the following:
	 | :irc.shakeababy.net 439 * :Please wait while we process your connection.
	 | :irc.shakeababy.net NOTICE AUTH :*** Looking up your hostname...
	 | ...
	 * Whereas connecting to irc.freenode.net results in:
	 | :calvino.freenode.net NOTICE * :*** Looking up your hostname...
	 | ...
	 * ngIRCd even WAITS for US to send the first data!
	 |
	 * This makes it difficult to predict and determine at what stages we
	 * should proceed with sending the connection data, and handling an event
	 * when we connect. The only guarantee is that 001 will be received when
	 * the connection is successful.
	 *
	 * Since I discovered ngIRCd does not send any data on initial connection
	 * (at least by default), then we have to go through our init procedure
	 * before the first read, otherwise it'll simply timeout, since the parse
	 * function never gets called. As such, we have to maintain this for all
	 * the other servers too!
	 */
	SendInit();

	while ( IsActive() || IsConnecting() )
	{
#if defined(USING_OPENSSL_NET)
		if (( buffer_read = BIO_read(_socket.get(), buffer, max_len)) == 0 )
		{
			if ( !BIO_should_retry(_socket.get()) )
				goto bio_abort_0;
			else
			{
				// sleep for a second, then retry
				SLEEP_SECONDS(1);
				continue;
			}
		}
		else if ( buffer_read < 0 )
		{
			if ( _socket != nullptr && BIO_should_retry(_socket.get()) )
			{
				continue;
			}

			goto bio_abort;
		}
		else
#elif defined(USING_BOOST_NET)
		if ( 0 )
		{
		}
#else
#	error No implementation exisits for this function with current project settings
#endif
		{
			// successfully retrieved data
			_last_data = time(NULL);
			_bytes_recv += buffer_read;

			/* ensure the buffer is nul-terminated; only this thread accesses
			 * this buffer, copies are made for later usage. */
			buffer[buffer_read] = '\0';

			/* if it's a PING, reply straight away, bypassing the
			 * queue; otherwise, add it. More efficient checking
			 * it here than allocating memory for the queue */
			if ( strncmp(buffer, "PING :", 6) == 0 )
			{
				// Turn PING into PONG
				buffer[1] = 'O';
				SendBypass("%s", buffer);
				continue;
			}
			/* we don't handle PONG's here (replies from our PING LAG12345) as it
			 * contains the server prefix like a normal message */

			/* reuse the same buffer that will be used for storing the 'prev'
			 * data, and make a copy of each instance. This eases debugging,
			 * and benefits the add_to_recv_queue functionality by making it
			 * do less work, at the cost of copying the string here - which
			 * will still be faster. */
			p = str_token(buffer, delim, &last);

			/* !!NOTE:: the str_token call will nul the newline 
			 * character, leaving only the linefeed behind, which we
			 * check for (and remove) */

			while ( p != nullptr )
			{
				if ( store_buffer[0] != '\0' )
				{
					// linefeed is 1 before nul
					len = strlen(p);
					if ( len < 1 || p[len-1] != '\r' )
						goto invalid_data;

					// nul the linefeed
					p[len-1] = '\0';
					// append the rest of the string to the previous buffer
					strlcat(store_buffer, p, sizeof(store_buffer));
					// and add it to the queue as normal
					AddToRecvQueue(store_buffer);
					store_buffer[0] = '\0';
				}
				else
				{
					len = strlcpy(store_buffer, p, sizeof(store_buffer));

					/* if linefeed is the last character, we've received a full
					 * message, otherwise retain it in the store_buffer ready
					 * for the next batch of data */
					if ( len > 1 && store_buffer[len-1] == '\r' )
					{
						// remove the linefeed
						store_buffer[len-1] = '\0';
						// add the whole string to the queue
						AddToRecvQueue(store_buffer);
						// whole message, start fresh with next iteration
						store_buffer[0] = '\0';
					}
				}

				p = str_token(nullptr, delim, &last);
			}

			/* do the notifications here, on end of data instance,
			 * rather than AddToRecvQueue - potential race condition
			 * with the queues otherwise? */

			// notify the pre-parse listeners there is new data available
			_irc_engine->NotifyListeners(LN_NewData, _irc_engine->Pools()->GetConnection(_id));
			// trigger the parser
			_irc_engine->Parser()->TriggerSync();
		}
	} // end while

	}
	catch ( std::exception& e )
	{
		std::cerr << fg_red << "Caught an exception; " << e.what() << "\n";
	}
	catch ( ... )
	{
		std::cerr << fg_red << "Caught an unhandled exception\n";
	}

	goto finish;

invalid_parent:
	std::cerr << fg_red << "The supplied connections parent_network was a nullptr\n";
	goto finish;
invalid_data:
	std::cerr << fg_red << "The received data was invalid\n";
	goto finish;
invalid_state:
	std::cerr << fg_red << "The connection state is invalid\n";
	goto finish;
#if defined(USING_OPENSSL_NET)
bio_abort:
	// as this thread blocks on the socket, it can be deleted legitimately
	if ( _socket != nullptr )
		std::cerr << fg_red << "BIO_should_retry returned false\n";
	goto finish;
bio_abort_0:
	if ( _socket != nullptr )
		std::cerr << fg_red << "BIO_should_retry returned false\n";
	goto finish;
#elif defined(USING_BOOST_NET)
#else
	// non-OpenSSL equivalent
#endif
finish:
	_state = CS_Disconnected;

#if defined(_WIN32)
	runtime.ThreadStopping(GetCurrentThreadId(), __FUNCTION__);
#else
	runtime.ThreadStopping(pthread_self(), __FUNCTION__);
#endif
	return 0;
}



#if defined(_WIN32)
uint32_t
#	if IS_VISUAL_STUDIO
	__stdcall
#	elif IS_GCC
	__attribute__((stdcall))
#	else
#		error "Unsupported compiler; this requires stdcall"
#	endif
#elif defined(__linux__) || defined(BSD)
void*
#endif
IrcConnection::ExecEstablishConnection(
	void* connection
)
{
#if defined(_WIN32)
	return ((IrcConnection*)connection)->EstablishConnection();
#else
	((IrcConnection*)connection)->EstablishConnection();
	return nullptr;
#endif
}



std::shared_ptr<IrcChannel>
IrcConnection::GetChannel(
	const char* channel_name
)
{
	if ( channel_name == nullptr )
		goto invalid_name;

	{
		std::lock_guard<std::mutex>	lock(_mutex);

		for ( auto c : _channel_list )
		{
			if ( strcmp(c.c_str(), channel_name) == 0 )
			{
				return _irc_engine->Pools()->GetChannel(_id, channel_name);
			}
		}
	}

	/* channel not found. Do not generate an error; this can be (and is used
	 * for) locating if a channel exists or not; the caller must raise an
	 * error if it's required */
	return nullptr;

invalid_name:
	std::cerr << fg_red << "The supplied channel name was a nullptr\n";
	return nullptr;
}



std::string
IrcConnection::GetCurrentNickname() const
{
	std::lock_guard<std::mutex>	lock(_mutex);
	std::shared_ptr<IrcNetwork>	net = _owner.lock();
	return net == nullptr ? "" : net->_client.nickname;
}



std::string
IrcConnection::GroupName() const
{
	std::lock_guard<std::mutex>	lock(_mutex);
	std::shared_ptr<IrcNetwork>	net = _owner.lock();
	return net == nullptr ? "" : net->_group_name;
}



bool
IrcConnection::IsActive()
{
	return (_state & CS_Active) != 0;
}



bool
IrcConnection::IsConnecting()
{
	return (_state & CS_Connecting) != 0;
}



bool
IrcConnection::IsDisconnected()
{
	return (_state & CS_Disconnected) != 0;
}



bool
IrcConnection::IsDisconnecting()
{
	return (_state & CS_Disconnecting) != 0;
}



std::string
IrcConnection::NetworkName() const
{
	std::lock_guard<std::mutex>	lock(_mutex);
	std::shared_ptr<IrcNetwork>	net = _owner.lock();
	return net == nullptr ? "" : net->_server.network;
}



std::shared_ptr<IrcNetwork>
IrcConnection::Owner()
{
	return _owner.lock();
}



EIrcStatus
IrcConnection::SendAway(
	const char* message
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( message != nullptr )
	{
		str_format(buffer, sizeof(buffer),
			"AWAY :%s",
			message);

		return AddToSendQueue(buffer);
	}
	else
	{
		return AddToSendQueue("AWAY : ");
	}
}



EIrcStatus
IrcConnection::SendBack()
{
	// unset our away status
	return AddToSendQueue("AWAY");
}



EIrcStatus
IrcConnection::SendBypass(
	const char* data_format,
	...
)
{
	char		buf[MAX_BUF_IRC_MSG_CRLF];
	va_list		args;
	int32_t		ret = 0;

	if ( data_format == nullptr )
		goto no_format;

	// the following is mostly a copy of the str_format utility function

	va_start(args, data_format);

#if MSVC_IS_VS8_OR_LATER
#	pragma warning ( push )
#	pragma warning ( disable : 4996 )	// variable or function may be unsafe
#endif

	// vsnprintf is fine when the buffer size is provided with -1 (for nul)
	ret = vsnprintf(buf, (sizeof(buf) - 1), data_format, args);

#if MSVC_IS_VS8_OR_LATER
#	pragma warning ( pop )
#endif

	va_end(args);

	/* if truncated, there will be no crlf and the server won't process it
	 * until the next one is provided! */

	if ( ret == -1 )
	{
		// destination text has been truncated
		buf[sizeof(buf)-1] = '\0';
		buf[sizeof(buf)-2] = '\n';
		buf[sizeof(buf)-3] = '\r';
		std::cout << fg_magenta << "Sending buffer truncated to read: " << buf << "\n";
	}
	else
	{
		/* to ensure nul-termination */
		buf[ret] = '\0';
	}

#if defined(USING_OPENSSL_NET)
	ret = BIO_puts(_socket.get(), buf);

	if ( ret <= 0 )
		goto openssl;
#else
#endif

	// Debug log, remove the cr+lf! (-2, not -3, array pos by strlen)
	buf[strlen(buf)-2] = '\0';
	LOG(ELogLevel::Debug) << "Sent on " << this << ": " << buf << "\n";

	_bytes_sent += ret;

	return EIrcStatus::OK;

no_format:
	std::cerr << fg_red << "The supplied data format was a nullptr\n";
	return EIrcStatus::MissingParameter;

#if defined(USING_OPENSSL_NET)
openssl:
	std::cerr << fg_red << "OpenSSL send error: " << ERR_error_string(ERR_get_error(), nullptr) << "\n";
	return EIrcStatus::OpenSSLError;
#endif

}



EIrcStatus
IrcConnection::SendCTCP(
	const char* target,
	const char* message
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( target == nullptr )
		goto no_target;
	if ( message == nullptr )
		goto no_message;

	str_format(buffer, sizeof(buffer),
		"PRIVMSG %s :\001%s\001",
		target, message);

	return AddToSendQueue(buffer);

no_target:
	return EIrcStatus::MissingParameter;
no_message:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendCTCPNotice(
	const char* destination,
	const char* message
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( destination == nullptr )
		goto no_dest;
	if ( message == nullptr )
		goto no_message;

	str_format(buffer, sizeof(buffer),
		"NOTICE %s :\001%s\001",
		destination, message);

	return AddToSendQueue(buffer);

no_dest:
	return EIrcStatus::MissingParameter;
no_message:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendInit()
{
	char		response[MAX_BUF_IRC_MSG];
	const char*	capabilities[] = { "multi-prefix", "uhnames" };
	const uint32_t	num_cap = sizeof(capabilities)/sizeof(char*);
	IrcNetwork*	network = std::shared_ptr<IrcNetwork>(_owner).get();
	uint32_t	len;
	uint32_t	i;
	char*		p = nullptr;
	EIrcStatus	retval;

	/* send our nick, user, and capability requests. If the server doesn't
	 * support them, there's not much we can do about it, so just dump all
	 * our initial requirements and handle the responses as best as possible. */

	_state |= CS_InitSent;

	if (( retval = SendBypass("CAP LIST\r\n")) != EIrcStatus::OK )
		return retval;

	/// @todo we should wait here, receive the replys, and REQ in response...

	/* use our copy/append trick to save using str_format for each
	 * capability */
	p = &response[(len = strlcpy(response, "CAP REQ ", sizeof(response)))];

	for ( i = 0; i < num_cap; i++ )
	{
		response[len] = '\0';
		strlcat(p, capabilities[i], sizeof(response)-len);

		// function will log all errors itself
		if (( retval = SendBypass("%s\r\n", response)) != EIrcStatus::OK )
			return retval;
	}

	if (( retval = SendBypass("CAP END\r\n")) != EIrcStatus::OK )
		return retval;

	/* use the first nickname in the profile configuration, unless
	 * we're reconnecting, in which case this is already set */
	if ( network->_client.nickname.empty() )
		network->_client.nickname = network->_profile_config.nicknames[0];

	/* We will not support 'PASS', as automatically sending a password
	 * across the wire is dangerous, especially so if we're not using SSL;
	 * put it down to the user if they want to send their pass plaintext */

	if (( retval = SendNick(network->_client.nickname.c_str())) != EIrcStatus::OK )
		return retval;

	/* user fields are non-modifiable once connected, so they are
	 * loaded from the profile configuration */
	return SendUser(
		network->_profile_config.ident.c_str(),
		network->_profile_config.mode,
		network->_profile_config.real_name.c_str());
}



EIrcStatus
IrcConnection::SendInvite(
	const char* channel_name,
	const char* nickname
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( channel_name == nullptr )
		goto no_channel;
	if ( nickname == nullptr )
		goto no_nickname;

	str_format(buffer, sizeof(buffer),
		"INVITE %s %s",
		nickname, channel_name);

	return AddToSendQueue(buffer);

no_channel:
	return EIrcStatus::MissingParameter;
no_nickname:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendJoin(
	const char* channel_name,
	const char* channel_key
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( channel_name == nullptr )
		goto no_channel;

	if ( channel_key == nullptr )
	{
		str_format(buffer, sizeof(buffer),
			"JOIN %s",
			channel_name);
	}
	else
	{
		str_format(buffer, sizeof(buffer),
			"JOIN %s %s",
			channel_name);
	}

	return AddToSendQueue(buffer);

no_channel:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendKick(
	const char* channel_name,
	const char* nickname,
	const char* msg
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( channel_name == nullptr )
		goto no_channel;
	if ( nickname == nullptr )
		goto no_nickname;

	if ( msg == nullptr )
	{
		str_format(buffer, sizeof(buffer),
			"KICK %s %s",
			channel_name, nickname);
	}
	else
	{
		str_format(buffer, sizeof(buffer),
			"KICK %s %s :%s",
			channel_name, nickname, msg);
	}

	return AddToSendQueue(buffer);

no_channel:
	return EIrcStatus::MissingParameter;
no_nickname:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendMode(
	const char* target,
	const char* mode
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( target == nullptr )
		goto no_target;
	if ( mode == nullptr )
		goto no_mode;

	str_format(buffer, sizeof(buffer),
		"MODE %s %s",
		target, mode);

	return AddToSendQueue(buffer);

no_target:
	return EIrcStatus::MissingParameter;
no_mode:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendNick(
	const char* nickname
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( nickname == nullptr )
		goto no_nick;

	str_format(buffer, sizeof(buffer),
		"NICK %s",
		nickname);

	return AddToSendQueue(buffer);

no_nick:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendIdentify(
	const char* service,
	const char* pass
)
{
#if 0	// useful code, might need to use some of these format strings for full support. Think this was from xchat
	// are all ircd authors idiots?
	switch (serv->nickservtype)
	{
	case 0:
		tcp_sendf (serv, "PRIVMSG NICKSERV :%s %s%s%s\r\n", cmd, arg1, arg2, arg3);
		break;
	case 1:
		tcp_sendf (serv, "NICKSERV %s %s%s%s\r\n", cmd, arg1, arg2, arg3);
		break;
	case 2:
		tcp_sendf (serv, "NS %s %s%s%s\r\n", cmd, arg1, arg2, arg3);
		break;
	case 3:
		tcp_sendf (serv, "PRIVMSG NS :%s %s%s%s\r\n", cmd, arg1, arg2, arg3);
		break;
	case 4:
		// why couldn't QuakeNet implement one of the existing ones?
		tcp_sendf (serv, "AUTH %s%s%s\r\n", cmd, arg1, arg2, arg3);
	}
#endif

	char	buffer[MAX_BUF_IRC_MSG];

	if ( service == nullptr )
		goto no_service;
	if ( pass == nullptr )
		goto no_pass;

	str_format(buffer, sizeof(buffer),
		"%s IDENTIFY %s",
		service, pass);

	return AddToSendQueue(buffer);

no_service:
	return EIrcStatus::MissingParameter;
no_pass:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendNotice(
	const char* target,
	const char* message
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( target == nullptr )
		goto no_target;
	if ( message == nullptr )
		goto no_message;

	str_format(buffer, sizeof(buffer),
		"NOTICE %s :%s",
		target, message);

	return AddToSendQueue(buffer);

no_target:
	return EIrcStatus::MissingParameter;
no_message:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendPart(
	const char* channel_name,
	const char* msg
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( channel_name == nullptr )
		goto no_channel;

	if ( msg == nullptr )
	{
		str_format(buffer, sizeof(buffer),
			"PART %s",
			channel_name);
	}
	else
	{
		str_format(buffer, sizeof(buffer),
			"PART %s :%s",
			channel_name, msg);
	}

	return AddToSendQueue(buffer);

no_channel:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendPrivmsg(
	const char* target,
	const char* privmsg
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	// do not support blank entries
	if ( target == nullptr )
		goto no_target;
	if ( privmsg == nullptr )
		goto no_message;

	str_format(buffer, sizeof(buffer),
		"PRIVMSG %s :%s",
		target, privmsg);

	// populate activity, notify? can't do this though, what if we're passing
	// a message through from the parser at the same time...

	return AddToSendQueue(buffer);

no_target:
	return EIrcStatus::MissingParameter;
no_message:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendRaw(
	const char* data
)
{
	return AddToSendQueue(data);
}



EIrcStatus
IrcConnection::SendQuit(
	const char* msg
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( msg == nullptr )
	{
		// embed our own quit message if none provided
		strlcpy(buffer, "QUIT :http://www.trezanik.org/", sizeof(buffer));
	}
	else
	{
		str_format(buffer, sizeof(buffer),
			"QUIT :%s",
			msg);
	}

	return AddToSendQueue(buffer);
}



EIrcStatus
IrcConnection::SendTopic(
	const char* channel_name,
	const char* topic
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( channel_name == nullptr )
		goto no_channel;

	if ( topic == nullptr )
	{
		// remove the channel topic
		str_format(buffer, sizeof(buffer),
			"TOPIC %s :\r\n",
			channel_name);
	}
	else if ( topic[0] != '\0' )
	{
		// set the channel topic
		str_format(buffer, sizeof(buffer),
			"TOPIC %s :%s\r\n",
			channel_name, topic);
	}
	else
	{
		// request the topic for the channel
		str_format(buffer, sizeof(buffer),
			"TOPIC %s\r\n",
			channel_name);
	}

	return AddToSendQueue(buffer);

no_channel:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::SendUser(
	const char* username,
	const uint16_t mode,
	const char* realname
)
{
	char	buffer[MAX_BUF_IRC_MSG];

	if ( username == nullptr )
		goto no_username;

	// if not set, real name is the same as the user/ident name
	if ( realname == nullptr )
		realname = username;

	/*
	rfc 1419 ::
	<username> <hostname> <servername> :<realname>
	rfc 2812 ::
	<user> <mode> <unused> :<realname>
	*/
	str_format(buffer, sizeof(buffer),
		"USER %s %d * :%s",
		username, mode, realname);

	return AddToSendQueue(buffer);

no_username:
	return EIrcStatus::MissingParameter;
}



EIrcStatus
IrcConnection::Setup(
	std::shared_ptr<IrcNetwork> network,
	std::shared_ptr<config_server> server_config
)
{
	int32_t	i;
	char	buffer[NI_MAXHOST];

	/* if a prior conn_str exists, we're reconnecting, so the network and
	 * server_config pointers are expected to be null. In case the IP has
	 * changed, we do want to do fresh lookups though, so don't skip this
	 * function! */
	if ( _params.conn_str[0] != '\0' )
		goto params_set;

	if ( network == nullptr )
		goto invalid_network;
	if ( server_config == nullptr )
		goto no_server;

	// copy the server data into the connection parameters
	_params.use_ssl	= server_config->ssl;
	_params.port	= server_config->port;
	_params.host	= server_config->hostname;
	_params.ip_addr	= server_config->ip_address;

params_set:

	// check if a host is specified first
	if ( _params.host.length() > 0 )
	{
		// setup the connection string to pass to OpenSSL
		str_format(buffer, sizeof(buffer),
			"%s:%u", _params.host.c_str(), _params.port);
		_params.conn_str = buffer;

		// verify we can identify an IP address for the host
#if defined(USING_BOOST_NET)
		io_service		io_service;
		ip::tcp::resolver	resolver(io_service);
		ip::tcp::resolver::query	query(_params.host);
		ip::tcp::endpoint	ep;
		/* we're only interested in one IP lookup, not all, so just
		 * use the first one that we resolve. */
		ep = *resolver.resolve(query);
		_params.data = ep.address().to_string();
#else
		i = host_to_ipv4(_params.host.c_str(), -1, buffer, sizeof(buffer));

		if ( i == -1 )
			goto lookup_failed;

		_params.data = buffer;
#endif
	}
	else if ( _params.ip_addr.length() > 0 )
	{
		str_format(buffer, sizeof(buffer),
			"%s:%u", _params.ip_addr.c_str(), _params.port);
		_params.conn_str = buffer;

		// reverse lookup for additional information
#if defined(USING_BOOST_NET)
		ip::tcp::endpoint	ep;
		ip::address_v4		ip = ip::address_v4::from_string(_params.ip_addr);
		io_service		ios;
		ip::tcp::resolver		res(ios);
		ip::tcp::resolver::iterator	dest;

		ep.address(ip);
		/* we're only interested in one hostname reverse lookup, not
		 * all, so just use the first one that we receive. */
		dest = res.resolve(ep);
		_params.data = dest->host_name();
#else
		ipv4_to_host(_params.ip_addr.c_str(), buffer, sizeof(buffer));
		_params.data = buffer;
#endif
	}
	else
	{
		goto no_server;
	}

	// reaching here, we must know an IP to connect to
	inet_pton(AF_INET, _params.ip_addr.c_str(), &_params.ip);

	// copy the data into the networks connection information
	network->UpdateServerInfo();

	if ( _params.use_ssl )
	{
#if defined(USING_OPENSSL_NET)
		// SSL connection requested
		SSL_CTX*	ssl_ctx = SSL_CTX_new(SSLv23_client_method());
		SSL*		ssl = nullptr;
		BIO*		socket = nullptr;

		if ( ssl_ctx == nullptr )
			goto openssl_context_failed;

		_ssl_context = std::unique_ptr<SSL_CTX>(ssl_ctx);

		socket = BIO_new_ssl_connect(ssl_ctx);

		if ( socket == nullptr )
			goto openssl_ssl_bio_failed;

		_socket = std::unique_ptr<BIO>(socket);

		BIO_get_ssl(_socket.get(), &ssl);
		_ssl = std::unique_ptr<SSL>(ssl);

		SSL_set_mode(_ssl.get(), SSL_MODE_AUTO_RETRY);
		BIO_set_conn_hostname(_socket.get(), _params.conn_str.c_str());
	}
	else
	{
		// non-SSL connection requested
		BIO*	socket = BIO_new_connect(const_cast<char*>(_params.conn_str.c_str()));
		if ( socket == nullptr )
			goto openssl_bio_failed;
		_socket.reset(socket);

#elif defined(USING_BOOST_NET)
		// SSL connection requested
	}
	else
	{
		// non-SSL connection requested
#else
#endif
	}

	// and anything else that needs putting across
	_params.allow_invalid_cert = network->_network_config.allow_invalid_cert;

	return EIrcStatus::OK;

invalid_network:
	std::cerr << fg_red << "The supplied irc_network was a nullptr\n";
	return EIrcStatus::MissingParameter;
no_server:
	std::cerr << fg_red << "There is no server specified by the input\n";
	return EIrcStatus::InvalidData;
lookup_failed:
	// already logged in function call
	return EIrcStatus::LookupFailed;

#if defined(USING_OPENSSL_NET)
openssl_context_failed:
	std::cerr << fg_red << "Failed to create the OpenSSL SSL context\n";
	return EIrcStatus::OpenSSLError;
openssl_ssl_bio_failed:
	std::cerr << fg_red << "Failed to create the OpenSSL SSL BIO\n";
	SSL_CTX_free(_ssl_context.release());
	return EIrcStatus::OpenSSLError;
openssl_bio_failed:
	std::cerr << fg_red << "Failed to create the OpenSSL BIO\n";
	return EIrcStatus::OpenSSLError;
#endif

}



END_NAMESPACE

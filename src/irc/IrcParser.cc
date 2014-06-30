
/**
 * @file	IrcParser.cc
 * @author	James Warren
 * @copyright	James Warren, 2013
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 */



#if defined(_WIN32)
#	include <process.h>		// _beginthreadex
#	include <WS2tcpip.h>
#elif defined(__linux__)
#	include <stdarg.h>		// variadic macros
#	include <stdlib.h>		// atoi
#	include <new>			// Allow overriding new
#	include <pthread.h>		// pthread_kill (sigthread.h)
#	include <signal.h>		// signals (SIGKILL)
#endif

#include <api/Runtime.h>
#include <api/Allocator.h>		// manual memory management
#include <api/Terminal.h>		// console output
#include <api/utils.h>			// utility functions
#include "IrcParser.h"			// prototypes
#include "IrcNetwork.h"
#include "IrcConnection.h"
#include "IrcChannel.h"
#include "IrcUser.h"
#include "IrcEngine.h"
#include "IrcPool.h"
#include "rfc1459.h"
#include "rfc2812.h"
#include "irc_channel_modes.h"		// channel modes/flags
#include "irc_user_modes.h"		// user modes/flags



BEGIN_NAMESPACE(APP_NAMESPACE)



IrcParser::IrcParser()
{
#if defined(_WIN32)

	_sync_event = CreateEvent(NULL, true, false, NULL);

	if ( _sync_event == nullptr )
	{
		std::cerr << fg_red << "Failed to create the parser synchronization event; Win32 error " << GetLastError() << "\n";
	}

#elif defined(__linux__) || defined(BSD)

	if ( !sync_event_construct(&_sync_event) )
	{
		// error already reported
	}

#endif
}



IrcParser::~IrcParser()
{
	Cleanup();
}



void
IrcParser::Cleanup()
{
#if 0	// Code Removed: thread handling done by the Runtime
#if defined(_WIN32)

	if ( _thread != nullptr && _thread != INVALID_HANDLE_VALUE )
	{
		DWORD	exit_code = 0;

		/* give the thread 1 second to close if it's doing something */
		WaitForSingleObject(_thread, 1000);

		if ( !GetExitCodeThread(_thread, &exit_code) || exit_code == STILL_ACTIVE )
		{
			/* tried to let the thread go peacefully - kill it */
			if ( !TerminateThread(_thread, EXIT_FAILURE) )
				std::cerr << fg_red << "Failed to terminate the parser thread; Win32 error " << GetLastError();
		}

		CloseHandle(_thread);
		_thread = nullptr;
		_thread_id = 0;
	}

#else

	/* copied from IrcConnection.cc::Cleanup() */
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
				generate_error_var_arg(EC_FunctionFailed, "Thread %d has been forcibly killed after failing to finish on request", _thread);

				/* second timeout, even after cancelling. Just
				 * kill it and live with any resource leaks */
				pthread_kill(_thread, SIGKILL);
			}
			else if ( ret != 0 )
			{
				generate_error_var_arg(EC_FunctionFailed, "Received errno %d after waiting for thread %d to finish", ret, _thread);
			}
		}

		_thread = 0;
	}
#endif
#endif	// Code Removed

#if defined(_WIN32)

	if ( _sync_event != nullptr )
		CloseHandle(_sync_event);

#elif defined(__linux__) || defined(BSD)

	if ( _sync_event.flag < 2 )
	{
		if ( !sync_event_destroy(&_sync_event) )
		{
			/* small resource leak on failure */
		}
	}

#endif
}



#if 0	// Code Removed: Now handled by Runtime::CreateThread
EIrcStatus
IrcParser::CreateThread()
{
#if defined(_WIN32)

	/* should only be called once, in initialization */

	_thread = (HANDLE)_beginthreadex(nullptr, 0,
		ExecParser, (void*)this,
		CREATE_SUSPENDED, (uintptr_t*)&_thread_id);

	if ( _thread == nullptr )
	{
		std::cerr << fg_red << "Failed to create the parser thread; Win32 error " << GetLastError() << "\n";
		return false;
	}

	_sync_event = CreateEvent(NULL, TRUE, FALSE, NULL);

	if ( _sync_event == nullptr )
	{
		std::cerr << fg_red << "Failed to create the parser synchronization event; Win32 error " << GetLastError() << "\n";
		/* has nothing to cleanup, has not started running yet */
		TerminateThread(_thread, EXIT_SUCCESS);
		_thread_id = 0;
		return false;
	}

	/* start the parser, return success */
	ResumeThread(_thread);
	return EIrcStatus::OK;


#elif defined(__linux__) || defined(BSD)

	s32		err;
	pthread_attr_t	attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	err = pthread_create(&_thread, &attr, ExecParseConnectionQueues, (void*)this);

	if ( err != 0 )
	{
		return EC_Errno;
	}
	else
	{
		/* create the synchronization object */

		if ( sync_event_construct(&_sync_event) != true )
		{
			// kill thread
			return EC_ApiFunctionFailed;
		}

		return EIrcStatus::OK;
	}

#endif
}
#endif	// Code Removed



EIrcStatus
IrcParser::ExtractIrcBufData(
	const char* buffer,
	ircbuf_data* data
) const
{
	char*	sender = (char*)&buffer[0];
	char*	code = nullptr;
	char*	code_data = nullptr;

	if ( buffer == nullptr )
		goto no_buffer;
	if ( data == nullptr )
		goto no_bufdata;

	// ignore the IRC standard prefix, if supplied
	if ( *sender == ':' )
		sender++;

	code = strchr(sender, ' ');

	if ( code == nullptr || *code == '\0' )
		goto missing_data;

	// nul-out the space, and proceed to the 'first' character
	*code = '\0';
	code++;

	// copy the data, up to the first space (which is now a nul)
	data->sender	= sender;

	code_data = strchr(code, ' ');

	if ( code_data == nullptr )
		goto missing_data;

	*code_data = '\0';

	if ( strlen(code) == 0 )
		goto missing_code;

	code_data++;

	data->code	= code;
	data->data	= code_data;

	return EIrcStatus::OK;

no_buffer:
	std::cerr << fg_red << "The supplied input buffer was a nullptr\n";
	return EIrcStatus::MissingParameter;
no_bufdata:
	std::cerr << fg_red << "The supplied output data store was a nullptr\n";
	return EIrcStatus::MissingParameter;
missing_data:
	std::cerr << fg_red << "Invalid buffer received; missing data: " << buffer << "\n";
	return EIrcStatus::InvalidParameter;
missing_code:
	std::cerr << fg_red << "Invalid buffer received; missing code: " << buffer << "\n";
	return EIrcStatus::InvalidParameter;
}



/* All the parser handler methods provide some sample data and their info.
 * The first line will be a 'Name' for the code, and wherever this particular
 * code originated from (https://www.alien.net.au/irc/irc2numerics.html) was a
 * great help here.
 *
 * After a spacer, each line details an example of a real connection data
 * return, dependent on the server *type* that sent it. Incomplete entries are
 * simply ones I've never tested or obtained to date. Note that every server can
 * be configured differently, so some results may vary, especially between IRC
 * daemons - there's a lot of inconsistency! I also can't fit every server here,
 * so don't bitch at me if it's not listed.
 *
 * Some results may expand multiple lines, so they'll be left-aligned.
 *
 * The examples all assume ircd.trezanik.org is the server name, the nick of the
 * user connection is 'trez', the ident 'tirc', and the hostmask that of my
 * development laptop, 'dev-laptop.trezanik.org'.
 */


EIrcStatus
IrcParser::Handle001(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	/* RPL_WELCOME [RFC2812]
	 *
	 * Bahamut:
	 * Hybrid:
	 * inspIRCd:	:ircd.trezanik.org 001 trez :Welcome to the Trezanik IRC Network trez!tirc@dev-laptop.trezanik.org
	 * ircd-seven:
	 * ircu:
	 * ngIRCd:	:ircd.trezanik.org 001 trez :Welcome to the Internet Relay Network trez!~tirc@dev-laptop.trezanik.org
	 * snircd:
	 * Unreal:
	 */

	std::shared_ptr<IrcNetwork>	network = connection->Owner();
	EIrcStatus	ret = EIrcStatus::Unknown;
	const char*	search = " :";
	char*		nick = nullptr;
	char*		nick_end = nullptr;
	char*		p = nullptr;
	irc_activity&	activity = connection->GetActivity();

	if ( network == nullptr )
		goto no_network;

	irc_client*	client = &network->_client;
	irc_server*	server = &network->_server;

	/* Our accessors to other variables is done by direct access, and not
	 * using the Get() methods (which cause a reference).
	 * This is because the connection, which is our route through to all the
	 * other variables (whether parents or children), is already referenced
	 * as part of the connection queue parsing loop.
	 * As such, this class will never be deleted (with a guarantee that the
	 * parent won't be either, if our design stays true), and the entire
	 * parser/listener/notifier setup is done in a single thread, so there
	 * should be no race conditions with plugins either.
	 */

	/* we have full private access to most Irc objects (see the documentation
	 * for this class as to the explanation!); this applies to all the other
	 * functions here too. */
	connection->_state &= ~CS_Connecting;
	connection->_state |= CS_Active;

	// we can retrieve our active nickname and the server name at the same time

	if (( nick_end = (char*)strstr(data->data.c_str(), search)) == nullptr )
		goto invalid_data;

	// nul-terminate so we can copy our nickname - data->data
	*nick_end = '\0';

	server->server	= sender->nickname;
	nick = (char*)data->data.c_str();

	/* first confirmation of our nickname; check to make sure that the one
	 * it says we are using is what we actually requested */
	if ( client->nickname.compare(nick) != 0 )
		goto nickname_mismatch;

	// rest of the data is still valid, to output to the user
	p = (nick_end + 1);
	// skip the colon
	p++;

	// prepare the activity data, then inform our listeners
	{
		activity.message	= p;
		activity.nickname	= nick;

		_irc_engine->NotifyListeners(LN_001, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

no_network:
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
invalid_data:
	std::cerr << fg_red << "The supplied data contains no nickname end: " << data->data << "\n";
	ret = EIrcStatus::InvalidData;
	goto cleanup;
nickname_mismatch:
	std::cerr << fg_red << "Nickname mismatch: Expected '" <<
		client->nickname << "', got '" << data->data << "'\n";
	ret = EIrcStatus::NickIsNotClient;
	goto cleanup;
cleanup:
	return ret;
}



EIrcStatus
IrcParser::Handle002(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	/* RPL_YOURHOST [RFC2812]
	 *
	 * Hybrid:	:ircd.trezanik.org 002 trez :Your host is ircd.trezanik.org[192.168.134.20/6697], running version hybrid-7.2.3+plexus-3.0.1(20100109_0-524)
	 * InspIRCd:	:ircd.trezanik.org 002 trez :Your host is ircd.trezanik.org, running version 2.0
	 * ngIRCd:	:ircd.trezanik.org 002 trez :Your host is ircd.trezanik.org, running version ngircd-20.3 (x86_64/unknown/linux-gnu)
	 */

	EIrcStatus	ret = EIrcStatus::Unknown;
	const char	search = ',';
	char*		host = nullptr;
	char*		host_end = nullptr;
	char*		p = nullptr;
	char*		p2 = nullptr;
	irc_server*	server = &connection->Owner()->_server;
	irc_activity&	activity = connection->GetActivity();


#if 0	/** @todo We can compare the host with live/config - but is storing necessary?? */
	/* IRC servers don't need to specify anything, so on 'failure' we still
	 * return true. Most I've seen retain a 'nice' format */
	if (( host_end = (char*)strchr(data->data.c_str(), search)) == nullptr )
		goto no_info;

	*host_end = '\0';

	if (( host = (char*)strrchr(data->data.c_str(), ' ')) == nullptr )
		goto no_info;

	// some ircds have the ip address + port alongside this string

	if (( p = strchr(host, '[')) != nullptr || ( p = strchr(host, '(')) != nullptr )
	{
		// appears the IP was specified
		*p = '\0';
		server->host	= host;
		p++;

		if (( p2 = strchr(p, '/')) != nullptr || ( p2 = strchr(p, ':')) != nullptr )
		{
			// appears the port was specified too - ignore (we have it already)
			*p2 = '\0';
		}
		server->ip_address	= p;
	}
	else
	{
		server->host	= host;
	}
#endif

#if 0	/** @todo implement LN_002 */
	// prepare the activity data, then inform our listeners
	{
		activity.message	= p;
		activity.nickname	= nick;

		_irc_engine->NotifyListeners(LN_002, connection);
	}
#endif

	ret = EIrcStatus::OK;
	goto cleanup;

no_info:
	std::cerr << fg_red << "The server did not supply an expected hostname string\n";
	ret = EIrcStatus::InvalidData;
	goto cleanup;
cleanup:
	return ret;
}



EIrcStatus
IrcParser::Handle003(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	/* RPL_CREATED [RFC2812]
	 *
	 * Hybrid:	:
	 * ngIRCd:	:ircd.trezanik.org 003 trez :This server has been started Sat Oct 19 2013 at 23:42:23 (BST)
	 */

	EIrcStatus	ret = EIrcStatus::Unknown;
	irc_activity&	activity = connection->GetActivity();
	const char*	p = strchr(data->data.c_str(), ':');	// skip our nick

	if ( p != nullptr )
		p++;

	// prepare the activity data, then inform our listeners
	{
		activity.message	= p;
		activity.instigator.nickname	= sender->nickname;

		_irc_engine->NotifyListeners(LN_003, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

cleanup:
	return ret;
}



EIrcStatus
IrcParser::Handle004(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	/* RPL_MYINFO [RFC2812]
	 *
	 * Hybrid:	:
	 * ngIRCd:	:ircd.trezanik.org 004 trez ircd.trezanik.org ngircd-20.3 abBcCioqrRswx abehiIklmMnoOPqQrRstvVz
	 */

	EIrcStatus	ret = EIrcStatus::Unknown;
	irc_activity&	activity = connection->GetActivity();
	const char*	p = strchr(data->data.c_str(), ' ');	// skip our nick

	if ( p != nullptr )
		p++;

	// prepare the activity data, then inform our listeners
	{
		activity.message	= p;
		activity.instigator.nickname	= sender->nickname;

		_irc_engine->NotifyListeners(LN_004, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

cleanup:
	return ret;
}



EIrcStatus
IrcParser::Handle005(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	/* RPL_ISUPPORT [draft-brocklesby-irc-isupport-03; de-facto standard, deprecates RFC2812 005 RPL_BOUNCE]
	 *
	 * InspIRCd:	:ircd.trezanik.org 005 trez AWAYLEN=200 CASEMAPPING=rfc1459 CHANMODES=b,k,l,imnpst CHANNELLEN=64 CHANTYPES=# CHARSET=ascii ELIST=MU FNC KICKLEN=255 MAP MAXBANS=60 MAXCHANNELS=20 MAXPARA=32 :are supported by this server
			:ircd.trezanik.org 005 trez MAXTARGETS=20 MODES=20 NETWORK=Trezanik NICKLEN=31 PREFIX=(ov)@+ STATUSMSG=@+ TOPICLEN=307 VBANLIST WALLCHOPS WALLVOICES :are supported by this server
	 * ngIRCd:	:ircd.trezanik.org 005 trez RFC2812 IRCD=ngIRCd CHARSET=UTF-8 CASEMAPPING=ascii PREFIX=(qaohv)~&@%+ CHANTYPES=#&+ CHANMODES=beI,k,l,imMnOPQRstVz CHANLIMIT=#&+:10 :are supported on this server
			:ircd.trezanik.org 005 trez CHANNELLEN=50 NICKLEN=9 TOPICLEN=490 AWAYLEN=127 KICKLEN=400 MODES=5 MAXLIST=beI:50 EXCEPTS=e INVEX=I PENALTY :are supported on this server
	 */

	std::shared_ptr<IrcNetwork>	network;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char		delim[] = " ";
	char		search = ' ';
	char*		p = nullptr;
	char*		str = nullptr;
	char*		last = nullptr;
	char*		nick_end = nullptr;
	char*		dup = nullptr;
	uint32_t	alloc;
	irc_activity&	activity = connection->GetActivity();

	if ( !connection->IsActive() )
		goto incorrect_state;

	network = connection->Owner();

	/* duplicate the buffer so str_token can modify it without affecting the
	 * original recv contents. */

	alloc = data->data.length() + 1;
	dup = (char*)MALLOC(alloc);
	
	if ( dup == nullptr )
		throw std::bad_alloc();

	strlcpy(dup, data->data.c_str(), alloc);
	str = dup;

	p = str_token(str, delim, &last);

	while ( p != nullptr )
	{
		// commented examples are from a standard connection to Rizon

		// end of the list for this message
		if ( *p == ':' )
			break;

		if ( strncmp(p, "NETWORK=", 8) == 0 )		// NETWORK=Rizon
			network->_server.network = (p+8);
		else if ( strncmp(p, "CHANTYPES=", 10) == 0 )	// CHANTYPES=#
			network->_server.chan_types = (p+10);
		else if ( strncmp(p, "NICKLEN=", 8) == 0 )	// NICKLEN=30
			network->_server.max_len_nick = (uint16_t)atoi((p+8));
		else if ( strncmp(p, "KICKLEN=", 8) == 0 )	// KICKLEN=160
			network->_server.max_len_kickmsg = (uint16_t)atoi((p+8));
		else if ( strncmp(p, "CHANNELLEN=", 11) == 0 )	// CHANNELLEN=50
			network->_server.max_len_channel = (uint16_t)atoi((p+11));
		else if ( strncmp(p, "AWAYLEN=", 8) == 0 )	// AWAYLEN=160
			network->_server.max_len_away = (uint16_t)atoi((p+8));
		else if ( strncmp(p, "MODES=", 6) == 0 )	// MODES=4
			network->_server.max_num_modes = (uint16_t)atoi((p+6));
		else if ( strncmp(p, "TOPICLEN=", 9) == 0 )	// TOPICLEN=390
			network->_server.max_len_topic = (uint16_t)atoi((p+9));
		else if ( strncmp(p, "PREFIX=", 7) == 0 )	// PREFIX=(qaohv)~&@%+
		{
			char*	psz = (p+7);
			size_t	len = strlen(psz);

			if ( psz == nullptr || *psz != '(' )
			{
				std::cerr << fg_red << "PREFIX is invalid; expected opening bracket in '" << p << "'\n";
				goto invalid_data;
			}
			while ( *psz != ')' && len )
			{
				psz++;
				--len;
			}
			if ( len == 0 )
			{
				std::cerr << fg_red << "PREFIX is invalid; no closing bracket in '" << p << "'\n";
				goto invalid_data;
			}
			if ( psz == (p+8) )
			{
				std::cerr << fg_red << "PREFIX is invalid; no prefixes specified in '" << p << "'\n";
				goto invalid_data;
			}

			// nul-terminate the prefix list
			*psz = '\0';
			// copy the contents, skipping the opening bracket
			network->_server.chan_mode_chars	= (p+8);
			// move up to the user modes
			if ( *++psz == '\0' )
			{
				std::cerr << fg_red << "PREFIX is invalid; no modes after prefix list in '" << p << "'\n";
				goto invalid_data;
			}
			// and copy these contents
			network->_server.chan_mode_symbols	= psz;
		}
		else if ( strncmp(p, "CHANLIMIT=", 10) == 0 )	// CHANLIMIT=#:75
		{
			//network->_server.max_num_channels = (u16)atoi((p+10));
		}
		else if ( strncmp(p, "CHANMODES=", 10) == 0 )	// CHANMODES=eIb,k,l,BMNORScimnpstz
		{
			char	buffer[56];
			char	cm_delim[] = ",";
			char*	psz = (p+10);
			char*	cm_last = nullptr;
			uint32_t	ui = 0;

			if ( strlen(psz) > 55 )  // a-z, A-Z, and 3 commas
			{
				std::cerr << fg_red << "CHANMODES data exceeds possible limit: '" << p << "'\n";
				goto invalid_data;
			}

			strlcpy(buffer, psz, sizeof(buffer));
			psz = str_token(buffer, cm_delim, &cm_last);

			while ( psz != nullptr )
			{
				switch ( ui )
				{
				case 0:
					network->_server.supported_modes_A = psz;
					break;
				case 1:
					network->_server.supported_modes_B = psz;
					break;
				case 2:
					network->_server.supported_modes_C = psz;
					break;
				case 3:
					network->_server.supported_modes_D = psz;
					break;
					/* further advancements in the protocol,
					 * if any, can be added here */
				default:
					std::cerr << fg_red << "More chanmode types reported (" << ui << ") than the known amount (4)\n";
					break;
				}

				++ui;
				psz = str_token(nullptr, cm_delim, &cm_last);
			}
		}

		p = str_token(nullptr, delim, &last);
	}

	/** @todo Use the rest of the RFC definitions on the variables not set by
	 * the server. Since Handle005 can be called more than once, we don't
	 * want to assign anything in there, in case they will be overwritten or
	 * otherwise cause issues - hence we apply the defaults once the server has
	 * ceased presenting itself to us. */

	if ( network->_server.max_num_modes == 0 )
		network->_server.max_num_modes = RFC1459_MODES;



	if (( nick_end = (char*)strchr(data->data.c_str(), search)) == nullptr )
		goto invalid_data;
	// data->data now nul terminates at the end of the nickname
	*nick_end = '\0';

	// move nick_end to the first character of the 'supported'
	nick_end++;

	if (( p = strchr(nick_end, ':')) == nullptr )
		goto invalid_data;

	// nul the end of the 'supported', which just leaves the message
	*p = '\0';
	p++;

	// prepare the activity data, then inform our listeners
	{
		activity.data			= nick_end;
		activity.message		= p;
		activity.instigator.nickname	= sender->nickname;

		_irc_engine->NotifyListeners(LN_005, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

incorrect_state:
	std::cerr << fg_red << "The connection state is invalid; not active\n";
	ret = EIrcStatus::InvalidState;
	goto cleanup;
invalid_data:
	ret = EIrcStatus::InvalidData;
	goto cleanup;
cleanup:
	FREE(dup);
	return ret;
}



EIrcStatus
IrcParser::Handle332(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcChannel>	channel = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_channel = nullptr;
	char*		extracted_nick = nullptr;
	char*		extracted_topic = nullptr;
	irc_activity&	activity = connection->GetActivity();

	if ( !ParseParameters(data->data.c_str(), 3, &extracted_nick, &extracted_channel, &extracted_topic) )
		goto parse_failure;

	if (( channel = connection->GetChannel(extracted_channel)) == nullptr )
		goto channel_not_found;

	channel->UpdateTopic(extracted_topic);

	// prepare the activity data, then inform our listeners
	{
		activity.channel_name	= extracted_channel;
		activity.data		= extracted_topic;

		_irc_engine->NotifyListeners(LN_332, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	goto cleanup;
channel_not_found:
	/** should we be generating an error? 332 could have no channel? */
	goto cleanup;
cleanup:
	FREE(extracted_nick);
	FREE(extracted_channel);
	FREE(extracted_topic);
	return ret;
}



EIrcStatus
IrcParser::Handle333(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcChannel>	channel = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_channel = nullptr;
	char*		extracted_setter = nullptr;
	char*		extracted_datetime = nullptr;
	irc_activity&	activity = connection->GetActivity();

	// :irc.siglost.com 333 trez #torze Torze!~quassel@staff.x22cheats.com 1349201058

	/** @todo check - 3 param, 1st = channel used to work fine, now there's an extra nick.. */
	//if ( !ParseParameters(data->data.c_str(), 3, &extracted_channel, &extracted_setter, &extracted_datetime) )
	if ( !ParseParameters(data->data.c_str(), 4, nullptr, &extracted_channel, &extracted_setter, &extracted_datetime) )
		goto parse_failure;

	if (( channel = connection->GetChannel(extracted_channel)) == nullptr )
		goto channel_not_found;

	// prepare the activity data, then inform our listeners
	{


		//_irc_engine->NotifyListeners(LN_333, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
channel_not_found:
	std::cerr << fg_red << "The extracted channel '" << extracted_channel << "' could not be found\n";
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
cleanup:
	FREE(extracted_setter);
	FREE(extracted_channel);
	FREE(extracted_datetime);
	return ret;
}



EIrcStatus
IrcParser::Handle353(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcChannel>	channel = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_type = nullptr;
	char*		extracted_channel = nullptr;
	char*		extracted_names = nullptr;
	char*		extracted_nick = nullptr;
	char*		p = nullptr;
	char*		last = nullptr;
	char		delim[] = " ";
	uint32_t	num_prefix;
	uint32_t	i;
	irc_server*	server = nullptr;
	ircbuf_sender	user;
	mode_update	umu;
	irc_activity&	activity = connection->GetActivity();

	if ( !ParseParameters(data->data.c_str(), 4,
		&extracted_nick, &extracted_type, &extracted_channel, &extracted_names) )
	{
		goto parse_failure;
	}

	if (( channel = connection->GetChannel(extracted_channel)) == nullptr )
		goto channel_not_found;


	server = &connection->Owner()->_server;

	if ( server->chan_mode_chars.empty() || server->chan_mode_symbols.empty() )
		goto unknown_prefixes;

	num_prefix = server->chan_mode_symbols.length();

	p = str_token(extracted_names, delim, &last);

	while ( p != nullptr )
	{
		umu.erase_existing	= false;
		umu.to_add		= UM_None;
		umu.to_remove		= UM_None;

		for ( i = 0; i < num_prefix; i++ )
		{
			if ( *p == server->chan_mode_symbols[i] )
			{
				/** @todo grab prefix meanings from config */
				if ( *p == '+' )	umu.to_add |= UM_Voice;
				else if ( *p == '%' )	umu.to_add |= UM_HalfOp;
				else if ( *p == '@' )	umu.to_add |= UM_Op;
				else if ( *p == '&' )	umu.to_add |= UM_Admin;
				else if ( *p == '~' )	umu.to_add |= UM_Owner;
				else			umu.to_add |= UM_Unknown;
				p++;
			}
		}

		if ( SplitSender(p, &user) != EIrcStatus::OK )
			goto invalid_data;

		// creates the user and adds it to the temporary names list
		if ( channel->AddNamesUser(
			user.nickname.c_str(), 
			user.ident.c_str(), 
			user.hostmask.c_str(), 
			&umu) != EIrcStatus::OK
		)
		{
			break;
		}

		p = str_token(nullptr, delim, &last);
	}

	/** @todo send a 353 for each individual nick, or the whole message? */
	// prepare the activity data, then inform our listeners
	{


		_irc_engine->NotifyListeners(LN_353, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
channel_not_found:
	std::cerr << fg_red << "The extracted channel '" << extracted_channel << "' could not be found\n";
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
invalid_data:
	ret = EIrcStatus::InvalidData;
	goto cleanup;
unknown_prefixes:
	std::cerr << fg_red << "No user/mode prefixes exist; required to parse IRC users\n";
	ret = EIrcStatus::UnknownResponse;
	goto cleanup;
cleanup:
	FREE(extracted_nick);
	FREE(extracted_channel);
	FREE(extracted_type);
	FREE(extracted_names);
	return ret;
}



EIrcStatus
IrcParser::Handle366(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcChannel>	channel = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_nick = nullptr;
	char*		extracted_channel = nullptr;
	char*		extracted_message = nullptr;
	irc_activity&	activity = connection->GetActivity();

	/* Rizon
	:$server 366 $nickname $channel :End of /NAMES list.
	*/

	// only interested in the nick + channel
	if ( !ParseParameters(data->data.c_str(), 3, &extracted_nick, &extracted_channel, &extracted_message) )
		goto parse_failure;

	if (( channel = connection->GetChannel(extracted_channel)) == nullptr )
		goto channel_not_found;


	// convert the temporary names list into the active userlist
	channel->PopulateUserlist();

	// prepare the activity data, then inform our listeners
	{


		_irc_engine->NotifyListeners(LN_366, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
channel_not_found:
	std::cerr << fg_red << "The extracted channel '" << extracted_channel << "' could not be found\n";
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
cleanup:
	FREE(extracted_nick);
	FREE(extracted_channel);
	FREE(extracted_message);
	return ret;
}



EIrcStatus
IrcParser::Handle372(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	EIrcStatus	ret = EIrcStatus::Unknown;
	irc_activity&	activity = connection->GetActivity();

	// prepare the activity data, then inform our listeners
	{


		//_irc_engine->NotifyListeners(LN_372, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

cleanup:
	return ret;
}



EIrcStatus
IrcParser::Handle375(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	EIrcStatus	ret = EIrcStatus::Unknown;
	irc_activity&	activity = connection->GetActivity();

	// prepare the activity data, then inform our listeners
	{


		//_irc_engine->NotifyListeners(LN_375, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

cleanup:
	return ret;
}



EIrcStatus
IrcParser::Handle376(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	EIrcStatus	ret = EIrcStatus::Unknown;
	irc_activity&	activity = connection->GetActivity();

	// prepare the activity data, then inform our listeners
	{


		//_irc_engine->NotifyListeners(LN_376, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

cleanup:
	return ret;
}


// 410 - invalid cap


EIrcStatus
IrcParser::Handle432(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_nick = nullptr;
	char*		extracted_bad_nick = nullptr;
	char*		extracted_message = nullptr;
	irc_activity&	activity = connection->GetActivity();

	if ( !ParseParameters(data->data.c_str(), 3, &extracted_nick, &extracted_bad_nick, &extracted_message) )
		goto parse_failure;

	// a = sender, b = destination_nickname, c = bad_nickname, d = message */
	//add_ui_event((E_MSG_CODE)432, connection, 4, sender->nickname, extracted_nick, extracted_bad_nick, extracted_message);

	/** @todo if ( auto_change_nick... ) */
	connection->AutoChangeNick();

	// prepare the activity data, then inform our listeners
	{


		//_irc_engine->NotifyListeners(LN_432, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
cleanup:
	FREE(extracted_nick);
	FREE(extracted_bad_nick);
	FREE(extracted_message);
	return ret;
}



EIrcStatus
IrcParser::Handle433(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_nick = nullptr;
	char*		extracted_used_nick = nullptr;
	char*		extracted_message = nullptr;
	irc_activity&	activity = connection->GetActivity();

	if ( !ParseParameters(data->data.c_str(), 3, &extracted_nick, &extracted_used_nick, &extracted_message) )
		goto parse_failure;

	/** @todo if ( auto_change_nick... ) */
	connection->AutoChangeNick();

	// prepare the activity data, then inform our listeners
	{


		//_irc_engine->NotifyListeners(LN_433, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
cleanup:
	FREE(extracted_nick);
	FREE(extracted_used_nick);
	FREE(extracted_message);
	return ret;
}



EIrcStatus
IrcParser::HandleCap(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	/* :ircd.trezanik.org CAP * ACK :multi-prefix */
	/* :ircd.trezanik.org CAP * NAK :uhnames */
	/*
	 * R	CAP LS
	 * S	CAP LIST
	 * S	CAP REQ
	 * R	CAP ACK
	 * R	CAP NAK
	 * S	CAP CLEAR
	 * S	CAP END
	 *
	 * Servers MUST respond to a REQ command with either the ACK or NAK subcommands to indicate acceptance or rejection of the capability set requested by the client
	*/
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_acknak = nullptr;
	char*		extracted_cap = nullptr;
	irc_activity&	activity = connection->GetActivity();

	// CAP param 1 will always be '*' as we won't have sent or confirmed NICK
	if ( !ParseParameters(data->data.c_str(), 3, nullptr, &extracted_acknak, &extracted_cap) )
		goto parse_failure;

	if ( strcmp(extracted_acknak, "ACK") == 0 )
	{
		connection->_cap_ack.push_back(extracted_cap);
	}
	else if ( strcmp(extracted_acknak, "NAK") == 0 )
	{
		connection->_cap_nak.push_back(extracted_cap);
	}
	else if ( strcmp(extracted_acknak, "LS") == 0 )
	{
		/* must have requested a list (atm, we don't at all) */
		/* believe we need a str_token here */
		connection->_cap_ls.push_back(extracted_cap);
	}
	else
	{
		goto what_acknak;
	}



	// prepare the activity data, then inform our listeners
	{
		// only nickname should be valid, but just in case...
		activity.instigator.hostmask	= sender->hostmask;
		activity.instigator.ident	= sender->ident;
		activity.instigator.nickname	= sender->nickname;
		activity.message		= extracted_cap;
		activity.data			= extracted_acknak;

		_irc_engine->NotifyListeners(LN_Cap, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
what_acknak:
	std::cerr << fg_red << "Unknown response to a CAP: " << data->data << "\n";
	ret = EIrcStatus::UnknownResponse;
	goto cleanup;
cleanup:
	FREE(extracted_acknak);
	FREE(extracted_cap);
	return ret;
}



EIrcStatus
IrcParser::HandleInvite(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcUser>	user = nullptr;
	std::shared_ptr<IrcChannel>	channel = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_nick = nullptr;
	char*		extracted_channel = nullptr;
	irc_activity&	activity = connection->GetActivity();

	if ( !ParseParameters(data->data.c_str(), 2, &extracted_nick, &extracted_channel) )
		goto parse_failure;

	user;
	channel;

	// prepare the activity data, then inform our listeners
	{


		_irc_engine->NotifyListeners(LN_Invite, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
cleanup:
	FREE(extracted_nick);
	FREE(extracted_channel);
	return ret;
}



EIrcStatus
IrcParser::HandleJoin(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcUser>	user = nullptr;
	std::shared_ptr<IrcChannel>	channel = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	std::string	channel_name;
	irc_activity&	activity = connection->GetActivity();

	if ( data->data[0] != ':' )
		goto invalid_data;

	/* remainder of the string is the channel name - check if we're joining 
	 * a new one, or if another user is joining one we're in */
	channel_name = data->data.substr(1);

	if ( channel_name.length() < 2 )
		goto invalid_data;

	if ( sender->nickname.compare(connection->Owner()->_client.nickname) == 0 )
	{
		// self; check if we still have the channel 'open', but is not active

		if (( channel = connection->GetChannel(channel_name.c_str())) == nullptr )
		{
			// totally brand new channel
			_irc_engine->CreateChannel(connection->Id(), channel_name.c_str());
			/*
			if (( ret = connection->AddChannel(channel_name.c_str()) == EIrcStatus::OK )
				channel = connection->GetChannel(channel_name.c_str());
			else
				goto channel_add_failure;*/

			/* note that we don't add our own user here - upon joining a channel,
			 * you should always receive a 353 NAMES list */
			/** @todo is there any way to guarantee/ensure this? */
		}

		channel->_flags |= CHANFLAG_ACTIVE;

		// prepare the activity data, then inform our listeners
		{
			activity.instigator.hostmask	= sender->hostmask;
			activity.instigator.ident	= sender->ident;
			activity.instigator.nickname	= sender->nickname;
			activity.channel_name		= channel_name;

			_irc_engine->NotifyListeners(LN_WeJoined, connection);
		}
	}
	else
	{
		// someone else joining a channel we're in

		if (( channel = connection->GetChannel(channel_name.c_str())) == nullptr )
			goto no_channel_object;
		if ( !(channel->_flags & CHANFLAG_ACTIVE) )
			goto invalid_channel_state;

		if ( _irc_engine->CreateUser(
			connection->Id(), 
			channel_name.c_str(),
			sender->nickname.c_str(), 
			sender->ident.c_str(), 
			sender->hostmask.c_str())
			!= EIrcStatus::OK
		)
		{
			goto user_add_failure;
		}

		// prepare the activity data, then inform our listeners
		{
			activity.instigator.hostmask	= sender->hostmask;
			activity.instigator.ident	= sender->ident;
			activity.instigator.nickname	= sender->nickname;
			activity.channel_name		= channel_name;

			_irc_engine->NotifyListeners(LN_Join, connection);
		}
	}

	ret = EIrcStatus::OK;
	goto cleanup;

invalid_data:
	ret = EIrcStatus::InvalidData;
	goto cleanup;
channel_add_failure:
	ret = EIrcStatus::ObjectAddError;
	goto cleanup;
no_channel_object:
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
invalid_channel_state:
	ret = EIrcStatus::InvalidState;
	goto cleanup;
user_add_failure:
	ret = EIrcStatus::ObjectAddError;
	goto cleanup;
cleanup:
	return ret;
}



EIrcStatus
IrcParser::HandleKick(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcChannel>	channel = nullptr;
	std::shared_ptr<IrcUser>	user = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_channel = nullptr;
	char*		extracted_kicked = nullptr;
	char*		extracted_kick_message = nullptr;
	irc_activity&	activity = connection->GetActivity();

	/* Rizon:
	:$nick!$ident@$host KICK $channel $nickname :$message
	*/

	if ( !ParseParameters(data->data.c_str(), 3, &extracted_channel, &extracted_kicked, &extracted_kick_message) )
		goto parse_failure;

	if (( channel = connection->GetChannel(extracted_channel)) == nullptr )
		goto channel_not_found;


	if ( strcmp(extracted_kicked, connection->Owner()->_client.nickname.c_str()) == 0 )
	{
		// we're the one that was kicked; remove compulsory items from the chan

		channel->_flags = CHANFLAG_RAW;
		channel->EraseUserlist();

		// prepare the activity data, then inform our listeners
		{


			_irc_engine->NotifyListeners(LN_GotKicked, connection);
		}

		/** @todo if ( AutoRejoinOnKick )*/
		{
			/** @todo retain the channel key to rejoin?? */
			connection->SendJoin(extracted_channel);
		}

		/* a = sender_nick, b = sender_ident, c = sender_host, d = channel, e = kicked_nick, f = kick_message */
		//add_ui_event(MC_Kick, connection, 6,
		//	sender->nickname, sender->ident, sender->hostmask,
		//	extracted_channel, extracted_kicked, extracted_kick_message);
	}
	else
	{
		// someone else in the channel was kicked

		if (( user = channel->GetUser(extracted_kicked)) == nullptr )
			goto kicked_not_found;

		if (( ret = channel->DeleteUser(user)) != EIrcStatus::OK )
			goto delete_failed;

		// prepare the activity data, then inform our listeners
		{


			_irc_engine->NotifyListeners(LN_Kick, connection);
		}

		/* a = sender_nick, b = sender_ident, c = sender_host, d = channel, e = kicked_nick, f = kick_message */
		//add_ui_event(MC_Kick, connection, 6,
		//	sender->nickname, sender->ident, sender->hostmask,
		//	extracted_channel, extracted_kicked, extracted_kick_message);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
channel_not_found:
	std::cerr << fg_red << "The extracted channel '" << extracted_channel << "' could not be found\n";
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
kicked_not_found:
	std::cerr << fg_red << "The extracted kicked nickname '" << extracted_kicked << "' was not found\n";
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
delete_failed:
	// report error?
	goto cleanup;
cleanup:
	FREE(extracted_channel);
	FREE(extracted_kicked);
	FREE(extracted_kick_message);
	return ret;
}



EIrcStatus
IrcParser::HandleKill(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcChannel>	channel = nullptr;
	std::shared_ptr<IrcUser>	user = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_nickname = nullptr;
	char*		extracted_message = nullptr;
	irc_activity&	activity = connection->GetActivity();

	/* Rizon:
	:$server KILL $nickname :$message
	*/

	if ( !ParseParameters(data->data.c_str(), 2, &extracted_nickname, &extracted_message) )
		goto parse_failure;

	if ( strcmp(extracted_nickname, connection->GetCurrentNickname().c_str()) == 0 )
	{
		// prepare the activity data, then inform our listeners
		{


			_irc_engine->NotifyListeners(LN_GotKilled, connection);
		}
	}
	else
	{
		/* consistency failure; KILLs should only be directed at the 
		 * affected user - either the server is lying or we lost track 
		 * of our nick */
		goto nick_not_client;

	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
nick_not_client:
	std::cerr << fg_red << "Killed nickname '" << extracted_nickname <<
		"' does not match the current client setting: '" <<
		connection->Owner()->_client.nickname.c_str() << "'\n";
	ret = EIrcStatus::NickIsNotClient;
	goto cleanup;
cleanup:
	FREE(extracted_nickname);
	FREE(extracted_message);
	return ret;
}



EIrcStatus
IrcParser::HandleMode(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcNetwork>	network = nullptr;
	std::shared_ptr<IrcChannel>	channel = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_target = nullptr;
	char*		extracted_changes = nullptr;
	char*		extracted_affected = nullptr;
	irc_activity&	activity = connection->GetActivity();
	char*		p = nullptr;
	char*		str = nullptr;
	char*		last = nullptr;
	char		delim[] = " ";
	bool		is_set = false;
	uint32_t	i;
	uint32_t	len;
	// must be shared_ptr if a part of a vector!
	std::vector<std::shared_ptr<mode_data>>	modes;

	/* Rizon:
	:$nickname!$ident@$hostmask MODE $nickname :+ix
	:$nickname!$ident@$hostmask MODE $channel +h $nickname
	*/

	if ( !ParseParameters(data->data.c_str(), 3,
		&extracted_target, &extracted_changes, &extracted_affected) )
		goto parse_failure;

	network = connection->Owner();

	p = extracted_changes;

	if ( strcmp(extracted_target, network->_client.nickname.c_str()) == 0 )
	{
		// 'self' user mode
		/// @todo complete me

		// prepare the activity data, then inform our listeners
		{


			//_irc_engine->NotifyListeners(LN_, connection);
		}
	}
	else if ( extracted_affected == nullptr || *extracted_affected == '\0' )
	{
		// simple channel/user modes - ms, ix, etc.

		// some, not all, have a colon prefixed
		if ( *p == ':' )
			p++;

		while ( *p != '\0' )
		{
			if ( *p == '+' )
				is_set = true;
			else if ( *p == '-' )
				is_set = false;
			// else the character is a continuation of the previous enable state
			else
			{
				// check if already at the limit
				if ( modes.size() >= network->_server.max_num_modes )
					goto limit_exceeded;

				std::shared_ptr<mode_data>	mode(new mode_data);

				mode->is_enabled	= is_set;
				mode->mode		= *p;
				mode->has_data		= false;
				mode->data		= "";

				modes.push_back(mode);
			}
			p++;
		}

		/* use get_channel on extracted_target; if no channel is found, 
		 * it is a user mode. Will use same functionality as below - 
		 * need to merge the two */
	}
	else
	{
		len = strlen(extracted_changes);
		is_set = (*p == '+');

		// ensure we're starting with valid data; make no assumptions
		if ( *p != '+' && *p != '-' )
			goto invalid_data;

		for ( i = 1; i < len; i++ )
		{
			/* increment on loop entry and then every other iteration (entry
			 * will bring us to the first real character) */
			p++;
			if ( *p == '+' )
				is_set = true;
			else if ( *p == '-' )
				is_set = false;
			// else the character is a continuation of the enable state
			else
			{
				std::shared_ptr<mode_data>	mode(new mode_data);

				mode->is_enabled	= is_set;
				mode->mode		= *p;
				mode->data		= "";

				if ( ModeHasArgument(connection, is_set, *p) )
					mode->has_data = true;

				modes.push_back(mode);
			}
		}

		p = extracted_affected;
		str = str_token(p, delim, &last);

		// for every mode, assign the data if it requires any
		for ( auto m : modes )
		{
			// only if it needs data; +m, +p, etc. do not have any data assigned!
			if ( m->has_data )
			{
				if ( str == nullptr )
				{
					std::cerr << fg_red << "No data remaining for required assignment; server supplied invalid data\n";
				}
				else
				{
					m->data	 = str;
				}

				str = str_token(nullptr, delim, &last);
			}
		}

		if ( !HasChannelPrefix(connection, extracted_target) )
		{
			// target was not a channel

		}
		else if ( (channel = connection->GetChannel(extracted_target)) == nullptr )
		{
			goto channel_not_found;
		}
		else
		{
			// target was a channel, and we have it opened
			for ( auto m : modes )
			{
				if ( !m->data.empty() )
				{
					mode_update	umu;
					uint16_t		update = UM_None;
					std::shared_ptr<IrcUser>	user;

					/// @todo; bring these in from configuration
					// also list against server modes (005 PREFIX)
					const char	char_voice = 'v';
					const char	char_halfop = 'h';
					const char	char_op = 'o';
					const char	char_admin = 'a';
					const char	char_owner = 'q';

					/// @todo extract mode mappings
					switch ( modes[i]->mode )
					{
					case char_voice:	update = UM_Voice; break;
					case char_halfop:	update = UM_HalfOp; break;
					case char_op:		update = UM_Op; break;
					case char_admin:	update = UM_Admin; break;
					case char_owner:	update = UM_Owner; break;
					default:
						break;
					}

					if ( update != UM_None )
					{
						user = channel->GetUser(m->data.c_str());
						if ( user != nullptr )
						{
							umu.erase_existing = false;
							umu.to_add = UM_None;
							umu.to_remove = UM_None;

							if ( modes[i]->is_enabled )
								umu.to_add = update;
							else
								umu.to_remove = update;

							user->Update(nullptr, nullptr, nullptr, &umu);
						}
					}
				}
			}
		}
	}

	/* note: if the server is the sender, b & c will be NULL; affected can
	 * be NULL or an additional 'list', depending on the mode type */
	/* a = sender_nick, b = sender_ident, c = sender_host, d = target, e = changes, f = affected */
	//add_ui_event(MC_Mode, connection, 6, sender->nickname, sender->ident, sender->hostmask, extracted_target, extracted_changes, extracted_affected);
	// @todo these must be put into each 'section' above

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
limit_exceeded:
	ret = EIrcStatus::LimitExceeded;
	goto cleanup;
invalid_data:
	ret = EIrcStatus::InvalidData;
	goto cleanup;
channel_not_found:
	std::cerr << fg_red << "The extracted target '" << extracted_target << "' could not be found\n";
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
cleanup:
	FREE(extracted_target);
	FREE(extracted_changes);
	FREE(extracted_affected);
	return ret;
}



EIrcStatus
IrcParser::HandleNick(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcNetwork>	network = nullptr;
	std::shared_ptr<IrcUser>	user = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	irc_activity&	activity = connection->GetActivity();
	std::string	new_nick;

	/* Rizon:
	:$nickname!$ident@$hostmask NICK :$new_nick
	*/

	if ( data->data[0] != ':' )
		goto invalid_data;

	network = connection->Owner();

	new_nick = (data->data.substr(1));

	if ( sender->nickname.compare(network->_client.nickname) == 0 )
	{
		// our own nickname has been changed; update our client info

		network->_client.nickname	= new_nick.c_str();

		// prepare the activity data, then inform our listeners
		{


			_irc_engine->NotifyListeners(LN_GotNickChanged, connection);
		}
	}
	else
	{
#if 0
		/* another user has updated their nick in at least one of the channels
		 * we are in - loop through them all and update as necessary */

		for ( auto channel : connection->_channel_list )
		{
			if (( user = channel->GetUser(sender->nickname.c_str())) != nullptr )
			{
				user->Update(new_nick.c_str(), nullptr, nullptr, nullptr);
			}
			else
			{
			}
		}

		/** @todo What if we're in no channels, but are querying with a user
		 * who changes nick... */
#endif

		// prepare the activity data, then inform our listeners
		{


			_irc_engine->NotifyListeners(LN_Nick, connection);
		}
	}

	ret = EIrcStatus::OK;
	goto cleanup;

invalid_data:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
not_found:
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
cleanup:
	return ret;
}



EIrcStatus
IrcParser::HandleNotice(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcNetwork>	network = connection->Owner();
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_destination = nullptr;
	char*		extracted_message = nullptr;
	irc_activity&	activity = connection->GetActivity();
	

	if ( !ParseParameters(data->data.c_str(), 2, &extracted_destination, &extracted_message) )
		goto parse_failure;

	if ( HasChannelPrefix(connection, extracted_destination) )
	{
		/** @todo complete me */

	}
	else
	{

		// Hybrid, Quakenet, InspIRCd, efnet: 'AUTH'
		// Freenode: '*'

		if ( network->_client.nickname.empty() )
		{
			char	preauth[] = "AUTH";
			char	preauth_freenode[] = "*";

			if ( strcmp(extracted_destination, preauth) == 0 ||
				strcmp(extracted_destination, preauth_freenode) == 0 )
			{
				/* pre-connection notice; own nickname not confirmed. When
				 * using CAPS, we get our nickname back assuming there is
				 * no conflict - otherwise the first we can 100% confirm
				 * the server + client are in sync is in the 001 */
			}
		}
		else if ( strcmp(extracted_destination, network->_client.nickname.c_str()) == 0 )
		{
			// the notice was directed at us

			static bool	init_sent = false;

			if ( !connection->IsActive() && !init_sent )
			{
				/* connection not yet sustained, so send through
				 * any initialization to perform */

				init_sent = true;

				if ( network->_profile_config.auto_identify )
				{
					connection->SendIdentify(
						network->_profile_config.autoident_service.c_str(),
						network->_profile_config.autoident_password.c_str());
				}
				if ( network->_network_config.auto_exec_commands )
				{
					for ( auto cmd : network->_network_config.commands )
					{
						connection->SendRaw(cmd.c_str());
					}
				}
				if ( network->_network_config.auto_join_channels )
				{
					for ( auto channel : network->_network_config.channels )
					{
						connection->SendJoin(channel.c_str());
					}
				}
			}
		}
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
cleanup:
	FREE(extracted_destination);
	FREE(extracted_message);
	return ret;
}



EIrcStatus
IrcParser::HandlePart(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcNetwork>	network = nullptr;
	std::shared_ptr<IrcChannel>	channel = nullptr;
	std::shared_ptr<IrcUser>	user = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_channel = nullptr;
	char*		extracted_message = nullptr;
	irc_activity&	activity = connection->GetActivity();

	network = connection->Owner();

	if ( !ParseParameters(data->data.c_str(), 2, &extracted_channel, &extracted_message) )
		goto parse_failure;

	// locate the channel the user was in, and remove them from it

	if (( channel = connection->GetChannel(extracted_channel)) == nullptr )
		goto channel_not_found;


	if ( sender->nickname.compare(network->_client.nickname) == 0 )
	{
		// we're the one that parted - channel is now inactive

		channel->_flags = CHANFLAG_RAW;

		/** @todo Close & delete the channel if not configured to remain open,
		 notify() - special, auto-delete, what? */

		// prepare the activity data, then inform our listeners
		{


			_irc_engine->NotifyListeners(LN_WeParted, connection);
		}

		channel->EraseUserlist();
	}
	else
	{
		if (( user = channel->GetUser(sender->nickname.c_str())) != nullptr )
		{
			/*sn.channel	= channel;
			sn.hostmask	= user->Hostmask();
			sn.ident	= user->Ident();
			sn.nickname	= user->Nickname();
			sn.message	= extracted_message;*/

			// prepare the activity data, then inform our listeners
			{


				_irc_engine->NotifyListeners(LN_Part, connection);
			}

			channel->DeleteUser(user);
		}
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
channel_not_found:
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
cleanup:
	return ret;
}



EIrcStatus
IrcParser::HandlePong(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	irc_activity&	activity = connection->GetActivity();

	// reply from our lag message
	connection->_lag_sent = 0;

	/** @todo compare recv time with sent time to determine lag */

	return EIrcStatus::OK;
}



EIrcStatus
IrcParser::HandlePrivmsg(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcChannel>	channel = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_destination = nullptr;
	char*		extracted_message = nullptr;
	irc_activity&	activity = connection->GetActivity();
	char		*p = nullptr;

	if ( !ParseParameters(data->data.c_str(), 2, &extracted_destination, &extracted_message) )
		goto parse_failure;

	/** @todo readd terminating \001 check; feeling lazy atm */
	if (	*extracted_message == '\001' &&
		*(extracted_message + 1) != '\0' )
		//extracted_message[(strlen(extracted_message)-1)] == '\001' )
	{
		// CTCP message

		// move up to the first real character
		p = (extracted_message + 1);

		if ( strncmp(p, "ACTION", 6) == 0 )
		{
			// CTCP ACTION - 'me' style privmsg

			/** @todo Same code as privmsg? */

			// prepare the activity data, then inform our listeners
			{


				//_irc_engine->NotifyListeners(LN_CtcpAction, connection);
			}
		}
		else
		{
			if ( strncmp(p, "VERSION", 7) == 0 )
			{
				// CTCP VERSION

				connection->SendBypass("NOTICE %s :\001VERSION %s\001\r\n",
					sender->nickname.c_str(), "Trezanik IRC");

				// prepare the activity data, then inform our listeners
				{


					//_irc_engine->NotifyListeners(LN_CtcpVersion, connection);
				}
			}
			/// @todo Handle the other CTCPs
		}

	}
	else
	{
		// Didn't contain a CTCP prefix/suffix

		activity.instigator.hostmask	= sender->hostmask;
		activity.instigator.ident	= sender->ident;
		activity.instigator.nickname	= sender->nickname;
		activity.message		= extracted_message;

		if ( HasChannelPrefix(connection, extracted_destination) )
		{
			// channel message

			if (( channel = connection->GetChannel(extracted_destination)) == nullptr )
				goto channel_not_found;

			activity.channel_name	= extracted_destination;
		}
		else
		{
			// query message
			activity.data		= extracted_destination;
		}


		{
			_irc_engine->NotifyListeners(LN_Privmsg, connection);
		}
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
channel_not_found:
	std::cerr << fg_red << "The extracted channel '" << extracted_destination << "' could not be found\n";
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
cleanup:
	FREE(extracted_destination);
	FREE(extracted_message);
	return ret;
}



EIrcStatus
IrcParser::HandleQuit(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcUser>	user = nullptr;
	//std::shared_ptr<IrcNetwork>	network = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	irc_activity&	activity = connection->GetActivity();
	std::string	quit_message;
	uint32_t	num_affected = 0;

	// no point using parse_parameters, only 1 'field'
	if ( data->data[0] != ':' )
		goto invalid_data;

	// either a nul or remainder of string
	quit_message = data->data.substr(1);

	if ( sender->nickname.compare(connection->Owner()->_client.nickname) == 0 )
	{
		/* we're the one quitting the network - !!TODO:: were we expecting it?
		 * If the server wants us out it will/should call KILL */

		/* how far do we want to cleanup the connection? Destroying all channels
		 * may not actually be desired - just remove CHANFLAG_ACTIVE? */

		connection->EraseChannelList();

		// prepare the activity data, then inform our listeners
		{


			_irc_engine->NotifyListeners(LN_WeQuit, connection);
		}
	}
	else
	{
		// prepare the activity data, then inform our listeners
		{
			activity.instigator.nickname	= sender->nickname;
			activity.instigator.ident	= sender->ident;
			activity.instigator.hostmask	= sender->hostmask;
			activity.message		= quit_message;

			_irc_engine->NotifyListeners(LN_Quit, connection);
		}

		// remove the user leaving from all channels we're maintaining
		for ( auto channel : _irc_engine->Pools()->IrcChannels()->Allocated() )
		{
			if (( user = channel->GetUser(sender->nickname.c_str())) != nullptr )
			{
				channel->DeleteUser(user);
				num_affected++;
			}

		}

		// query windows?

		if ( num_affected == 0 )
		{
			/* we received the message, so there must/should be at 
			 * least one channel that is affected.. */
			std::cerr << fg_red << "Received a QUIT, but no users were affected. Recommend restart, likely corruption\n";
		}
	}

	ret = EIrcStatus::OK;
	goto cleanup;

invalid_data:
	std::cerr << fg_red << "Invalid data: " << data->data.c_str();
	ret = EIrcStatus::InvalidData;
	goto cleanup;
cleanup:
	return ret;
}



EIrcStatus
IrcParser::HandleTopic(
	std::shared_ptr<IrcConnection> connection,
	ircbuf_data* data,
	ircbuf_sender* sender
)
{
	std::shared_ptr<IrcChannel>	channel = nullptr;
	EIrcStatus	ret = EIrcStatus::Unknown;
	char*		extracted_channel = nullptr;
	char*		extracted_topic = nullptr;
	irc_activity&	activity = connection->GetActivity();

	if ( !ParseParameters(data->data.c_str(), 2, &extracted_channel, &extracted_topic) )
		goto parse_failure;

	if (( channel = connection->GetChannel(extracted_channel)) == nullptr )
		goto channel_not_found;

	channel->UpdateTopic(extracted_topic);

	// prepare the activity data, then inform our listeners
	{


		_irc_engine->NotifyListeners(LN_Topic, connection);
	}

	ret = EIrcStatus::OK;
	goto cleanup;

parse_failure:
	ret = EIrcStatus::ParsingError;
	goto cleanup;
channel_not_found:
	std::cerr << fg_red << "The extracted channel '" << extracted_channel << "' could not be found\n";
	ret = EIrcStatus::ObjectNotFound;
	goto cleanup;
cleanup:
	FREE(extracted_channel);
	FREE(extracted_topic);
	return ret;
}



bool
IrcParser::HasChannelPrefix(
	const std::shared_ptr<IrcConnection> connection,
	char* str
) const
{
	std::shared_ptr<IrcNetwork>	network = nullptr;

	if ( connection == nullptr )
	{
		std::cerr << fg_red << "The supplied connection was a nullptr\n";
		return false;
	}
	if ( str == nullptr )
	{
		std::cerr << fg_red << "The input string was a nullptr\n";
		return false;
	}
	if (( network = connection->Owner()) == nullptr )
	{
		std::cerr << fg_red << "The supplied connections owning network was a nullptr\n";
		return false;
	}

	// chan types is populated only after receiving 005, so check for an empty string
	if ( network->_server.chan_types.empty() )
		return false;

	// only check the first character...
	return strchr(network->_server.chan_types.c_str(), *str) != nullptr;
}



bool
IrcParser::ModeHasArgument(
	const std::shared_ptr<IrcConnection> connection,
	bool is_set,
	char mode
) const
{
	std::shared_ptr<IrcNetwork>	network = nullptr;

	if ( connection == nullptr )
	{
		std::cerr << fg_red << "The supplied connection was a nullptr\n";
		return false;
	}
	if ( mode == '\0' )
	{
		std::cerr << fg_red << "The supplied mode was a NUL\n";
		return false;
	}
	if (( network = connection->Owner()) == nullptr )
	{
		std::cerr << fg_red << "The supplied connections owning network was a nullptr\n";
		return false;
	}
	

	// if it's a user/nick mode, it must have an argument (+v, +o)
	if ( strchr(network->_server.chan_mode_chars.c_str(), mode) != nullptr )
		return true;

	// A & B type modes will always have one (+e, +I)
	if ( strchr(network->_server.supported_modes_A.c_str(), mode) != nullptr ||
		strchr(network->_server.supported_modes_B.c_str(), mode) != nullptr )
		return true;

	// C type modes will have one, if the mode is being enabled (+k, +l)
	if ( strchr(network->_server.supported_modes_C.c_str(), mode) != nullptr && is_set )
		return true;

	// D type modes will have none, and default return none for unknown modes
	return false;
}



char*
IrcParser::ParseParam(
	char** data
) const
{
	char*	p = nullptr;

	if ( data == nullptr )
	{
		std::cerr << fg_red << "The input string was a nullptr\n";
		return nullptr;
	}
	if ( *data == nullptr )
	{
		std::cerr << fg_red << "The input string started with a nullptr\n";
		return nullptr;
	}

	if ( **data == ':' )
	{
		// last parameter
		p = *data;
		*data += strlen(*data);
		return (p + 1);
	}

	p = *data;

	while ( **data != '\0' && **data != ' ' )
		(*data)++;

	if ( **data == ' ' )
		*(*data)++ = '\0';

	return p;
}



bool
IrcParser::ParseParameters(
	const char* buffer,
	uint32_t num_args,
	...
) const
{
	char**		arg_str = nullptr;
	char*		tmp_str = nullptr;
	char*		p = (char*)buffer;
	uint32_t	alloc;
	uint32_t	i = num_args;   // copy in case we need to revert our actions
	va_list		args;

	if ( buffer == nullptr )
	{
		std::cerr << fg_red << "The supplied buffer was a nullptr\n";
		return false;
	}
	
	if ( num_args == 0 )
	{
		std::cerr << fg_red << "No arguments were supplied\n";
		return false;
	}

	// va_start MUST receive the last member passed into the function!
	va_start(args, num_args);

	/* num_args should ALWAYS be the number of arguments within the buffer; if
	 * not interested in a parameter, pass NULL in, instead of just leaving
	 * the data early (which can/will result in receiving the rest of the data
	 * within the last parameter, which will not be desired!) */
	while ( i-- > 0 )
	{
		/* get the variable passed down */
		arg_str = (char**)va_arg(args, char**);

		/* if the last parameter, also check for the separating colon */
		if ( i == 0 )
			tmp_str = *p == ':' ? p+1 : p;
		else
			tmp_str = ParseParam(&p);

		/* we want to ignore some arguments that will appear before the rest of
		 * the data (e.g. &extracted_nick, NULL, &extracted_data), so NULLs
		 * must be acceptable when passed in. */
		if ( arg_str != nullptr && *arg_str == nullptr )
		{
			alloc = strlen(tmp_str) + 1;
			*arg_str = (char*)MALLOC(alloc);
			strlcpy(*arg_str, tmp_str, alloc);

			if ( &*arg_str == nullptr )
				throw std::bad_alloc();
		}
	}

	va_end(args);
	return true;
}



#if defined(_WIN32)
uint32_t __stdcall
#elif defined(__linux__) || defined(BSD)
void*
#endif
IrcParser::ExecParser(
	void* this_ptr
)
{
#if defined(_WIN32)
	((IrcParser*)this_ptr)->RunParser();
	return 0;
#else
	((IrcParser*)this_ptr)->RunParser();
	return nullptr;
#endif
}



EIrcStatus
IrcParser::ParseConnectionQueues(
	std::shared_ptr<IrcConnection> connection
)
{
	EIrcStatus	ret_code;

	if ( connection == nullptr )
	{
		std::cerr << fg_red << "The supplied connection was a nullptr\n";
		return EIrcStatus::MissingParameter;
	}

	do
	{
		ret_code = ParseNextRecvQueueItem(connection);
		/* continue the loop regardless of if the result was successful
		 * or an error; errors will be logged, could only apply to a
		 * single queue entry, and shouldn't kline the entire process. */
	}
	while ( ret_code != EIrcStatus::QueueEmpty );

	// repeat for the send queue
	do
	{
		ret_code = ProcessNextSendQueueItem(connection);
	}
	while ( ret_code != EIrcStatus::QueueEmpty );

	return EIrcStatus::OK;
}



EIrcStatus
IrcParser::ParseNextRecvQueueItem(
	std::shared_ptr<IrcConnection> connection
) const
{
	std::string	queue_str;
	const char*	err = nullptr;
	ircbuf_data	buf_data;
	ircbuf_sender	sender;

	if ( connection == nullptr )
		goto no_connection;

	/* prevent a race condition, if AddToRecvQueue is amending the queue at
	 * the exact time we're trying to grab an item from it. Release the lock
	 * asap, so have a dedicated scope for it. */
	{
		std::lock_guard<std::mutex>	lock(connection->_mutex);

		if ( connection->_recv_queue.empty() )
		{
			return EIrcStatus::QueueEmpty;
		}

		queue_str = connection->_recv_queue.front();
		connection->_recv_queue.pop();
	}

	std::cout << fg_cyan << "Parsing " << fg_white << queue_str.c_str() << "\n";

	if ( strncmp(queue_str.c_str(), "ERROR :", 7) == 0 )
	{
		/* the server can disconnect us, for things like registration
		 * time-outs. Pass the error string so it can be logged. */
		err = queue_str.substr(7).c_str();
		goto srv_error;
	}
#if 0	// Code Removed: now done pre-reading the first buffer in EstablishConnection()
	else if ( !(connection->_state & CS_InitSent) )
	{
		connection->SendInit();
	}
#endif
	else
	{
		// reduce duplication and spam; assign handler by function pointer
		typedef EIrcStatus (IrcParser::*pf_CodeHandler)(std::shared_ptr<IrcConnection>, ircbuf_data*, ircbuf_sender*);

		pf_CodeHandler	parser_func;
		uint32_t	len;
		uint16_t	numeric;

		/* generates errors themselves */
		if ( ExtractIrcBufData(queue_str.c_str(), &buf_data) != EIrcStatus::OK )
			goto extract_failed;
		if ( SplitSender(buf_data.sender.c_str(), &sender) != EIrcStatus::OK )
			goto split_failed;
		if ( buf_data.code.length() < 3 )
			goto invalid_code;


		numeric = (uint16_t)atoi(buf_data.code.c_str());

		if ( numeric > 0 && numeric < 1000 )
		{
			switch ( numeric )
			{
			case 1:		parser_func = &IrcParser::Handle001; goto exec;
			case 2:		parser_func = &IrcParser::Handle002; goto exec;
			case 5:		parser_func = &IrcParser::Handle005; goto exec;
			case 332:	parser_func = &IrcParser::Handle332; goto exec;
			case 333:	parser_func = &IrcParser::Handle333; goto exec;
			case 353:	parser_func = &IrcParser::Handle353; goto exec;
			case 366:	parser_func = &IrcParser::Handle366; goto exec;
			case 372:	parser_func = &IrcParser::Handle372; goto exec;
			case 375:	parser_func = &IrcParser::Handle375; goto exec;
			case 376:	parser_func = &IrcParser::Handle376; goto exec;
			case 432:	parser_func = &IrcParser::Handle432; goto exec;
			case 433:	parser_func = &IrcParser::Handle433; goto exec;
			default:
				std::cout << fg_magenta << "Unhandled numeric: " << buf_data.code.c_str() << "\n";
				goto cleanup;
			}
		}
		else
		{
			switch ( buf_data.code[0] )
			{
			case 'C':
				{
					switch ( buf_data.code[1] )
					{
					case 'A':
						{
							switch ( buf_data.code[2] )
							{
							case 'P':	parser_func = &IrcParser::HandleCap; goto exec;
							default:
								goto unhandled_text;
							}
						}
					default:
						goto unhandled_text;
					}
				}
			case 'I':
				{
					switch ( buf_data.code[1] )
					{
					case 'N':
						{
							switch ( buf_data.code[2] )
							{
							case 'V':	parser_func = &IrcParser::HandleInvite; goto exec;
							default:
								goto unhandled_text;
							}
						}
					default:
						goto unhandled_text;
					}
				}
			case 'J':
				{
					switch ( buf_data.code[1] )
					{
					case 'O':	parser_func = &IrcParser::HandleJoin; goto exec;
					default:
						goto unhandled_text;
					}
				}
			case 'K':
				{
					switch ( buf_data.code[1] )
					{
					case 'I':
						{
							switch ( buf_data.code[2] )
							{
							case 'C':	parser_func = &IrcParser::HandleKick; goto exec;
							case 'L':	parser_func = &IrcParser::HandleKill; goto exec;
							default:
								goto unhandled_text;
							}
						}
					default:
						goto unhandled_text;
					}
				}
			case 'M':
				{
					switch ( buf_data.code[1] )
					{
					case 'O':	parser_func = &IrcParser::HandleMode; goto exec;
					default:
						goto unhandled_text;
					}
				}
			case 'N':
				{
					switch ( buf_data.code[1] )
					{
					case 'I':	parser_func = &IrcParser::HandleNick; goto exec;
					case 'O':	parser_func = &IrcParser::HandleNotice; goto exec;
					default:
						goto unhandled_text;
					}
				}
			case 'P':
				{
					switch ( buf_data.code[1] )
					{
					case 'A':	parser_func = &IrcParser::HandlePart; goto exec;
					case 'O':	parser_func = &IrcParser::HandlePong; goto exec;
					case 'R':	parser_func = &IrcParser::HandlePrivmsg; goto exec;
					default:
						goto unhandled_text;
					}
				}
			case 'Q':
				{
					switch ( buf_data.code[1] )
					{
					case 'U':	parser_func = &IrcParser::HandleQuit; goto exec;
					default:
						goto unhandled_text;
					}
				}
			case 'T':
				{
					switch ( buf_data.code[1] )
					{
					case 'O':	parser_func = &IrcParser::HandleTopic; goto exec;
					default:
						goto unhandled_text;
					}
				}
			default:
unhandled_text:
				std::cout << fg_magenta << "Unhandled text-code: " << buf_data.code.c_str() << "\n";
				goto cleanup;
			}
		}
exec:
		//dbgprint("Executing Handler, 0x%p\n", parser_func);

		// calling member function pointers is fun!
		((IrcParser*)this->*parser_func)(connection, &buf_data, &sender);
	}

cleanup:

	return EIrcStatus::OK;

no_connection:
	std::cerr << fg_red << "The supplied connection was a nullptr\n";
	return EIrcStatus::MissingParameter;
extract_failed:
	return EIrcStatus::ParsingError;
split_failed:
	return EIrcStatus::ParsingError;
invalid_code:
	std::cerr << fg_red << "An invalid code was received: " << buf_data.code.c_str() << "\n";
	return EIrcStatus::InvalidData;
srv_error:
	std::cerr << fg_red << "The server closed the connection: " << err << "\n";
	connection->_state = CS_Disconnected;
	return EIrcStatus::ServerClosed;
}



EIrcStatus
IrcParser::ProcessNextSendQueueItem(
	std::shared_ptr<IrcConnection> connection
) const
{
	if ( connection == nullptr )
	{
		std::cerr << fg_red << "The supplied connection was a nullptr\n";
		return EIrcStatus::MissingParameter;
	}

	if ( connection->_send_queue.empty() )
	{
		return EIrcStatus::QueueEmpty;
	}

	// to add processing...

	return EIrcStatus::OK;
}



EIrcStatus
IrcParser::RunParser()
{
	try
	{

	for ( ;; )
	{
#if defined(_WIN32)
		DWORD	wait_ret;

		ResetEvent(_sync_event);

		wait_ret = WaitForSingleObject(_sync_event, INFINITE);

		/* object type can only return _0 or FAILED, as it never
		 * times out and is not a mutex */
		if ( wait_ret == WAIT_FAILED )
		{
			std::cerr << fg_red << "WaitForSingleObject() failed for the parser sync event; last error= " << GetLastError() << "\n";
			return EIrcStatus::OSAPIError;
		}
		else if ( wait_ret == WAIT_OBJECT_0 )
#else
		sync_event_wait(&_sync_event);
#endif
		{
			// abort immediately if the app is quitting
			if ( runtime.IsQuitting() )
				break;

			for ( auto c : _irc_engine->Pools()->IrcConnections()->Allocated() )
			{
				ParseConnectionQueues(c);
			}
		}
	}

	}
	catch ( std::exception& e )
	{
		std::cerr << fg_red << "Caught an exception; " << e.what() << "\n";
	}
	catch ( ... )
	{
		std::cerr << fg_red << "Caught an unhandled exception\n";
	}

#if defined(_WIN32)
	runtime.ThreadStopping(GetCurrentThreadId(), __FUNCTION__);
#else
	runtime.ThreadStopping(pthread_self(), __FUNCTION__);
#endif

	return EIrcStatus::OK;
}



EIrcStatus
IrcParser::SplitSender(
	const char* buffer,
	ircbuf_sender* sender
) const
{
	/* although we modify buffer, we will not alter its length (we only
	 * replace spaces with nul characters), therefore this function is still
	 * safe to use with a caller of a CHARSTRINGTYPE->c_str() */

	char*	nickname = (char*)&buffer[0];
	char*	ident = nullptr;
	char*	hostmask = nullptr;

	if ( buffer == nullptr )
		goto no_buffer;
	if ( sender == nullptr )
		goto no_sender;

	ident = strchr(nickname, '!');

	if ( ident == nullptr )
	{
		/* message from the server - ideally should be a check with what
		 * we received on the initial connection, and if it doesn't 
		 * match it should be regarded as invalid. */
		sender->nickname	= nickname;
		return EIrcStatus::OK;
	}

	// null-out the space, and proceed to the 'first' character
	*ident = '\0';
	ident++;

	// copy the data, up to the first space (which is now a nul)
	sender->nickname	= nickname;

	hostmask = strchr(ident, '@');

	if ( hostmask == nullptr )
		goto missing_data;

	*hostmask = '\0';
	hostmask++;

	sender->ident		= ident;
	sender->hostmask	= hostmask;

	return EIrcStatus::OK;

no_buffer:
	std::cerr << fg_red << "The supplied buffer was a nullptr\n";
	return EIrcStatus::MissingParameter;
no_sender:
	std::cerr << fg_red << "The supplied sender struct was a nullptr\n";
	return EIrcStatus::MissingParameter;
missing_data:
	std::cerr << fg_red << "The hostmask is missing where expected: " << ident << "\n";
	return EIrcStatus::ParsingError;
}



void
IrcParser::TriggerSync() const
{
#if defined(_WIN32)
	if ( !SetEvent(_sync_event) )
		goto evt_failure;

	return;

evt_failure:
	std::cerr << fg_red << "Failed to signal the parser event; last error= " << GetLastError() << "\n";

#elif defined(__linux__) || defined(BSD)

	sync_event_set(&_sync_event);

#endif
}



END_NAMESPACE

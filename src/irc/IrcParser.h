#pragma once

/**
 * @file	IrcParser.h
 * @author	James Warren
 * @brief	The class that parses all input from an IrcConnection
 */



#include <memory>			// std::shared_ptr

#include <api/Runtime.h>
#include "IrcObject.h"
#include "IrcListener.h"
#include "irc_structs.h"
#include "irc_status.h"


#if defined(USING_BOOST_NET)
#	include <boost/asio.hpp>
#else
#	include "nethelper.h"
#endif

#if defined(__linux__) || defined(BSD)
#	include "sync_event.h"
#endif



BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class IrcConnection;
class IrcEngine;



/**
 * All the connections queues are utilized by this class.
 *
 * This class is NOT an IrcListener - it is independent, and is actually what
 * triggers all the OnXxx methods for the listeners. The data must be parsed
 * before we can notify them, after all.
 *
 * A dedicated thread gets synchronized from an IrcConnection class when it
 * gets data; the function then grabs the data from the connection queue,
 * parses it, populates the last activity from the connection, and notifies
 * all listeners as to the event.
 *
 * None of the HandleXxx functions validate the input pointers, as they should
 * be pre-validated (unless for some unknown reason you would try to call them
 * yourself).
 *
 * This class has direct private access to most other objects; while this means
 * we're strongly coupled, it enables us to update the objects at the time we're
 * parsing the relevant data (there's no other appropriate time to do it) so it
 * just makes it less complex without sacrificing too much in terms of
 * maintenance of design.
 *
 * @class IrcParser
 */
class SBI_IRC_API IrcParser : public IrcObject
{
	// we are created on the stack in IrcEngine.cc
	friend class IrcEngine;
	// calls ExecParser
	//friend irc_status Runtime::CreateThread(E_THREAD_TYPE, void*, const char*);
private:

#if defined(_WIN32)
	HANDLE			_sync_event;
#else
	mutable sync_event	_sync_event;
#endif


	/**
	 * Used for creating a thread for the RunParser() function.
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
	 * @param[in] thisptr A pointer to the IrcParser class
	 * @return Always returns 0. Whatever happens in the called function
	 * stays unique and separate.
	 * @sa Runtime::CreateThread, IrcConnection::ExecEstablishConnection
	 */
#if defined(_WIN32)
	static uint32_t __stdcall
#elif defined(__linux__)
	static void*
#endif
	ExecParser(
		void* thisptr
	);


	/**
	 * Parses the 001 numeric.
	 *
	 * As this is the first handler, it may contain some comments that are
	 * pertinent to some of the other handler functions, and design choices.
	 *
	 * All the HandleXxx functions have the same structure: goto's on 
	 * errors, and everything ends up at a cleanup label, which returns the 
	 * actual return value. Returning is not allowed anywhere else, even if 
	 * there's nothing to cleanup.
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle001(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 002 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle002(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 003 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle003(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 004 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle004(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 005 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle005(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 331 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle331(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 332 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle332(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 333 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle333(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 353 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle353(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 366 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle366(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 372 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle372(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 375 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle375(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 376 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle376(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 432 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle432(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses the 433 numeric
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	Handle433(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses CAP
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandleCap(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses INVITE
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandleInvite(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses JOIN
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandleJoin(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses KICK
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandleKick(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses KILL
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandleKill(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses MODE
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandleMode(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses NICK
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandleNick(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses NOTICE 
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandleNotice(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses PART
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandlePart(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses PONG
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandlePong(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses PRIVMSG
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandlePrivmsg(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses QUIT
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandleQuit(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 * Parses TOPIC
	 *
	 * @param connection The connection the data was received from
	 * @param data The segmented data received into sender, code + data
	 * @param sender The nickname, ident and hostmask combo
	 * @return If the processed data was valid, EIrcStatus::Ok is returned.
	 * @return If parsing/processing fails, returns the relevant EIrcStatus.
	 */
	EIrcStatus
	HandleTopic(
		std::shared_ptr<IrcConnection> connection,
		ircbuf_data* data,
		ircbuf_sender* sender
	);

	/**
	 *
	 *
	 */
	EIrcStatus
	ParseConnectionQueues(
		std::shared_ptr<IrcConnection> connection
	);


	/**
	 *
	 */
	EIrcStatus
	RunParser();


	// private constructor; we want one instance that is controlled
	IrcParser();

public:
	~IrcParser();


	/**
	 * Destroys the parsing thread and cleans up any other associated class
	 * data, such as the sync event.
	 *
	 * Always called in the destructor.
	 */
	void
	Cleanup();


#if 0	// Code Removed: Now handled by Runtime::CreateThread
	/**
	 * Creates the thread which waits for triggered events, then iterates every
	 * connection, emptying its recv, and then send queues. We *could* create a
	 * parser thread for each connection, but I'd rather use less resources in a
	 * continual state, and take the extra cpu hit when processing.
	 * Either way, the visible effects are almost certainly non-existent.
	 *
	 * @return Returns true if it was created, otherwise false
	 */
	EIrcStatus
	CreateThread();
#endif


	/**
	 * Analyzes the supplied @a buffer and extracts the sender, data-code and data,
	 * storing it in the supplied structure. The entire received data from the
	 * server should be input, otherwise erroneous results will be returned.
	 * Note that the original buffer will be modified, with inserted NUL's.
	 *
	 * @param buffer The buffer to have the data extracted from
	 * @param data The structure used to store the extracted data
	 * @return Returns IRCS_OK if the data is retrieved successfully, otherwise
	 * the appropriate EIrcStatus will be returned.
	 */
	EIrcStatus
	ExtractIrcBufData(
		const char* buffer,
		ircbuf_data* data
	) const;


	/**
	 * Determines if the supplied string has a channel prefix, determined from the
	 * data initially received from the server. Only checks the very first char of
	 * the supplied @a str.
	 *
	 * @param connection The irc_connection containing the server data
	 * @param str The string to check for a prefix
	 * @return Returns true/false as required, if the supplied string begins
	 * with a channel prefix (as previously supplied from the server).
	 */
	bool
	HasChannelPrefix(
		const std::shared_ptr<IrcConnection> connection,
		char* str
	) const;


	/**
	 * Checks whether the supplied mode has any 'data' associated with it - 
	 * used when parsing mode messages, to determine the number of 
	 * parameters there should be in retrieval.
	 *
	 * @param connection The irc_connection owning the data
	 * @param is_set Boolean flag if the mode needs to be 'set' to have data
	 * @param mode The mode itself
	 * @return Returns false if any parameter is invalid, or the mode does 
	 * indeed not have any arguments with it. Returns true if it does.
	 */
	bool
	ModeHasArgument(
		const std::shared_ptr<IrcConnection> connection,
		bool is_set,
		char mode
	) const;


	/**
	 * Parses the next recv queue item, determining the IRC message code (if
	 * any), and interpreting the data, preparing it for usage in the parser
	 * handling functions (HandleXXX). Also sends the server initialization 
	 * if it has not yet been done for this connection.
	 *
	 * @param connection The connection containing the recv queue to process
	 * @return Returns IRCS_OK if the data was parsed, even if no handler
	 * exists for the text
	 * @return Returns IRCS_QueueEmpty if there are no queue items
	 * @return Otherwise, the appropriate EIrcStatus is returned
	 */
	EIrcStatus
	ParseNextRecvQueueItem(
		std::shared_ptr<IrcConnection> connection
	) const;


	/**
	 * Parses the next send queue item, which should be a raw IRC message.
	 *
	 * @param connection The connection containing the send queue to process
	 * @return Returns IRCS_OK if a queue item was popped and sent off to
	 * the server
	 * @return Returns IRCS_QueueEmpty if there are no queue items
	 * @return Otherwise, the appropriate EIrcStatus is returned
	 */
	EIrcStatus
	ProcessNextSendQueueItem(
		std::shared_ptr<IrcConnection> connection
	) const;


	/**
	 * Checks the supplied data for the IRC separator (:) and/or the end of
	 * the string without one, and retrieves the next 'parameter', if any,
	 * of the data.
	 *
	 * @param data The pointer-to-char-pointer to check
	 * @return Returns a pointer to the extracted parameter on 'success', 
	 * otherwise a nullptr is returned. If the number of arguments supplied 
	 * to ParseParameters() is accurate for the message type, this shouldn't
	 * ever return a nullptr.
	 */
	char*
	ParseParam(
		char** data
	) const;


	/**
	 * Splits the supplied buffer into the IRC separated list of parameters
	 * (:) storing the results in the supplied arguments, by calling
	 * ParseParam().
	 *
	 * @code
	 char*    extracted_channel = NULL;
	 char*    extracted_kicked = NULL;
	 char*    extracted_kick_message = NULL;

	 if ( !ParseParameters(data->data, 3, &extracted_channel, &extracted_kicked, &extracted_kick_message) )
		goto parse_failure;

	 // the extracted_* variables now contain an allocated buffer
	 * @endcode
	 *
	 * @param[in] buffer The buffer containing the original string from the 
	 * server
	 * @param[in] num_args The number of arguments to extract/parse from the
	 * buffer
	 * @param[out] ... The variable list of arguments, which should be 
	 * pointers to unallocated buffers (char**)
	 * @return Returns true if the arguments are populated successfully, 
	 * or false on any failure.
	 */
	bool
	ParseParameters(
		const char* buffer,
		uint32_t num_args,
		...
	) const;


	/**
	 * Splits the supplied string, which should be in the format 
	 * $nick!$ident@$host into its three separate parts, storing them in the
	 * supplied sender struct. No memory is allocated in doing so, but the 
	 * original string is modified.
	 *
	 * @param[in] buffer The buffer containing the sender information
	 * @param[out] sender The ircbuf_sender to store the separate sender 
	 * parts in
	 * @return Returns IRCS_Success if the sender was split into its three 
	 * parts (or single part if it is a server), otherwise the appropriate 
	 * EIrcStatus
	 */
	EIrcStatus
	SplitSender(
		const char* buffer,
		ircbuf_sender* sender
	) const;


	/**
	 * Notifies the parser there is work to do. Using this rather than a
	 * poll, which would use more CPU time and cause a delay in send/recv.
	 *
	 * @sa RunParser()
	 */
	void
	TriggerSync() const;
};



END_NAMESPACE

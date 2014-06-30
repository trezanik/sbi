#pragma once

/**
 * @file	irc_status.h
 * @author	James Warren
 * @brief	IRC-specific status codes returned from functions
 */



#include <api/definitions.h>
 

BEGIN_NAMESPACE(APP_NAMESPACE)



/**
 * IRC status code enumeration.
 *
 * These are not error codes; for example, when creating a new user in a channel
 * we'll try to Get() that user when we expect it to not exist (in order to
 * verify we're not trying to add the same user twice, which would mean we've
 * been handling something incorrectly or not at all); as a result, the return
 * value of ObjectNotFound is intended and should not be reported as an error.
 *
 * @enum EIrcStatus
 */
enum class EIrcStatus
{
	OK,
	MissingParameter,	// expected parameter was missing or null
	InvalidParameter,	// supplied parameter is invalid
	ObjectNotFound,		// a user/channel/etc. was not found
	ParsingError,		// data parsing failed when should never
	OSAPIError,		// OS function failed
	QueueEmpty,		// The queue contained no items
	NickIsNotClient,	// Expected client nickname, did not match
	InvalidState,		// The state is incompatible or invalid in context
	ObjectAddError,		// Failed to create an IrcObject
	ObjectFreeError,	// Failed to free an IrcObject
	InvalidData,		// The data is not in a recognizable format
	LimitExceeded,		// A limit was exceeded
	ServerClosed,		// The server closed the connection
	UnknownResponse,	// Server supplied valid but unknown data
	NoOwner,		// The objects parent was a nullptr
	NoMoreNicks,		// There are no nicknames left to use
	OpenSSLError,		// OpenSSL encountered an error
	LookupFailed,		// DNS lookup failed
	Unknown			// Placeholder/default; should never see this reported
};



END_NAMESPACE

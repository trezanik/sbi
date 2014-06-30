#pragma once

/**
 * @file	rfc1459.h
 * @author	James Warren
 * @brief	Definitions for the IRC RFC 1459
 */



#include <api/char_helper.h>


BEGIN_NAMESPACE(APP_NAMESPACE)


// all variables are prefixed with the RFC number
#define RFC1459_CASEMAPPING	"rfc1459")
#define RFC1459_CHANNEL_LEN	200
#define RFC1459_CHAN_TYPES	"#&")
#define RFC1459_MODES		3
#define RFC1459_NICK_LEN	9
#define RFC1459_PREFIX		"(ov)@+")

// 'USER' cmd:	<username> <hostname> <servername> <realname>


END_NAMESPACE

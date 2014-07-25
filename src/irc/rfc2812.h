#pragma once

/**
 * @file	src/irc/rfc2812.h
 * @author	James Warren
 * @brief	Definitions for the IRC RFC 2812
 */



#include <api/char_helper.h>


BEGIN_NAMESPACE(APP_NAMESPACE)


// all variables are prefixed with the RFC number
#define RFC2812_MAX_NICKNAME_LENGTH		9

// 'USER' cmd:	<user> <mode> <unused> <realname>


END_NAMESPACE

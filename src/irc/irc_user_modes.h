#pragma once

/**
 * @file	irc_user_modes.h
 * @author	James Warren
 * @brief	Defines the user modes known for IrcUsers
 */



#include <api/definitions.h>


BEGIN_NAMESPACE(APP_NAMESPACE)


/**
 * Known server user modes - used for arranging the nicklist and checking 
 * authorization internally.
 *
 * At time of writing, the most modes I've ever seen on a server is six:
 * no_mode, Voice, HalfOp, Op, Admin, and Owner
 * UM_Unknown is reserved for unknown types, and will be sorted separately.
 * No point putting them at the top, since you can't go higher than owner..
 *
 * @enum E_USER_MODE
 */
enum E_USER_MODE
{
	UM_None = 0x0,		/**< Default user state */
	UM_Voice = 0x1,		/**< User is 'voiced' */
	UM_HalfOp = 0x2,	/**< User has 'halfop' */
	UM_Op = 0x4,		/**< User has 'op' */
	UM_Admin = 0x8,		/**< User is a channel admin */
	UM_Owner = 0x10,	/**< User is the channel owner */
	UM_Unknown = 0xFF	/**< Plain unknown type */
};



END_NAMESPACE

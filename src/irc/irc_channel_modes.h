#pragma once

/**
 * @file	irc_channel_modes.h
 * @author	James Warren
 * @brief	Defines the available flags for an IrcChannel
 */



#include <api/definitions.h>


BEGIN_NAMESPACE(APP_NAMESPACE)


#define CHANFLAG_RAW		0x00000000	/**< Default Channel State */
#define CHANFLAG_ACTIVE		0x00000001	/**< Channel object exists, but we're not actually in it */
#define CHANFLAG_RESERVED0	0x00000002
#define CHANFLAG_RESERVED1	0x00000004
#define CHANFLAG_RESERVED2	0x00000008
#define CHANFLAG_USERLIMIT	0x00000010	/**< Limit to number of users that can join */
#define CHANFLAG_KEY		0x00000020	/**< Password 'key' required to join */
#define CHANFLAG_PRIVATE	0x00000040	/**< Must be member of channel to find it */
#define CHANFLAG_SECRET		0x00000080	/**< Server will pretend channel doesn't exist for some commands */
#define CHANFLAG_TOPIC_PROTECT	0x00000100	/**< Only ops can set topic */
#define CHANFLAG_NO_MESSAGES	0x00000200	/**< No external messages allowed in */
#define CHANFLAG_INVITE_ONLY	0x00000400	/**< Can only join if invited or on invite-list */
#define CHANFLAG_MODERATED	0x00000800	/**< Moderated - only users with +v or greater may talk */
#define CHANFLAG_BANDWIDTH_SAVE	0x00001000	/**< Bandwidth saver - don't receive messages if idle */
#define CHANFLAG_SECURE_ONLY	0x00002000	/**< Only users connected via SSL may join the channel */
#define CHANFLAG_MODERATE_UNREG	0x00004000	/**< Moderated, but anyone authenticated is allowed to talk */
#define CHANFLAG_REGGED_ONLY	0x00008000	/**< Only authenticated users may join the channel */
#define CHANFLAG_IRC_OPS_ONLY	0x00010000	/**< Only IRC Ops may join the channel */


END_NAMESPACE

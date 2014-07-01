
/**
 * @file	IrcObject.cc
 * @author	James Warren
 * @copyright	James Warren, 2013-2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <api/Allocator.h>	// self-deletion
#include <api/Runtime.h>	// acquire irc engine
#include <api/Log.h>		// logging
#include "IrcEngine.h"		// irc engine
#include "IrcObject.h"		// prototypes


BEGIN_NAMESPACE(APP_NAMESPACE)


IrcObject::IrcObject()
{
	/* other engine accesses use the IRC_RUNTIME_MODULE macro; we don't here
	 * because we want a permanent assignment. */
	if (( _irc_engine = static_cast<IrcEngine*>(runtime.GetObjectFromModule(IRC_MODULE_NAME))) == nullptr )
	{
		throw std::runtime_error("The irc engine could not be acquired");
	}
}



IrcObject::~IrcObject()
{
}



END_NAMESPACE

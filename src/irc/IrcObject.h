#pragma once

/**
 * @file	src/irc/IrcObject.h
 * @author	James Warren
 * @brief	The base class for all application objects
*/



#include <atomic>			// C++11 atomic operations
#include <set>				// storing detailed reference info
#include <api/char_helper.h>



BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class IrcEngine;




/**
 * The base class for all application objects.
 *
 * Uses a custom reference counter to auto-delete the object when nothing is
 * referencing it anymore. Done by C++11 atomic operations, which retains the
 * performance.
 *
 * Yes, we could use a shared_ptr for every object - and yes, I initially started
 * designing it that way. I very quickly encountered unbreachable hurdles, and
 * started resorting to hacks, until I determined it was quicker, easier, more
 * fun, and less to maintain by doing it our custom way!
 *
 * @class Object
 */
class SBI_IRC_API IrcObject
{
private:
	//NO_CLASS_ASSIGNMENT(IrcObject);
	NO_CLASS_COPY(IrcObject);


protected:
	/* derived classes need to notify the engine of their creation, so do
	 * not make private */
	IrcEngine*		_irc_engine;

public:
	IrcObject();
	// Make this class abstract, must be derived
	virtual ~IrcObject() = 0;


};


END_NAMESPACE

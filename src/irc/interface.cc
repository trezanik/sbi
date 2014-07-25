
/**
 * @file	src/irc/interface.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <memory>

#include <api/interface.h>
#include "IrcEngine.h"




std::unique_ptr<APP_NAMESPACE::IrcEngine>	irc_engine;



int
destroy_interface()
{
	using namespace APP_NAMESPACE;

	irc_engine.release();

	return (int)EInterfaceStatus::Ok;
}



void*
instance(
	void* params
)
{
	using namespace APP_NAMESPACE;

	// unused
	params;

	return irc_engine.get();
}



int
spawn_interface()
{
	using namespace APP_NAMESPACE;

	// add custom menu options, load modules, etc.
	irc_engine.reset(new IrcEngine);

	return (int)EInterfaceStatus::Ok;
}

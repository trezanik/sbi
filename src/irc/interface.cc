
/**
 * @file	interface.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <memory>

#include <api/interface.h>
#include "IrcEngine.h"




using namespace APP_NAMESPACE;


std::unique_ptr<IrcEngine>	irc_engine;



int
#if defined(_WIN32)
__stdcall
#endif
destroy_interface()
{
	irc_engine.release();

	return (int)EInterfaceStatus::Ok;
}



void*
#if defined(_WIN32)
__stdcall
#endif
instance(
	void* params
)
{
	// unused
	params;

	return irc_engine.get();
}



int
#if defined(_WIN32)
__stdcall
#endif
spawn_interface()
{
	// add custom menu options, load modules, etc.
	irc_engine.reset(new IrcEngine);

	return (int)EInterfaceStatus::Ok;
}

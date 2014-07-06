
/**
 * @file	IpcListener.cc
 * @author	James Warren
 * @copyright	James Warren, 2013
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <api/Runtime.h>
#include <api/Interprocess.h>

#include "IpcListener.h"
#include "IrcEngine.h"


BEGIN_NAMESPACE(APP_NAMESPACE)



IpcListener::IpcListener(
	IrcEngine* engine
)
{
	_irc_engine = engine;
	_irc_engine->AttachListener(this);

	runtime.Interprocess()->Open("libirc", EIPCAction::Bi);
}



IpcListener::~IpcListener()
{
	_irc_engine->DetachListener(this);
}



void
IpcListener::On001(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
	auto ipc = runtime.Interprocess()->Connect("libirc", EIPCAction::Bi);

	if ( ipc == nullptr )
		return;

	ipc->Write(activity.data.c_str());
}



void
IpcListener::OnJoin(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnKick(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnMode(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnNick(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnNotice(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnPart(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnPrivmsg(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnQuit(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnOurJoin(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnOurKick(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnOurKicked(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnOurNick(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnOurNotice(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnOurPrivmsg(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
	auto ipc = runtime.Interprocess()->Connect("libirc", EIPCAction::Bi);

	if ( ipc == nullptr )
		return;

	ipc->Write(activity.data.c_str());
}



void
IpcListener::OnOurPart(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



void
IpcListener::OnOurQuit(
	std::shared_ptr<IrcConnection> connection,
	irc_activity& activity
)
{
}



END_NAMESPACE

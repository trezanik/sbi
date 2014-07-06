#pragma once

/**
 * @file	IpcListener.h
 * @author	James Warren
 * @brief	Receives IRC events that get forwarded through the IPC methods
 */



#include <api/Interprocess.h>
#include <api/Ipc.h>

#include "IrcListener.h"


BEGIN_NAMESPACE(APP_NAMESPACE)



class IrcEngine;



/**
 * This class attaches itself as a listener to all IRC events, and then forwards
 * them through the IPC setup to inform e.g. the GUI, which can then modify the
 * interface as applicable (new line of text, etc.)
 *
 * @class IpcListener
 */
class SBI_IRC_API IpcListener : public IrcListener
{
	// we are created by the IrcEngine
	friend class IrcEngine;
private:
	std::shared_ptr<Ipc>	_ipc;
	IrcEngine*		_irc_engine;

	IpcListener(
		IrcEngine* engine
	);
protected:
public:
	~IpcListener();







	/* we override what we're interested in - bot related/usable messages.
	 * Naturally over time, the more features you require will result in the
	 * more messages you want notification of. Implement anything you want
	 * to override from IrcListener.h */

	virtual void
	On001(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnJoin(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnKick(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnMode(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnNick(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnNotice(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnPart(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnPrivmsg(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnQuit(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnOurJoin(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnOurKick(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnOurKicked(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnOurNick(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnOurNotice(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnOurPrivmsg(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnOurPart(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

	virtual void
	OnOurQuit(
		std::shared_ptr<IrcConnection> connection,
		irc_activity& activity
	) override;

};



END_NAMESPACE

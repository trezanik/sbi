
/**
 * @file	src/Qt5GUI/rpc_commands.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <QtWidgets/qapplication.h>

#include <api/RpcServer.h>
#include <api/Runtime.h>			// RpcServer accessor

#include "rpc_commands.h"
#include "library.h"
#include "UI.h"
#include "ui_windowtype.h"
#include "UiThreadExec.h"



BEGIN_NAMESPACE(APP_NAMESPACE)
BEGIN_NAMESPACE(GUI_NAMESPACE)



json_spirit::Value
gui_CreateWindow(
	const json_spirit::Array& params,
	bool help
)
{
	std::shared_ptr<window_params>	p;
	std::unique_ptr<UiThreadExec>	ui_thread;

	if ( params.size() != 4 )
	{
		throw std::runtime_error("Invalid parameter count");
	}

	p.reset(new window_params);
	p->window_type	= params[0].get_uint64();
	p->text		= params[1].get_str();
	p->page		= params[2].get_uint64();
	p->parent	= params[3].get_uint64();
	
	ui_thread.reset(new UiThreadExec(p));

	QObject::connect(ui_thread.get(), SIGNAL(start_create()), ui_thread.get(), SLOT(do_create()));

	// must be in GUI thread
	ui_thread->moveToThread(QApplication::instance()->thread());
	ui_thread->start();
	emit ui_thread->start_create();
	/* since we're a freshly created RpcHandler thread, we block nothing
	 * from executing while we wait for the start_create() work to finish.
	 * Use the default (indefinite) wait, returning when run() finishes. */
	ui_thread->wait();
	
	return 0;
}



json_spirit::Value
gui_DestroyWindow(
	const json_spirit::Array& params,
	bool help
)
{
	return 0;
}



json_spirit::Value
gui_GetStackWidget(
	const json_spirit::Array& params,
	bool help
)
{
	if ( params.size() != 0 )
	{
		throw std::runtime_error("Invalid parameter count");
	}

	return (uint64_t)g_ui->StackWidget();
}



json_spirit::Value
gui_Help(
	const json_spirit::Array& params,
	bool help
)
{
	if ( help || params.size() > 1 )
	{
		throw std::runtime_error(
			"help [command]\n"
			"List commands, or get help for a command."
		);
	}

	std::string	command;

	if ( params.size() > 0 )
		command = params[0].get_str();

	// rpc table for the GUI? 
	// Qt5GUI()->GetRpcTable()->Help(command)

	return 0;
}



END_NAMESPACE
END_NAMESPACE

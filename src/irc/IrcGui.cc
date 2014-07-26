
/**
 * @file	src/irc/IrcGui.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <stdexcept>

#if defined(USING_DEFAULT_QT5_GUI)
#	include <QtWidgets/qstackedwidget.h>
#	include <QtWidgets/qwidget.h>
#	if IS_VISUAL_STUDIO
#		if IS_DEBUG_BUILD
#			pragma comment ( lib, "Qt5Cored.lib" )
#			pragma comment ( lib, "Qt5Guid.lib" )
#			pragma comment ( lib, "Qt5Widgetsd.lib" )
#		else
#			pragma comment ( lib, "Qt5Core.lib" )
#			pragma comment ( lib, "Qt5Gui.lib" )
#			pragma comment ( lib, "Qt5Widgets.lib" )
#		endif
#	endif
#	include <Qt5GUI/UI.h>
#endif

#include "IrcGui.h"



BEGIN_NAMESPACE(APP_NAMESPACE)




#if defined(USING_DEFAULT_QT5_GUI)


using namespace GUI_NAMESPACE;


IrcGui::IrcGui()
{
#if 0
	/* @important 
	 * From this constructor (i.e. the first call to the GUI, which is done
	 * the second IrcEngine is constructed), we are accessing the GUI in the
	 * external library directly; it MUST exist prior to this.
	 * At time of writing, we currently can only load the libirc through the
	 * GUI, so we can guarantee that it exists, and will always exist as 
	 * long as the Irc objects do. 
	 */
	_stack_widget = g_ui->StackWidget();

	if ( _stack_widget == nullptr )
		throw std::runtime_error("The stack widget to utilize is a nullptr");
#endif
}



IrcGui::~IrcGui()
{
}



void
IrcGui::CreateMainPage()
{
	_main_page = new QWidget;

	_stack_widget->addWidget(_main_page);
}



void
IrcGui::CreateChannelPage()
{
	QWidget*	channel_widget = new QWidget;

	_stack_widget->addWidget(channel_widget);
}



void
IrcGui::CreateNetworkPage()
{
	QWidget*	network_widget = new QWidget;

	_stack_widget->addWidget(network_widget);
}



void
IrcGui::CreateQueryPage()
{
	QWidget*	query_widget = new QWidget;

	_stack_widget->addWidget(query_widget);
}



#endif	// USING_DEFAULT_QT5_GUI




END_NAMESPACE

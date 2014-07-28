
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
#if defined(USING_JSON_SPIRIT_RPC)
#	include <json_spirit/json_spirit_utils.h>
#endif

#include "IrcGui.h"



BEGIN_NAMESPACE(APP_NAMESPACE)




#if defined(USING_DEFAULT_QT5_GUI)


using namespace GUI_NAMESPACE;



IrcGui::IrcGui()
{
}



IrcGui::~IrcGui()
{
}



void
IrcGui::CreateMain()
{
	// rpcexec -> get stack widget
	// rpcexec -> createwindow(page)
	// rpcexec -> createwindow(output, page)
	// rpcexec -> createwindow(tree, page)



}



void
IrcGui::CreateChannel()
{
	
}



void
IrcGui::CreateNetwork()
{
	
}



void
IrcGui::CreateQuery()
{
	
}



#endif	// USING_DEFAULT_QT5_GUI




END_NAMESPACE

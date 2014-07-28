#pragma once

/**
 * @file	src/irc/IrcGui.h
 * @author	James Warren
 * @brief	IRC-specific GUI functionality
 */



/* for claritys sake, each GUI type (Qt5, WxWidgets, etc.) has its own class
 * definition within their own preprocessor blocks, despite them having the same
 * name. This also separates any mandatory inclusions. 
 *
 * The priority is also to get one user interface actually fully functional 
 * before supporting hand-made ones, as the default is more likely to be used.
 */


#if defined(USING_DEFAULT_QT5_GUI)
#	include <Qt5GUI/RpcWidget.h>
#endif


#include <api/definitions.h>		// namespace
#include <api/RpcClient.h>


BEGIN_NAMESPACE(APP_NAMESPACE)



// forward declarations
class IrcEngine;



#if defined(USING_DEFAULT_QT5_GUI)
// Default, official Qt5 GUI code

/**
 * All user-interface related tasks are executed or called through this class;
 * none of the other classes in this library handle ANY UI code at all, with the
 * sole exception of the accessor within IrcEngine.
 *
 * @class IrcGui
 */
class IrcGui : public RpcClient
{
	// we are created on the stack in IrcEngine's UI() method
	friend class IrcEngine;
private:
	NO_CLASS_ASSIGNMENT(IrcGui);
	NO_CLASS_COPY(IrcGui);


	// IRC page, the widget added to the QStackedWidget
	GUI_NAMESPACE::RpcQWidget*	_irc_page;

	// output window for all IRC input/output
	GUI_NAMESPACE::RpcQWidget*	_output;

	// Server/channel/query treeview
	GUI_NAMESPACE::RpcQWidget*	_tree;



	// private constructor; we want one instance that is controlled
	IrcGui();

protected:
public:
	~IrcGui();


	/**
	 * Creates the default IRC output page, contained within the UI's 
	 * QStackedWidget. Alongside this, the output and treeview windows are
	 * also created within this page.
	 */
	void
	CreateMain();


	void
	CreateChannel();

	void
	CreateNetwork();

	void
	CreateQuery();
};


#endif	// USING_DEFAULT_QT5_GUI




END_NAMESPACE

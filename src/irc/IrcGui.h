#pragma once

/**
 * @file	IrcGui.h
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
// forward declare Qt objects, save including big headers where possible
class QStackedWidget;
class QWidget;
#endif


#include <api/definitions.h>		// namespace



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
class IrcGui
{
	// we are created on the stack in IrcEngine's UI() method
	friend class IrcEngine;
private:
	NO_CLASS_ASSIGNMENT(IrcGui);
	NO_CLASS_COPY(IrcGui);


	QStackedWidget*		_stack_widget;
	QWidget*		_main_page;


	// private constructor; we want one instance that is controlled
	IrcGui();

protected:
public:
	~IrcGui();


	/**
	 * Creates the default IRC output page, contained within the UI's 
	 * QStackedWidget.
	 */
	void
	CreateMainPage();


	void
	CreateChannelPage();

	void
	CreateNetworkPage();

	void
	CreateQueryPage();
};


#endif	// USING_DEFAULT_QT5_GUI




END_NAMESPACE

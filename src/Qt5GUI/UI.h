#pragma once

/**
 * @file	src/Qt5GUI/UI.h
 * @author	James Warren
 * @brief	Qt5GUI root
 */



#include <memory>

#include <QtCore/qstring.h>
#include <QtCore/qobject.h>

#include <api/definitions.h>
#include <api/types.h>

#include "library.h"
#include "ui_status.h"
#include "ui_windowtype.h"



class QApplication;
class QMainWindow;
class QStackedWidget;
class Ui_MainWindow;
struct window_params;


// fix Windows namespace pollution
#if defined(CreateWindow)
#	undef CreateWindow
#endif


BEGIN_NAMESPACE(APP_NAMESPACE)
BEGIN_NAMESPACE(GUI_NAMESPACE)
#define MIN_APP_WINDOW_SIZE	QSize(400,300)
#define APP_SIZE_INCREMENT	QSize(4,4)
END_NAMESPACE
END_NAMESPACE



/**
 *
 *
 * @class UI
 */
class SBI_QT5GUI_API UI : public QObject
{
	Q_OBJECT

private:
	NO_CLASS_ASSIGNMENT(UI);
	NO_CLASS_COPY(UI);

#if defined(USING_LIBCONFIG)
	
#endif

	QApplication*		_app;		/**< Core Qt application */
	QMainWindow*		_wnd;		/**< the primary window */

	//Connections*		_dlg_conn;	/**< Connections dialog */
	//Profiles*		_dlg_prof;	/**< Profiles dialog */
	//Preferences*		_dlg_pref;	/**< Preferences dialog */

	Ui_MainWindow*		_base;		/**< The 'interfaceable' main window */


	/**
	 * Adds the RPC functions this class utilizes to the available list
	 * within the runtime.
	 */
	void
	PopulateRpcTable();


public:
	UI();
	~UI();


	/**
	 * Acquires the version information for the application
	 */
	QString
	ApplicationVersion(
		uint8_t version_format = 0
	) const;


	/**
	 * Creates the windows show for the initial user interface, containing the
	 * application window, menu and status bars.
	 *
	 * @return Returns EGuiStatus::Ok if everything is created without fault,
	 * otherwise the applicable status is returned
	 */
	APP_NAMESPACE::GUI_NAMESPACE::EGuiStatus
	CreateDefaultWindows();


	/**
	 * Creates a window with the specified parameters. Designed for use via
	 * RPC calls and should not be used internally - members are accessible
	 * directly and will be quicker + easier to work with.
	 *
	 * @warning
	 * Qt will generate an assertion failure if the thread calling this
	 * function is not the GUI thread. We supply and use the UiThreadExec
	 * class in order to work with this.
	 *
	 * @sa UiThreadExec
	 */
	APP_NAMESPACE::GUI_NAMESPACE::EGuiStatus
	CreateWindow(
		window_params* wnd_params
	);


	/**
	 * Queries the main application window for its size and position, then
	 * stores the results in the supplied references.
	 *
	 * @param[out] x The reference to store the windows x position in
	 * @param[out] y The reference to store the windows y position in
	 * @param[out] w The reference to store the windows width in
	 * @param[out] h The reference to store the windows height in
	 * @return Returns false if the window information cannot be required,
	 * in which case the supplied parameters are not modified; otherwise,
	 * this function returns true
	 */
	APP_NAMESPACE::GUI_NAMESPACE::EGuiStatus
	GetMainWindowParameters(
		int& x,
		int& y,
		int& w,
		int& h
	) const;


	/**
	 * 
	 */
	APP_NAMESPACE::GUI_NAMESPACE::EGuiStatus
	LoadConfig(
		const char* path
	);


	/**
	 * Actually runs the ui handler; in this case, the qt application.
	 */
	void
	Run() const;


	APP_NAMESPACE::GUI_NAMESPACE::EGuiStatus
	SetupSignals();


	/**
	 * Shows or hides the main window; allows the default creation mechanisms
	 * to complete fully before showing the main window, otherwise components
	 * are added while it is shown, and may cause noticeable defects.
	 *
	 * @param[in] enabled (Optional) If true, shows the window; false hides it.
	 * @throw EC_Abnormal if the class member pointer _wnd is nullptr
	 */
	void
	Show(
		bool enabled = true
	);


	/**
	 * Returns a pointer to the QStackedWidget that all interfaces add their
	 * unique page to. This is used by the interfaces to be able to add
	 * themselves in when they're loading.
	 */
	QStackedWidget*
	StackWidget() const;


	/* We use a similar method as that in Configuration to store settings.
	 * Since this class is only accessed by its own components, and should
	 * be standalone, I'm happy enough to not proxy the members, making them
	 * fully public.
	 * As always, custom libraries can do this however they want. */
	struct {
		uint32_t		flags;

		struct {
			uint32_t		height;
			uint32_t		width;
			int32_t			x;
			int32_t			y;
			std::string		title;
		} main_window;

		uint16_t		dialog_opacity;

		struct {
			uint32_t		locations_size;
			uint32_t		output_size;
			uint32_t		users_size;
		} tree_layout;
	} ui;


private slots:

	/**
	 * Displays the 'About' dialog for the application.
	 */
	void
	About() const;


	/**
	 * Displays the 'About' dialog for the version of Qt being used.
	 */
	void
	AboutQt() const;


	
	void
	OpenInterfacesLoadDialog() const;
	void
	OpenInterfacesUnloadDialog() const;

	void
	OpenModulesLoadDialog() const;
	void
	OpenModulesUnloadDialog() const;



	/**
	 * Quits the application.
	 */
	void
	Quit() const;

};



BEGIN_NAMESPACE(APP_NAMESPACE)
BEGIN_NAMESPACE(GUI_NAMESPACE)

// global variable - UI.cc
extern std::unique_ptr<UI>	g_ui;


END_NAMESPACE
END_NAMESPACE

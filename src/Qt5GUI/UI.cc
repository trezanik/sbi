#if defined(USING_QT5_GUI)
/**
 * @file	UI.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 */



#include <QtWidgets/QApplication.h>
#include <QtWidgets/QMainWindow.h>
#include <QtCore/qstring.h>

#if defined(USING_LIBCONFIG)
#	if IS_VISUAL_STUDIO
#		pragma warning ( push )
// C4290 : C++ exception specification ignored except to indicate a function is not __declspec(nothrow)
#		pragma warning ( disable : 4290 )
#	endif
#	include <libconfig/libconfig.h++>
#	if IS_VISUAL_STUDIO
#		pragma warning ( pop )
#	endif
#	if defined(_WIN32)
#		pragma comment ( lib, "libconfig++.lib" )
#	endif
#endif

#include <api/definitions.h>
#include <api/version.h>
#include <api/Configuration.h>
#include <api/utils.h>
#include <api/Log.h>
#include "library.h"			// prototypes and GUI library
#include "UI.h"				// library GUI class

// our dialogs/widgets/windows
#include "generated/ui_UI.h"		// Ui_MainWindow
#include "AboutDialog.h"
#include "InterfacesLoadDialog.h"
#include "InterfacesUnloadDialog.h"
#include "ModulesLoadDialog.h"
#include "ModulesUnloadDialog.h"



BEGIN_NAMESPACE(APP_NAMESPACE)
BEGIN_NAMESPACE(GUI_NAMESPACE)

/* Global pointer to the UI; this is only valid after spawn_interface() has
 * succeeded, and before destroy_interface() has been called. Any attempt to 
 * access it before then will most likely result in a crash.
 * Since we create the UI before any plugin access, only our internal projects
 * need to be careful regarding this. */
std::unique_ptr<UI>	g_ui;

END_NAMESPACE
END_NAMESPACE



using namespace APP_NAMESPACE;
using namespace GUI_NAMESPACE;



UI::UI()
{
}



UI::~UI()
{
}



void
UI::About() const
{
	AboutDialog*	dlg = new AboutDialog;

	dlg->SetModel(this);
	dlg->exec();

	delete dlg;
}



void
UI::AboutQt() const
{
	_app->aboutQt();
}



QString
UI::ApplicationVersion(
	uint8_t version_format
) const
{
	QString		ret;

	switch ( version_format )
	{
	case 1:	
		ret = APPLICATION_VERSION_STR;
		break;
	case 2: 
		ret = APPLICATION_VERSION_STR;
		ret += APPLICATION_VERSION_DATETIME;
		break;
	default:
		ret = APPLICATION_VERSION_STR;
		break;
	}

	return ret;
}



EGuiStatus
UI::CreateDefaultWindows()
{
	int32_t		argc = 0;
	char**		argv = nullptr;
	QString		title = "Social Bot Interface";
	QString		init_out;
	int32_t		w = ui.main_window.width;
	int32_t		h = ui.main_window.height;
	int32_t		x = ui.main_window.x;
	int32_t		y = ui.main_window.y;

	// create the Qt application (for execution) and the main window
	_app = new QApplication(argc, argv);
	_wnd = new QMainWindow;

	// can use _app->setStyle();

	// create the Qt Designer window
	_base = new Ui_MainWindow;
	_base->setupUi(_wnd);
	_base->retranslateUi(_wnd);

	// setup the titlebar
	if ( !ui.main_window.title.empty() )
	{
		title += " - ";
		title.append(ui.main_window.title.c_str());
	}
	// titlebar text must be set after the Qt Designer window is created
	_wnd->setWindowTitle(title);
	// resize the window based on config
	_wnd->resize(w, h);
	_wnd->setMinimumSize(MIN_APP_WINDOW_SIZE);
	_wnd->setSizeIncrement(APP_SIZE_INCREMENT);
#if defined(_WIN32)
	// avoid the taskbar when on Windows
	int32_t		min_x;
	int32_t		min_y;
	int32_t		max_x;
	int32_t		max_y;
	RECT		work_area;

	if ( SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area, 0) )
	{
		HWND	hwnd = (HWND)_wnd->winId();
		RECT	wnd_rect;

		GetWindowRect(hwnd, &wnd_rect);
		min_x = wnd_rect.left;
		max_x = wnd_rect.right;
		min_y = wnd_rect.top;
		max_y = wnd_rect.bottom;

		/* if we breach the taskbar position, then we override the cfg
		 * and move to the top-left most capable position */
		if ( x < min_x || x > max_x || y < min_y || y > max_y )
			_wnd->move(work_area.left, work_area.top);
		else
			_wnd->move(x, y);
	}
	else
	{
		_wnd->move(x, y);
	}
#else
	_wnd->move(x, y);
#endif


	// link menu options (and others) into their correct function calls
	SetupSignals();


	return EGuiStatus::OK;
}



EGuiStatus
UI::GetWindowParameters(
	int& x,
	int& y,
	int& w,
	int& h
) const
{
	if ( _wnd == nullptr )
	{
		return EGuiStatus::NoWindow;
	}

	x = _wnd->x();
	y = _wnd->y();
	w = _wnd->width();
	h = _wnd->height();

	return EGuiStatus::OK;
}



EGuiStatus
UI::LoadConfig(
	const char* path
)
{
#if defined(USING_LIBCONFIG)
	libconfig::Config	cfg;

#	if defined(_WIN32)
	wchar_t		w[MAX_PATH];
	mb_to_utf8(w, path, _countof(w));
	if ( !path_exists(w) )
#	else
#		error nix equivalent
#	endif
	{
		// should never happen if we're using the same as Configuration
		return EGuiStatus::ConfigNotFound;
	}

	try
	{
		cfg.readFile(path);
	}
	catch ( libconfig::FileIOException& e )
	{
		// error reading the file.
		return EGuiStatus::ConfigIOError;
	}
	catch ( libconfig::ParseException& e )
	{
		// error parsing the file.
		return EGuiStatus::ConfigParseError;
	}

	/** @todo put this all into a struct based setup, so we can simply loop
	 * it easily and provide non-spammy, DRY warnings */

	if ( !cfg.lookupValue("ui.main_window.pos_x", (int32_t&)ui.main_window.x) )
	{
		LOG(ELogLevel::Warn) << "No main window x position specified\n";
	}
	if ( !cfg.lookupValue("ui.main_window.pos_y", (int32_t&)ui.main_window.y) )
	{
		LOG(ELogLevel::Warn) << "No main window y position specified\n";
	}
	if ( !cfg.lookupValue("ui.main_window.width", (int32_t&)ui.main_window.width) )
	{
		LOG(ELogLevel::Warn) << "No main window width specified\n";
	}
	if ( !cfg.lookupValue("ui.main_window.height", (int32_t&)ui.main_window.height) )
	{
		LOG(ELogLevel::Warn) << "No main window height specified\n";
	}
	if ( !cfg.lookupValue("ui.main_window.title", ui.main_window.title) )
	{
		// no warning, purely optional customization
	}

#endif

	return EGuiStatus::OK;
}



void
UI::OpenInterfacesLoadDialog() const
{
	InterfacesLoadDialog*	dlg = new InterfacesLoadDialog;

	dlg->SetModel(this);
	dlg->exec();

	delete dlg;
}



void
UI::OpenInterfacesUnloadDialog() const
{
	InterfacesUnloadDialog*	dlg = new InterfacesUnloadDialog;

	dlg->SetModel(this);
	dlg->exec();

	delete dlg;
}



void
UI::OpenModulesLoadDialog() const
{
	ModulesLoadDialog*	dlg = new ModulesLoadDialog;

	dlg->SetModel(this);
	dlg->exec();

	delete dlg;
}



void
UI::OpenModulesUnloadDialog() const
{
	ModulesUnloadDialog*	dlg = new ModulesUnloadDialog;

	dlg->SetModel(this);
	dlg->exec();

	delete dlg;
}



void
UI::Run() const
{
	if ( _app != nullptr )
	{
		_app->exec();
	}
}



EGuiStatus
UI::SetupSignals()
{
	QObject::connect(_base->action_About, SIGNAL(triggered()), this, SLOT(About()));
	QObject::connect(_base->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(AboutQt()));
	QObject::connect(_base->action_InterfaceLoad, SIGNAL(triggered()), this, SLOT(OpenInterfacesLoadDialog()));
	QObject::connect(_base->action_InterfaceUnload, SIGNAL(triggered()), this, SLOT(OpenInterfacesUnloadDialog()));
	QObject::connect(_base->action_ModuleLoad, SIGNAL(triggered()), this, SLOT(OpenModulesLoadDialog()));
	QObject::connect(_base->action_ModuleUnload, SIGNAL(triggered()), this, SLOT(OpenModulesUnloadDialog()));
	QObject::connect(_base->actionE_xit, SIGNAL(triggered()), this, SLOT(Quit()));

	return EGuiStatus::OK;
}



void
UI::Show(
	bool enabled
)
{
	if ( _wnd == nullptr )
	{
		throw std::runtime_error("The window is a nullptr");
	}

	enabled ? _wnd->show() : _wnd->hide();
}



void
UI::Quit() const
{
	// shouldn't be possible to reach here without this, but ah well
	if ( _app != nullptr )
	{
		_app->quit();
	}
}




#endif	// USING_QT5_GUI


/**
 * @file	src/Qt5GUI/UI.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <QtWidgets/qapplication.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qstackedwidget.h>
#include <QtWidgets/qlabel.h>
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

#if defined(__linux__)
#	include <sys/stat.h>		// file ops
#endif

#include <api/definitions.h>
#include <api/version.h>
#include <api/Configuration.h>
#include <api/utils.h>
#include <api/Log.h>
#include <api/Terminal.h>
#include <api/Runtime.h>
#include <api/RpcServer.h>

#include "library.h"			// prototypes and GUI library
#include "UI.h"				// library GUI class
// our dialogs/widgets/windows
#include "generated/ui_UI.h"		// Ui_MainWindow
#include "AboutDialog.h"
#include "InterfacesLoadDialog.h"
#include "InterfacesUnloadDialog.h"
#include "ModulesLoadDialog.h"
#include "ModulesUnloadDialog.h"
// rpc exposure
#include "rpc_commands.h"
#include "RpcWidget.h"
#include "UiThreadExec.h"



BEGIN_NAMESPACE(APP_NAMESPACE)
BEGIN_NAMESPACE(GUI_NAMESPACE)

/* Global pointer to the UI; this is only valid after spawn_interface() has
 * succeeded, and before destroy_interface() has been called. Any attempt to 
 * access it before then will most likely result in a crash.
 * Since we create the UI before any plugin access, only our internal projects
 * need to be careful regarding this. */
std::unique_ptr<UI>	g_ui;

/* Unique ID value used for widgets created by RPC functions. The ID is used so
 * the window can be utilized later, rather than searching for it via some more
 * complex methods. */
std::atomic_uint	g_rpc_widget_id;

END_NAMESPACE
END_NAMESPACE



using namespace APP_NAMESPACE;
using namespace GUI_NAMESPACE;



UI::UI()
{
	// start ids at 1, reserve 0
	g_rpc_widget_id = 1;
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
	QString		title = "Social Bot Interface";
	QString		init_out;
	int32_t		w = ui.main_window.width;
	int32_t		h = ui.main_window.height;
	int32_t		x = ui.main_window.x;
	int32_t		y = ui.main_window.y;
	
#if defined(_WIN32)
	// on Windows, 0 argc & nullptr argv work fine
	int32_t		argc = 0;
	char**		argv = nullptr;
#else
	/* on Linux, 0 argc & nullptr argv will cause strlen segfault (and this is
	 * actually standards-compliant); vars must also always exist, so can't let
	 * them be destroyed out of scope, so make them static - see:
	 * http://www.stackoverflow.com/questions/1519885/defining-own-main-functions-arguments-argc-and-argv */
	static char	argv0[] = "sbi";
	static char* 	argv[] = { &argv0[0], NULL };
	static int32_t	argc = sizeof(argv)/sizeof(argv[0]) - 1;
#endif
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
	// allow RPC clients to call the functions we expose (like CreateWindow)
	PopulateRpcTable();


	return EGuiStatus::OK;
}



EGuiStatus
UI::CreateWindow(
	window_params* wnd_params
)
{
	QWidget*	parent = reinterpret_cast<QWidget*>(wnd_params->parent);
	QWidget*	new_wnd = nullptr;

	/* with the exception of the first and special entry, each window type
	 * is organized alphabetically, just like in ui_windowtype.h.
	 *
	 * While this is nasty, I can't think of any better way to do it right
	 * now, as each type may require special handling; otherwise it'd be
	 * trivial. */

	switch ( wnd_params->window_type )
	{
	case EGuiWindowType::Page:
	{
		RpcQWidget*		page = new RpcQWidget;
		_base->stacked_widget->addWidget(page);
		_base->stacked_widget->setCurrentWidget(page);
		break;
	}
	case EGuiWindowType::ComboBox:
	{
		RpcQComboBox*		combo = new RpcQComboBox;

		new_wnd = combo;
		break;
	}
	case EGuiWindowType::DoubleSpinBox:
	{
		RpcQDoubleSpinBox*	spin = new RpcQDoubleSpinBox;

		new_wnd = spin;
		break;
	}
	case EGuiWindowType::GroupBox:
	{
		RpcQGroupBox*		group = new RpcQGroupBox;

		new_wnd = group;
		break;
	}
	case EGuiWindowType::Label:
	{
		RpcQLabel*		label = new RpcQLabel;

		if ( !wnd_params->text.empty() )
			label->setText(wnd_params->text.c_str());
					    
		label->move(5, 5);

		new_wnd = label;
		break;
	}
	case EGuiWindowType::LineEdit:
	{
		RpcQLineEdit*		line = new RpcQLineEdit;

		new_wnd = line;
		break;
	}
	case EGuiWindowType::ListWidget:
	{
		RpcQListWidget*		list = new RpcQListWidget;

		new_wnd = list;
		break;
	}
	case EGuiWindowType::PushButton:
	{
		RpcQPushButton*		button = new RpcQPushButton;

		new_wnd = button;
		break;
	}
	case EGuiWindowType::RadioButton:
	{
		RpcQRadioButton*	radio = new RpcQRadioButton;

		new_wnd = radio;
		break;
	}
	case EGuiWindowType::SpinBox:
	{
		RpcQSpinBox*		spin = new RpcQSpinBox;

		new_wnd = spin;
		break;
	}
	case EGuiWindowType::StackedWidget:
	{
		RpcQStackedWidget*	stack = new RpcQStackedWidget;

		new_wnd = stack;
		break;
	}
	case EGuiWindowType::TableWidget:
	{
		RpcQTableWidget*	table = new RpcQTableWidget;

		new_wnd = table;
		break;
	}
	case EGuiWindowType::TextBrowser:
	{
		RpcQTextBrowser*	text = new RpcQTextBrowser;

		new_wnd = text;
		break;
	}
	case EGuiWindowType::ToolButton:
	{
		RpcQToolButton*		tool = new RpcQToolButton;

		new_wnd = tool;
		break;
	}
	case EGuiWindowType::TreeWidget:
	{
		RpcQTreeWidget*		tree = new RpcQTreeWidget;
		
		
		new_wnd = tree;
		break;
	}
	case EGuiWindowType::Widget:
	{
		RpcQWidget*		widget = new RpcQWidget;


		new_wnd = widget;
		break;
	}
	default:
		break;
	}


	// prevent code duplication, set common attributes here
	if ( new_wnd != nullptr )
	{
		if ( parent != nullptr )
			new_wnd->setParent(parent);

		new_wnd->show();
	}


	return EGuiStatus::OK;
}



EGuiStatus
UI::GetMainWindowParameters(
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
	struct stat sts;
	if ( stat(path, &sts) == -1 && errno == ENOENT )
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
		std::cerr << fg_red << "Error attempting to read the configuration file '" << path << "'; " << e.what() << "\n";
		return EGuiStatus::ConfigIOError;
	}
	catch ( libconfig::ParseException& e )
	{
		// error parsing the file.
		std::cerr << fg_red << e.getError() << " parsing " << e.getFile() << ":" << e.getLine() << "\n";
		return EGuiStatus::ConfigParseError;
	}

	/** @todo put this all into a struct based setup, so we can simply loop
	 * it easily and provide non-spammy, DRY warnings */

	if ( !cfg.lookupValue("ui.main_window.pos_x", (int32_t&)ui.main_window.x) )
	{
		LOG(ELogLevel::Warn) << "No main window x position specified\n";
		ui.main_window.x = 0;
	}
	if ( !cfg.lookupValue("ui.main_window.pos_y", (int32_t&)ui.main_window.y) )
	{
		LOG(ELogLevel::Warn) << "No main window y position specified\n";
		ui.main_window.y = 0;
	}
	if ( !cfg.lookupValue("ui.main_window.width", (int32_t&)ui.main_window.width) )
	{
		LOG(ELogLevel::Warn) << "No main window width specified\n";
		ui.main_window.width = 768;
	}
	if ( !cfg.lookupValue("ui.main_window.height", (int32_t&)ui.main_window.height) )
	{
		LOG(ELogLevel::Warn) << "No main window height specified\n";
		ui.main_window.height = 1024;
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
UI::PopulateRpcTable()
{
	/* must be static, as the RpcCommand pointers must remain valid in order
	 * to be called and accessed from the RpcTable */
	/** @todo Make RpcCommand additions safer from clients error setups? */
	static const RpcCommand RpcCommands[] =
	{
		{ "gui_create_window", &gui_CreateWindow, RPCF_UNLOCKED },
		{ "gui_destroy_window", &gui_DestroyWindow, RPCF_UNLOCKED },
		{ "gui_get_stack_widget", &gui_GetStackWidget, RPCF_UNLOCKED },
		{ "gui_help", &gui_Help, RPCF_UNLOCKED },
	};

	for ( uint32_t i = 0; i < (sizeof(RpcCommands) / sizeof(RpcCommands[0])); i++ )
	{
		const RpcCommand*	pcmd = &RpcCommands[i];

		runtime.RPC()->GetRpcTable()->AddRpcCommand(pcmd);
	}
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
	// initial interface
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



QStackedWidget*
UI::StackWidget() const
{
	return _base->stacked_widget;
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


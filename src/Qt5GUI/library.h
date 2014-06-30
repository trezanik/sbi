#pragma once

/**
 * @file	library.h
 * @author	James Warren
 * @brief	Qt5GUI library core components for execution
 */



#if 0
#include <QtCore/qobject.h>
#include <QtCore/qsignalmapper.h>
#include <QtGui/qevent.h>
#include <QtGui/qkeysequence.h>
#include <QtGui/qsyntaxhighlighter.h>
#include <QtGui/qpainter.h>
#include <QtWidgets/QApplication.h>
#include <QtWidgets/QLabel.h>
#include <QtWidgets/QMainWindow.h>
#include <QtWidgets/QMenu.h>
#include <QtWidgets/QMenuBar.h>
#include <QtWidgets/qsplitter.h>
#include <QtWidgets/qstackedwidget.h>
#include <QtWidgets/QStatusBar.h>
#include <QtWidgets/qtabbar.h>
#include <QtWidgets/qtabwidget.h>
#include <QtWidgets/QTextEdit.h>
#include <QtWidgets/QTreeView.h>
#include <QtWidgets/QCheckBox.h>
#include <QtWidgets/QMessageBox.h>
#include <QtWidgets/QMdiArea.h>
#include <QtWidgets/QMdiSubWindow.h>
#endif

#include <api/definitions.h>
#include <api/types.h>



// dedicated namespace for the GUI
#define GUI_NAMESPACE		Qt5GUI



#if defined(__cplusplus)
extern "C" {
#endif


// SBI_QT5GUI_API will always be dllexport here (on Windows builds)

/**
 * Destroys the UI previously brought up by spawn_interface(), cleaning up any
 * associated resources. This opportunity is also used to enable save-on-exit
 * functionality (like the main window size + position), before the windows are
 * deleted.
 *
 * One of the three exported functions from the library, which are mandatory in
 * order to be loaded by the executable dynamically.
 *
 * @retval 0 This function always returns 0
 */
SBI_QT5GUI_API
int32_t
destroy_interface();


/**
 * Enters the processing loop for the GUI (like Win32's MessageLoop) - does not
 * return until the UI relinquishes control, usually through closing the window.
 *
 * One of the three exported functions from the library, which are mandatory in
 * order to be loaded by the executable dynamically.
 *
 * @retval 0 This function always returns 0
 */
SBI_QT5GUI_API
int32_t
process_interface();


/**
 * Creates the Root user interface object, the default windows, and shows it.
 *
 * One of the three exported functions from the library, which are mandatory in
 * order to be loaded by the executable dynamically.
 *
 * @retval 0 On successful startup
 * @return If the startup was successful but encountered non-fatal errors, a
 * positive number is returned. On error, a negative number is returned.
 */
SBI_QT5GUI_API
int32_t
spawn_interface();


#if defined(__cplusplus)
}	// extern "C"
#endif

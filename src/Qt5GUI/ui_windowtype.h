#pragma once

/**
 * @file	src/Qt5GUI/ui_windowtype.h
 * @author	James Warren
 * @brief	GUI-specific window types
 */




#include <api/definitions.h>


BEGIN_NAMESPACE(APP_NAMESPACE)
BEGIN_NAMESPACE(GUI_NAMESPACE)



/**
 * User Interface window type enumeration.
 *
 * Classic (pre-C++11) enumeration type as we need to utilize the types 
 * available to us in the RPC libraries.
 *
 * @enum EGuiWindowType
 */
enum EGuiWindowType
{
	Page = 0,		// QWidget for QStackedWidget
	Tree,			// QTreeWidget
	List,			// QListWidget
	Table,			// QTableWidget
	Label,			// QLabel
	Unknown	= UINT_MAX	// Placeholder/default; should never see this reported
};



END_NAMESPACE
END_NAMESPACE

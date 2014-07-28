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
 * available to us in the RPC libraries, so usually restricted to integers.
 *
 * Each entry refers to one of the typedefs in RpcWidget.h; each name, outside
 * of the special cases, is the Qt class name without the Q prefix.
 *
 * @enum EGuiWindowType
 */
enum EGuiWindowType
{
	Page = 0,		// special: QWidget for QStackedWidget
	CheckBox,
	ComboBox,
	DoubleSpinBox,
	GroupBox,
	Label,
	LineEdit,
	ListWidget,
	PushButton,
	RadioButton,
	SpinBox,
	StackedWidget,
	TableWidget,
	TextBrowser,
	ToolButton,
	TreeWidget,
	Widget,			// plain QWidget
	Unknown	= UINT_MAX	// special: Placeholder/default; should never see this
};



END_NAMESPACE
END_NAMESPACE

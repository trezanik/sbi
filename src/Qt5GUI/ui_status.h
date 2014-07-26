#pragma once

/**
 * @file	src/Qt5/ui_status.h
 * @author	James Warren
 * @brief	GUI-specific status codes returned from functions
 */




#include <api/definitions.h>


BEGIN_NAMESPACE(APP_NAMESPACE)
BEGIN_NAMESPACE(GUI_NAMESPACE)



/**
* User Interface status code enumeration.
*
* @sa EIrcStatus
* @enum EGuiStatus
*/
enum class EGuiStatus
{
	OK,
	NoWindow,		// there was no window handle
	ConfigNotFound,		// configuration file not found
	ConfigIOError,		// failed to open/read/write/close the configuration file
	ConfigParseError,	// failed to parse the configuration file
	Unknown			// Placeholder/default; should never see this reported
};



END_NAMESPACE
END_NAMESPACE

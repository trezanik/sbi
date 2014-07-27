#pragma once

/**
 * @file	src/Qt5GUI/rpc_commands.h
 * @author	James Warren
 * @brief	Qt5GUI RPC functions, as they cannot be class members
 */



#if defined(USING_JSON_SPIRIT_RPC)
#	include <json_spirit/json_spirit_utils.h>
#endif

#include <api/definitions.h>



BEGIN_NAMESPACE(APP_NAMESPACE)
BEGIN_NAMESPACE(GUI_NAMESPACE)




/**
 *
 */
SBI_QT5GUI_API
json_spirit::Value
gui_CreateWindow(
	const json_spirit::Array& params,
	bool help
);



/**
 *
 */
SBI_QT5GUI_API
json_spirit::Value
gui_DestroyWindow(
	const json_spirit::Array& params,
	bool help
);



/**
 *
 */
SBI_QT5GUI_API
json_spirit::Value
gui_Help(
	const json_spirit::Array& params,
	bool help
);



END_NAMESPACE
END_NAMESPACE

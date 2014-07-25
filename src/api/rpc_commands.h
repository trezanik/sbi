#pragma once

/**
 * @file	src/api/rpc_commands.h
 * @author	James Warren
 * @brief	API RPC functions, as they cannot be class members
 */



#if defined(USING_JSON_SPIRIT_RPC)
#	include <json_spirit/json_spirit_utils.h>
#endif

#include "definitions.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



/* We break our normal code style naming convention here; since all the actual
 * functionality is within the class function, and these are just wrappers, we
 * will prefix 'api_' to the original member function. 
 * This way which function is to be called is much more visible.
 *
 * Client interfaces are free to do this however they want, but for the sake of
 * ease-of-use, they should have a dedicated namespace and always follow the
 * same convention.
 */



/**
 *
 */
SBI_API
json_spirit::Value
api_GetEnvironmentCoreCount(
	const json_spirit::Array& params,
	bool fHelp
);


/**
 * 
 */
SBI_API
json_spirit::Value
api_Help(
	const json_spirit::Array& params,
	bool fHelp
);


/**
 *
 */
SBI_API
json_spirit::Value
api_Stop(
	const json_spirit::Array& params,
	bool fHelp
);



END_NAMESPACE

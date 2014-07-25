#pragma once

/**
 * @file	src/sbi/app.h
 * @author	James Warren
 * @brief	Application initialization, execution, and cleanup routines
 */



#include <api/types.h>
#include <api/char_helper.h>


/*

(54   ) CORE_API void VARARGS appFailAssert( const ANSICHAR* Expr, const ANSICHAR* File, INT Line );
(149  ) 	Check macros for assertions.
(157  ) 	#define check(expr)  {if(!(expr)) appFailAssert( #expr, __FILE__, __LINE__ );}
(158  ) 	#define verify(expr) {if(!(expr)) appFailAssert( #expr, __FILE__, __LINE__ );}
(168  ) 	#define checkSlow(expr)  {if(!(expr)) appFailAssert( #expr, __FILE__, __LINE__ );}
(169  ) 	#define verifySlow(expr) {if(!(expr)) appFailAssert( #expr, __FILE__, __LINE__ );}
*/


BEGIN_NAMESPACE(APP_NAMESPACE)


/**
 * Executes the main processing loop of the application. Does not return
 * until the application window is closed.
 *
 * Executed only after app_init() has returned, and is wrapped in a
 * dedicated exception handler.
 */
 void
 app_exec();


/**
 * Initializes the application; the command line arguments from main are
 * passed into the function, where they are processed in
 * parse_commandline().
 *
 * Is wrapped in a dedicated exception handler.
 *
 * @param[in] argc The number of arguments within argv
 * @param[in] argv An array of pointers to arguments
 */
void
app_init(
	int32_t argc,
	char** argv
);


/**
 * Cleans up any resources still loaded by the application.
 *
 * Executed only after app_exec() has returned, and is wrapped in a
 * dedicated exception handler.
 */
void
app_stop();


/**
 * @param[in] argc The number of arguments within argv
 * @param[in] argv An array of pointers to arguments
 * @retval false if an invalid option was passed in
 * @retval true if all options were processed, or none to do
 */
bool
parse_commandline(
	int32_t argc,
	char** argv
);


END_NAMESPACE

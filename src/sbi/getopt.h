#pragma once

/**
 * @file	getopt.h
 * @author	James Warren
 * @brief	Win32 version of *nix getopt for parsing command line options
 */



#include <api/char_helper.h>


BEGIN_NAMESPACE(APP_NAMESPACE)


#if defined(_WIN32)

extern int32_t		getopt_ind;	/**< argv index */
extern char*	getopt_arg;	/**< argument pointer */


/**
 * Near-identical in functionality to the normal getopt utility function. As
 * this does not exist on Windows, this is a custom built-one designed to do the
 * same as on a *nix build.
 *
 * @param argc
 * @param argv
 * @param opt
 * @return
 */
int32_t
getopt(
	int32_t argc,
	char** argv,
	char* opt
);

#else

/* linux, unix, have a dedicated command line parser already, so we'll use that
 * one to minimize unforeseen complications. */
#	include <getopt.h>		// posix/gnu getopt (per-system)

#endif	// _WIN32


END_NAMESPACE

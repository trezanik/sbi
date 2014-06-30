#pragma once

/**
 * @file	version.h
 * @author	James Warren
 * @brief	Application and API versions
 */



#include "definitions.h"		// STRINGIFY


// rc file uses these; allow pre-define override for automation tasks
#if !defined(APPLICATION_VERSION_MAJOR)
#	define APPLICATION_VERSION_MAJOR	0
#endif
#if !defined(APPLICATION_VERSION_MINOR)
#	define APPLICATION_VERSION_MINOR	1
#endif
#if !defined(APPLICATION_VERSION_REVISION)
#	define APPLICATION_VERSION_REVISION	1
#endif
#if !defined(APPLICATION_VERSION_BUILD)
#	define APPLICATION_VERSION_BUILD	0
#endif

static const int		APPLICATION_VERSION =
	  1000000 * APPLICATION_VERSION_MAJOR
	+   10000 * APPLICATION_VERSION_MINOR
	+     100 * APPLICATION_VERSION_REVISION
	+       1 * APPLICATION_VERSION_BUILD;

#define APPLICATION_VERSION_STR				\
	    STRINGIFY(APPLICATION_VERSION_MAJOR)	\
	"." STRINGIFY(APPLICATION_VERSION_MINOR)	\
	"." STRINGIFY(APPLICATION_VERSION_REVISION)	\
	"." STRINGIFY(APPLICATION_VERSION_BUILD)

#define APPLICATION_VERSION_DATETIME		" (" __DATE__ ", " __TIME__ ")"



// the API version however, is static and internal; so no overrides permitted.
#if defined(API_VERSION_MAJOR)
#	undef API_VERSION_MAJOR
#endif
#if defined(API_VERSION_MAJOR)
#	undef API_VERSION_MINOR
#endif
#if defined(API_VERSION_REVISION)
#	undef API_VERSION_REVISION
#endif

#define API_VERSION_MAJOR			1
#define API_VERSION_MINOR			0
#define API_VERSION_REVISION			0


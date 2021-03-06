#pragma once

/**
 * @file        src/api/compiler.h
 * @author      James Warren
 * @brief       Supported compiler macros, definitions and functionality
 */



#if defined(_MSC_VER) && defined(__GNUC__)
#	error "Multiple compiler definitions detected (_MSC_VER, __GNUC__)"
#endif
#if defined(_MSC_VER) && defined(__clang__)
#	error "Multiple compiler definitions detected (_MSC_VER, __clang__)"
#endif
// We do NOT check __GNUC__ && __clang__, as clang defines the GNUC macros



/*----------------------------------------------------------------------------
 * clang-specific (Due to GNUC defines, always do clang checks first)
 *---------------------------------------------------------------------------*/

#if defined(__clang__)
#	define IS_CLANG	1
#endif

#if IS_CLANG
#	define CLANG_IS_V2		(__clang_major__ == 3)
#	define CLANG_IS_V3		(__clang_major__ == 3)
#	define CLANG_IS_V2_OR_LATER	(__clang_major__ >= 2)
#	define CLANG_IS_V3_OR_LATER	(__clang_major__ >= 3)

#	define CLANG_VER_IS_OR_LATER_THAN(maj,min) \
	__clang_major__ > maj || \
		(__clang_major__ == maj && (__clang_minor__ > min) || \
			(__clang_minor__ == min))

#	define CLANG_VER_IS_OR_LATER_THAN_PATCH(maj,min,patch) \
	__clang_major__ > maj || \
		(__clang_major__ == maj && (__clang_minor__ > min) || \
			(__clang_minor__ == min && __clang_patchlevel__ > patch))
#endif



/*----------------------------------------------------------------------------
 * GCC-specific
 *---------------------------------------------------------------------------*/

#if defined(__GNUC__) && !defined(__clang__)
#	define IS_GCC	1
#endif

#if IS_GCC

#       define GCC_IS_V2                (__GNUC__ == 2)
#       define GCC_IS_V3                (__GNUC__ == 3)
#       define GCC_IS_V4                (__GNUC__ == 4)
#       define GCC_IS_V2_OR_LATER       (__GNUC__ >= 2)
#       define GCC_IS_V3_OR_LATER       (__GNUC__ >= 3)
#       define GCC_IS_V4_OR_LATER       (__GNUC__ >= 4)

#       define GCC_VER_IS_OR_LATER_THAN(maj,min) \
		__GNUC__ > maj || \
			(__GNUC__ == maj && (__GNUC_MINOR__ > min) || \
				(__GNUC_MINOR__ == min))

#       define GCC_VER_IS_OR_LATER_THAN_PATCH(maj,min,patch) \
		__GNUC__ > maj || \
			(__GNUC__ == maj && (__GNUC_MINOR__ > min) || \
				(__GNUC_MINOR__ == min && __GNUC__PATCHLEVEL__ > patch))

#       if GCC_VER_IS_OR_LATER_THAN(4,6)
//#             pragma message "GCC version 4.6 or later detected; using nullptr"
#       else
#               error nullptr does not exist until GCC version 4.6.0 - update the compiler or provide a workaround
#       endif
#endif  // IS_GCC



/*----------------------------------------------------------------------------
 * Visual Studio-specific
 *---------------------------------------------------------------------------*/

#if defined(_MSC_VER)
#       define IS_VISUAL_STUDIO 1
#endif

#if IS_VISUAL_STUDIO
#       define MS_VISUAL_CPP_VS8        1400    /**< Visual Studio 2005 */
#       define MS_VISUAL_CPP_VS9        1500    /**< Visual Studio 2008 */
#       define MS_VISUAL_CPP_VS10       1600    /**< Visual Studio 2010 */
#       define MS_VISUAL_CPP_VS11       1700    /**< Visual Studio 2012 */
#       define MS_VISUAL_CPP_VS12       1800    /**< Visual Studio 2013 */

#       define MSVC_IS_VS8              (_MSC_VER == MS_VISUAL_CPP_VS8)         /**< Is Visual Studio 2005 */
#       define MSVC_IS_VS9              (_MSC_VER == MS_VISUAL_CPP_VS9)         /**< Is Visual Studio 2008 */
#       define MSVC_IS_VS10             (_MSC_VER == MS_VISUAL_CPP_VS10)        /**< Is Visual Studio 2010 */
#       define MSVC_IS_VS11             (_MSC_VER == MS_VISUAL_CPP_VS11)        /**< Is Visual Studio 2012 */
#       define MSVC_IS_VS12             (_MSC_VER == MS_VISUAL_CPP_VS12)        /**< Is Visual Studio 2013 */
#       define MSVC_IS_VS8_OR_LATER     (_MSC_VER >= MS_VISUAL_CPP_VS8)         /**< Is or later than Visual Studio 2005 */
#       define MSVC_IS_VS9_OR_LATER     (_MSC_VER >= MS_VISUAL_CPP_VS9)         /**< Is or later than Visual Studio 2008 */
#       define MSVC_IS_VS10_OR_LATER    (_MSC_VER >= MS_VISUAL_CPP_VS10)        /**< Is or later than Visual Studio 2010 */
#       define MSVC_IS_VS11_OR_LATER    (_MSC_VER >= MS_VISUAL_CPP_VS11)        /**< Is or later than Visual Studio 2012 */
#       define MSVC_IS_VS12_OR_LATER    (_MSC_VER >= MS_VISUAL_CPP_VS12)        /**< Is or later than Visual Studio 2013 */
#       define MSVC_BEFORE_VS8          (_MSC_VER < MS_VISUAL_CPP_VS8)          /**< Is Pre-Visual Studio 2005 */
#       define MSVC_BEFORE_VS9          (_MSC_VER < MS_VISUAL_CPP_VS9)          /**< Is Pre-Visual Studio 2008 */
#       define MSVC_BEFORE_VS10         (_MSC_VER < MS_VISUAL_CPP_VS10)         /**< Is Pre-Visual Studio 2010 */
#       define MSVC_BEFORE_VS11         (_MSC_VER < MS_VISUAL_CPP_VS11)         /**< Is Pre-Visual Studio 2012 */
#       define MSVC_BEFORE_VS12         (_MSC_VER < MS_VISUAL_CPP_VS12)         /**< Is Pre-Visual Studio 2013 */
#       define MSVC_LATER_THAN_VS8      (_MSC_VER > MS_VISUAL_CPP_VS8)          /**< Is Post-Visual Studio 2005 */
#       define MSVC_LATER_THAN_VS9      (_MSC_VER > MS_VISUAL_CPP_VS9)          /**< Is Post-Visual Studio 2008 */
#       define MSVC_LATER_THAN_VS10     (_MSC_VER > MS_VISUAL_CPP_VS10)         /**< Is Post-Visual Studio 2010 */
#       define MSVC_LATER_THAN_VS11     (_MSC_VER > MS_VISUAL_CPP_VS11)         /**< Is Post-Visual Studio 2012 */
#       define MSVC_LATER_THAN_VS12     (_MSC_VER > MS_VISUAL_CPP_VS12)         /**< Is Post-Visual Studio 2013 */


#       if MSVC_BEFORE_VS10
		/* VS2008 and earlier do not have nullptr. Be warned that many
		 * features we're starting to use are not supported before now.. */
#               define nullptr  NULL
		/* We also add the Windows versions that did not exist at the
		 * time VS2008 was released (see sdkddkver.h) */
#               define _WIN32_WINNT_WIN6        0x0600
#               define _WIN32_WINNT_VISTA       0x0600
#               define _WIN32_WINNT_W08         0x0600
#               define _WIN32_WINNT_LONGHORN    0x0600
#               define _WIN32_WINNT_WIN7        0x0601
#               define _WIN32_WINNT_WIN8        0x0602
#               define _WIN32_WINNT_WINBLUE     0x0603
#       endif
#	if MSVC_BEFORE_VS12
		/* VS2012 and onwards support C++11 static_assert; use some form
		 * of error for the older compilers:
		 * http://blogs.msdn.com/b/abhinaba/archive/2008/10/27/c-c-compile-time-asserts.aspx
		 */
#		if defined(USING_BOOST)
#			include <boost/static_assert.hpp>
#			define static_assert	BOOST_STATIC_ASSERT_MSG
#		else
#			error Complete me
#		endif
#	endif
#endif  // IS_VISUAL_STUDIO





/* Debug-build detection; we try our best using the common idioms for using a
 * debug/release build, and use that.
 *
 * Internally, we define DEBUG_BUILD to 1 if in debug mode, otherwise, no
 * definition is provided. This allows #if DEBUG_BUILD or #ifdef DEBUG_BUILD to
 * work, and easier identification of the setup; you're in release mode if you
 * cannot find this.
 */
#if defined(IS_DEBUG_BUILD)
	// our definition; overrides all others
#       warning "Using IS_DEBUG_BUILD override; it is safer to define/undefine NDEBUG as appropriate"
#elif defined(NDEBUG) || defined(_NDEBUG)
	// something saying we're in NON-DEBUG mode; check for conflicts
#       if defined(_DEBUG) || defined(DEBUG) || defined(__DEBUG__)
#               error "Conflicting definitions for Debug/Release mode"
#       endif
	// no conflict - we're in release mode.
//#     pragma message "Building in Release mode."
#else
	// release mode not specified - check if DEBUG mode specified
#       if defined(_DEBUG) || defined(DEBUG) || defined(__DEBUG__)
		// yup; debug mode is definitely desired.
#               define IS_DEBUG_BUILD   1
//#             pragma message "Building in Debug mode."
#       else
		/* debug mode also not specified; do it, but warn the user */
#               warning "No Debug/Release definition found; assuming Release"
//#             pragma message "Building in Release mode."
#       endif
#endif  // DEBUG_BUILD

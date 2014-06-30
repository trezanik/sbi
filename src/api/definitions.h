#pragma once

/**
 * @file	definitions.h
 * @author	James Warren
 * @brief	Common definitions and helper macros
 */



/** Prevents the class from being assigned */
#define NO_CLASS_ASSIGNMENT(class_name)		private: class_name operator = (class_name const&)

/** Prevents the class from being copied */
#define NO_CLASS_COPY(class_name)		private: class_name(class_name const&)

/** Calls placement new on object as of class type. Convenience macro to ensure
 * no extra/misplaced parameters are present */
#define CONSTRUCT(object, type)			object = new (object) type


/** Compile-time assertion checks (requires C++11) */
#define COMPILE_TIME_CHECK(expression, message)		static_assert(expression, message)

/** Generic buffer size for random use */
#define MAX_LEN_GENERIC		250


// MSVC pragmas - used so other platforms will have no affect
#ifndef MSVC_PRAGMA
#	define MSVC_PRAGMA(...)
#endif


/** Stops Visual Studio indenting namespaces, and enables us to change the
 * namespace name from this one file rather than doing a potentially incorrect
 * find + replace. Also removes multiple curly-bracket spammage at times, that
 * could be misconstrued as being part of a function. */
#define BEGIN_NAMESPACE(name)	namespace name {
#define END_NAMESPACE		}
#define APP_NAMESPACE		SBI



// One-liner platform independent sleep
#if defined(_WIN32)
#	define SLEEP_SECONDS(secs)	Sleep(secs * 1000)
#	define SLEEP_MILLISECONDS(ms)	Sleep(ms)
#else
#	define SLEEP_SECONDS(secs)	sleep(secs)
#	define SLEEP_MILLISECONDS(ms)	sleep(ms * 1000)
#endif


// Raw text literal as supplied
#define STRINGIFY(str)	REAL_TEXT(str)
#define REAL_TEXT(str)	#str


// Path separator character
#if defined(_WIN32)
#	define PATH_CHAR	'\\'
#	define LINE_END		"\r\n"
#else
#	define PATH_CHAR	'/'
#	define LINE_END		"\n"
#endif


// for the compilers that don't have __func__ but do have __FUNCTION__
#ifndef __func__
#	define __func__	__FUNCTION__
#endif


#if IS_GCC
#	define DEPRECATED_FUNCTION(func)	func __attribute__ ((deprecated))
#elif IS_VISUAL_STUDIO
#	define DEPRECATED_FUNCTION(func)	__declspec(deprecated) func
#else
#	define DEPRECATED_FUNCTION(func)	func
#endif



#if IS_VISUAL_STUDIO || defined(_WIN32)

#	pragma warning ( disable : 4251 )	// needs to have dll-interface

	// Windows standards-compliance functions
#	define unlink			_unlink

	// Library : API
#	if defined(API_EXPORTS)
#		define SBI_API		__declspec(dllexport)
#	else
#		define SBI_API		__declspec(dllimport)
#	endif
	// Library : IRC
#	if defined(IRC_EXPORTS)
#		define SBI_IRC_API	__declspec(dllexport)
#	else
#		define SBI_IRC_API	__declspec(dllimport)
#	endif
	// Library : Reddit
#	if defined(REDDIT_EXPORTS)
#		define SBI_REDDIT_API	__declspec(dllexport)
#	else
#		define SBI_REDDIT_API	__declspec(dllimport)
#	endif
	// Library : Twitter
#	if defined(TWITTER_EXPORTS)
#		define SBI_TWITTER_API	__declspec(dllexport)
#	else
#		define SBI_TWITTER_API	__declspec(dllimport)
#	endif
	// Library : Qt5 GUI
#	if defined(QT5GUI_EXPORTS)
#		define SBI_QT5GUI_API	__declspec(dllexport)
#	else
#		define SBI_QT5GUI_API	__declspec(dllimport)
#	endif

#	define SBI_ALWAYS_EXPORT	__declspec(dllexport)
#	define SBI_ALWAYS_IMPORT	__declspec(dllimport)

#else
#	define SBI_API
#	define SBI_IRC_API
#	define SBI_TWITTER_API
#	define SBI_QT5GUI_API
#endif

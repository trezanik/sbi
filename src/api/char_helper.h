#pragma once

/**
* @file        char_helper.h
* @author      James Warren
* @brief       Cross-platform string conversion helper (mostly for Windows)
*/



#include <sstream>		// so we can create a typedef for stringstream
#include <string>		// so we can create a typedef for string
#include "compiler.h"
#include "definitions.h"
#include "types.h"



BEGIN_NAMESPACE(APP_NAMESPACE)
BEGIN_NAMESPACE(PlatformType)


/* to prevent preprocessor spammage within structs and classes, we have our
 * own datatype for the common character types and strings. These bear no
 * relevance to source files, which still need to be preprocessor'd, so if
 * these show in a source file it is considered an error. */
#if defined(_WIN32)
#	if IS_VISUAL_STUDIO
#		if defined(_WCHAR_T_DEFINED)
			typedef wchar_t			CHARTYPE;
			typedef std::wstringstream	CHARSTREAMTYPE;
			typedef std::wstring		CHARSTRINGTYPE;
#			define RESET_STREAM(s)		s.str(L"")
#		else
			typedef char			CHARTYPE;
			typedef std::stringstream	CHARSTREAMTYPE;
			typedef std::string		CHARSTRINGTYPE;
#			define RESET_STREAM(s)		s.str("")
#		endif
#	else
		typedef char			CHARTYPE;
		typedef std::stringstream	CHARSTREAMTYPE;
		typedef std::string		CHARSTRINGTYPE;
#		define RESET_STREAM(s)		s.str("")
#	endif
#else
    // I assume Android + Linux are happy with the same data types??
    typedef char			CHARTYPE;
    typedef std::stringstream	CHARSTREAMTYPE;
    typedef std::string		CHARSTRINGTYPE;
#	define RESET_STREAM(s)		s.str("")
#endif

#if 0
// about to set this definition; if present, remove it
#if defined(char_TEXT)
#	undef char_TEXT
#endif

#if defined(_WIN32)
#	if IS_VISUAL_STUDIO
#		if defined(_WCHAR_T_DEFINED)
			typedef wchar_t			CHARTYPE;
			typedef std::wstringstream	CHARSTREAMTYPE;
			typedef std::wstring		CHARSTRINGTYPE;
#			define char_COUNT(str)	_countof(str)
#			define str)	L##str

			/* override 'standard' functions, so we don't have to
			 * modify non-Windows builds or spam preprocessors */
#			define strchr			wcschr
#			define strrchr			wcsrchr
#			define strcmp			wcscmp
#			define strncmp			wcsncmp
#			define strlen			wcslen
#			define strstr			wcsstr
#			define printf			wprintf
#			define fprintf			fwprintf
#			define vsnprintf		_vsnwprintf
#			define _fsopen			_wfsopen
			/** @todo add all the standard functions here */
#		else
			typedef char			CHARTYPE;
			typedef std::stringstream	CHARSTREAMTYPE;
			typedef std::string		CHARSTRINGTYPE;
#			define char_COUNT(str)	sizeof(str)
#			define str)	str
#			define char_STRINGIFY(str)	str)
#		endif
#		define CHARTYPE_SIZE		sizeof(CHARTYPE)
#	endif
#else
	// I assume Android + Linux are happy with the same data types??
	typedef char			char;
	typedef std::stringstream	CHARSTREAMTYPE;
	typedef std::string		CHARSTRINGTYPE;
#	define char_COUNT(str)	sizeof(str)
#	define str)	str
#	define char_SIZE		sizeof(char)
#endif
#endif


#if defined(VERIFY_STATIC_ASSERTS)
/*-----------------------------------------------------------------------------
 * Ensure data types are expected sizes
 *----------------------------------------------------------------------------*/

COMPILE_TIME_CHECK(sizeof(uint8_t) == 1, "sizeof(uint8_t) should be 1 byte");
COMPILE_TIME_CHECK(int32_t(uint8_t(-1)) == 0xFF, "(uint8_t - 1) should be 0xFF");

COMPILE_TIME_CHECK(sizeof(uint16_t) == 2, "sizeof(uint16_t) should be 2 bytes");
COMPILE_TIME_CHECK(int32_t(uint16_t(-1)) == 0xFFFF, "(uint16_t - 1) should be 0xFFFF");

COMPILE_TIME_CHECK(sizeof(uint32_t) == 4, "sizeof(uint32_t) should be 4 bytes");
COMPILE_TIME_CHECK(int32_t(uint32_t(-1)) == 0xFFFFFFFF, "(uint32_t - 1) should be 0xFFFFFFFF");

#if defined(_WIN64) || defined(__x86_64__)
	COMPILE_TIME_CHECK(sizeof(uintptr_t) == 8, "Expected a 64-bit platform to have a pointer size of 8 bytes");
#else
	COMPILE_TIME_CHECK(sizeof(uintptr_t) == 4, "Expected a 32-bit platform to have a pointer size of 4 bytes");
#endif
COMPILE_TIME_CHECK(sizeof(void*) == sizeof(uintptr_t), "A pointer should be the same size as a uintptr_t");


COMPILE_TIME_CHECK(sizeof(char) == 1, "sizeof(char) should be 1 byte");

COMPILE_TIME_CHECK(char(-1) < char(0), "Default char should be signed");


#if defined(_WCHAR_T_SIZE)
#	if defined(_WIN32)
		// Win32 wchar_t = 2 bytes
		COMPILE_TIME_CHECK(sizeof(uint16_t) == _WCHAR_T_SIZE, "sizeof(wchar_t) should be the size of uint16_t");
		COMPILE_TIME_CHECK(WCHAR_MIN == 0x0000, "wchar_t minimum value should be 0x0000");
		COMPILE_TIME_CHECK(WCHAR_MAX == 0xffff, "wchar_t maximum value should be 0xffff");
#	else
		// Linux/Unix/Mac OSX wchar_t = 4 bytes
		COMPILE_TIME_CHECK(sizeof(uint32_t) == _WCHAR_T_SIZE, "sizeof(wchar_t) should be the size of uint32_t");
#	endif
#endif
#endif	// VERIFY_STATIC_ASSERTS


/*-----------------------------------------------------------------------------
 * secure our string handling
 *----------------------------------------------------------------------------*/
#if defined(USING_API_WARNINGS)
// force a compile-time check to fail unspecifically
#	define GUARANTEE_FAILURE			(0 == 1)
#	define DISABLED_FUNCTIONS_MESSAGE_TIME		"C-style time functions must be replaced with their secure counterparts (localtime_s|localtime_r)"
#	define DISABLED_FUNCTIONS_MESSAGE_CSTRING	"strcpy, strcat must be replaced with strlcpy and strlcat, respectively"
//#	define ctime		COMPILE_TIME_CHECK(GUARANTEE_FAILURE, DISABLED_FUNCTIONS_MESSAGE_TIME);
//#	define localtime	COMPILE_TIME_CHECK(GUARANTEE_FAILURE, DISABLED_FUNCTIONS_MESSAGE_TIME);
#	define strcpy		COMPILE_TIME_CHECK(GUARANTEE_FAILURE, DISABLED_FUNCTIONS_MESSAGE_CSTRING);
#	define strcat		COMPILE_TIME_CHECK(GUARANTEE_FAILURE, DISABLED_FUNCTIONS_MESSAGE_CSTRING);
#endif	// USING_API_WARNINGS


END_NAMESPACE // PlatformType
END_NAMESPACE // APP_NAMESPACE

using namespace APP_NAMESPACE::PlatformType;

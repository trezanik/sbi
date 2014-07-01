#pragma once

/**
 * @file        types.h
 * @author      James Warren
 * @brief       Defines the platform data types for the application
 */



#include "definitions.h"


// Visual Studio 2010+ started to use standards, like <cstdint>
#if MSVC_BEFORE_VS10
	/* This is just a copy of the VS2012 stdint.h, with a definition
	 * modification to prevent a warning, as it's mixed with the older
	 * headers in VS2008 (lines 120, 123). */
#       include "c99/stdint.h"
#else
#       include <cstdint>
#endif


/** @todo trying to use Windows code with these disabled types causes 
 * incredible grief! See if we can get this working side-by-side easily */
#if !defined(_WIN32)
	// Disable TRUE/FALSE macros
#	ifdef TRUE
#		undef TRUE
#	endif
#	ifdef FALSE
#		undef FALSE
#	endif


// Trick to prevent duplicated types (mostly as a result of Windows' headers)
namespace ErrorConflictingType
{
	/* Used to cause compile errors through typedefs of unportable types. If unportable types 
	are really necessary please use the global scope operator to access it (i.e. ::&nbsp;INT). 
	Windows header file includes can be wrapped in #include "AllowWindowsPlatformTypes.h" 
	and #include "HideWindowsPlatformTypes.h" */
	class DisabledType
	{
		DisabledType();
		~DisabledType();
	};

	
	// Disabled type - use int32_t instead
	typedef DisabledType INT;

	// Disabled type - use uint32_t instead
	typedef DisabledType UINT;
	
	// Disabled type - use uint32_t instead
	typedef DisabledType DWORD;

	// Disabled type - use float instead
	typedef DisabledType FLOAT;

	// Disabled type - use true instead
	typedef DisabledType TRUE;

	// Disabled type - use false instead
	typedef DisabledType FALSE;
}

using namespace ErrorConflictingType;

#endif	// _WIN32

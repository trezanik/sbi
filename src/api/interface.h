#pragma once

/**
 * @file	interface.h
 * @author	James Warren
 * @brief	Exported functions for interfaces
 * @note
 * We must use 'int' instead of EInterfaceStatus to get round compilation
 * failures. They're fully interchangeable, and if a fix is possible, we will
 * replace the code where applicable.
 * Assume the int return values are EInterfaceStatus wherever feasible.
 */



#include "interface_status.h"	// used by those including this, and ready for fix
#include "definitions.h"



#if defined(__cplusplus)
extern "C" {
#endif




/**
 * Cleans up any resources created by the interface. Required to be called when
 * the module unloads.
 *
 * @retval EInterfaceStatus::Ok on initialization success
 * @retval ...
 */
SBI_ALWAYS_EXPORT
int
#if defined(_WIN32)
__stdcall
#endif
destroy_interface();



/**
 * Returns a pointer to the interface instance. Return type must be void as the
 * api/runtime cannot know what object the instance will be, since it's compiled
 * with no knowledge of the interface.
 *
 * @warning
 * Assumes spawn_interface() has been called before now as part of the loading
 * procedure, which creates the object this refers to. Third-party interfaces
 * can do this however they want, but is useful to know what to expect.
 */
SBI_ALWAYS_EXPORT
void*
#if defined(_WIN32)
__stdcall
#endif
instance(
	void* params
);



/**
 * Creates the instance within the library. The interface should use this
 * opportunity to set everything up internally, including creating any threads
 * needed to retain execution.
 *
 * @note
 * Win32's DllMain can be used in place of this (and destroy_interface()), but
 * since we want to support all *nixes too, these are the approved + supported
 * methods of initialization and destruction.
 * If you really want to do any handling in DllMain, feel free - you're not
 * limited to the functions you can call (since the app has loaded all the DLLs
 * required before now, so there'll be no deadlock risk).
 *
 * @retval EInterfaceStatus::Ok on initialization success
 * @retval ...
 */
SBI_ALWAYS_EXPORT
int
#if defined(_WIN32)
__stdcall
#endif
spawn_interface();



#if defined(__cplusplus)
}	// extern "C"
#endif

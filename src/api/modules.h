#pragma once

/**
 * @file	modules.h
 * @author	James Warren
 * @brief	Plugins/modules components
 */



#include "definitions.h"



BEGIN_NAMESPACE(APP_NAMESPACE);


/**
 * When loading a module, one of these values will be returned. This applies to
 * all GUI and plugin/interface modules.
 */
enum class EModuleLoadFailure
{
	NoError = 0,
	FileNotFound,
	FileIncompatible,
	CouldNotBeLoadedByOS,
	FailedToInitialize
};



/**
 * Primarily used to determine if a module was built in debug/release mode,
 * but also handy for if we're doing testing, we can skip loading huge portions
 * of code or even entire libraries.
 */
enum class EBuildConfiguration
{
	Debug = 0,	/**< Application debugging */
	Release,	/**< Application public release */
	Test,		/**< Unit/normal testing */
	Unknown		/**< Unknown/unrecognized */
};



END_NAMESPACE

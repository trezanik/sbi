#pragma once

/**
 * @file	interfaces.h
 * @author	James Warren
 * @brief	Interface operational functions
 */



#include <vector>
#include <memory>
#include <string>

#include "interface_status.h"
#include "definitions.h"
#include "types.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



// as noted in the interface header, return value is actually EInterfaceStatus
typedef int32_t	(__stdcall *fp_interface)();
typedef void*	(__stdcall *fp_instance)(void*);



/**
* Passed to the GUI on request for available interfaces.
*
* The library is loaded into memory, and retained until the vector is deleted,
* or moved as a result of the user requesting loading of the interface.
*/
struct AvailableInterfaceDetails
{
	// library file name (no directory)
	std::string	file_name;
	// pointer to the loaded library
	void*		library_handle;
	// pointer to the destroy_interface exported function
	fp_interface	pf_destroy_interface;
	// pointer to the instance exported function
	fp_instance	pf_instance;
	// pointer to the spawn_interface exported function
	fp_interface	pf_spawn_interface;

	// memory maintenance, unload libraries in destructor
	AvailableInterfaceDetails();
	~AvailableInterfaceDetails();
};



struct AvailableModuleDetails
{
};



SBI_API
std::vector<std::shared_ptr<AvailableInterfaceDetails>>
get_available_interfaces();



SBI_API
std::vector<std::shared_ptr<AvailableModuleDetails>>
get_available_modules();



END_NAMESPACE

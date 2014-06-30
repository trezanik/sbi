#pragma once

/**
 * @file	interfaces.h
 * @author	James Warren
 * @brief	Interface operational functions
 */



#include <vector>
#include <string>
#include "interface_status.h"
#include "definitions.h"



BEGIN_NAMESPACE(APP_NAMESPACE)


SBI_API
std::vector<std::string>
get_available_interfaces();



SBI_API
std::vector<std::string>
get_available_modules();



END_NAMESPACE

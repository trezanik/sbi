#pragma once

/**
 * @file	src/api/interface_status.h
 * @author	James Warren
 * @brief	Interface library status codes
 */



enum class EInterfaceStatus
{
	Ok = 0,
	FileNotFound,
	LoadFailure,
	InsufficientResources,
	Unknown = -1
};

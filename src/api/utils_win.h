#pragma once

/**
 * @file	src/api/utils_win.h
 * @author	James Warren
 * @brief	Windows-specific utility functions
 */



#include <vector>
#include "char_helper.h"


BEGIN_NAMESPACE(APP_NAMESPACE)



/** Holds version information for a file; to be used with binaries */
struct file_version_info
{
	/*Comments	InternalName	ProductName
	CompanyName	LegalCopyright	ProductVersion
	FileDescription	LegalTrademarks	PrivateBuild
	FileVersion	OriginalFilename	SpecialBuild*/

	// Module version (e.g. 6.1.7201.17932)
	uint16_t	major;
	uint16_t	minor;
	uint16_t	revision;
	uint16_t	build;
	// File description
	wchar_t		description[1024];
};


/** Holds information about a module (DLL) */
struct ModuleInformation
{
	wchar_t			name[512];	/**< Module name (full path) */
	file_version_info	fvi;
};




/**
 *
 */
SBI_API
bool
path_exists(
	const wchar_t* path
);



/**
 * 
 */
SBI_API
wchar_t*
error_code_as_string(
	uint64_t code
);



/**
 * Retrieves the current path for the executing binary, storing the result in
 * the supplied @a buffer. Comes with the trailing backward-slash.
 *
 * @param[in,out] buffer The preallocated buffer to store the result in
 * @param[in] buffer_size The size of the preallocated buffer
 * @return Returns the amount of characters written to buffer if successful
 * @return On failure, 0 is returned
 */
SBI_API
uint32_t
get_current_binary_path(
	wchar_t* buffer,
	uint32_t buffer_size
);



/**
* Obtains the version information for the specified file.
*
* A file_version_info struct is passed in, which will contain the results of
* the data acquisition. It is immediately reset to 0 or blank values, in the
* event of failure.
* ntdll on Win7 SP1 x64 would be:
* - major = 6
* - minor = 1
* - rev = 7601
* - build = 18229
*
* @retval true if the acquisition succeeds, and the struct populated
* @retval false if the acquisition fails
*/
SBI_API
bool
get_file_version_info(
	wchar_t* path,
	file_version_info* fvi
);



/**
 *
 */
SBI_API
void*
get_function_address(
	const char* func_name,
	const wchar_t* module_name
);



/**
 * Enumerates all the loaded modules in the running process, and logs them to
 * file. Does not check if the config has logging enabled; the caller should do
 * so.
 *
 * @warning
 * Each ModuleInformation* returned needs its memory freeing when it is no
 * longer needed; failure to do so will result in a memory leak.
 *
 * @return A vector of structs containing information, such as the name and
 * address, of each loaded module.
 */
SBI_API
std::vector<ModuleInformation*>
get_loaded_modules();



/* after much investigation and research, for the purposes of cross-platform
 * compatibility, least maintenance and least issues, this is what's decided:
 * 1) Use char everywhere. Convert on the fly as needed:
 *	#if _WIN32
 *	wchar_t		buf[x];
 *	to_utf8(mb_string, buf, countof(buf));
 *	ApiFunctionW(buf);
 *	#endif
 * 2) Use CHARTYPE typedef for headers, to save preprocessor spammage.
 *	CHARTYPE should not appear in source files
 * 3) Dedicate source files using a particular type where appropriate.
 *	mb_to_utf8(), for example, should reside in utils_win
 *	host_to_ipv4 does not cater to this, so is preprocessor'd in source
 */


/**
 * Uses MultiByteToWideChar to convert the input multi-byte string into UTF8
 *
 * @param[out] dest The destination buffer
 * @param[in] dest The string to copy
 * @param[in] dest The size of the destination buffer, in characters
 * @return true is returned if the conversion occurs without errors, otherwise
 * returns false.
 */
SBI_API
bool
mb_to_utf8(
	wchar_t* dest,
	const char* src,
	const uint32_t dest_size
);



/**
 * Windows-specific thread rename code. Dedicated utility function due to the
 * amount of code required.
 *
 * @sa rename_thread
 */
void
set_thread_name(
	uint32_t thread_id,
	const char* name
);



#if 0	// Consider this for future; zero priority at the moment
SBI_API
bool
to_utf16(
wchar_t* dest,
const char* src,
const uint32_t dest_size
);



SBI_API
bool
to_utf32(
wchar_t* dest,
const char* src,
const uint32_t dest_size
);
#endif



/**
 * An exact duplicate of str_format, but using wchar_t instead.
 *
 * @sa str_format
 */
SBI_API
uint32_t
wcs_format(
	wchar_t* dest,
	uint32_t dest_size,
	wchar_t* format,
	...
);



/**
 * Uses WideCharToMultiByte to convert the input wide-character string into a
 * multi-byte one
 *
 * @param[out] dest The destination buffer
 * @param[in] dest The string to copy
 * @param[in] dest The size of the destination buffer, in characters
 * @return true is returned if the conversion occurs without errors, otherwise
 * returns false.
 */
SBI_API
bool
wide_to_mb(
	char* dest,
	const wchar_t* src,
	const uint32_t dest_size
);





END_NAMESPACE

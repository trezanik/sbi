#pragma once

/**
 * @file        utils.h
 * @author      James Warren
 * @brief       Consistent secure string & utility functionality, multi-platform
 */



#if !defined(_WIN32)
#	define __STDC_FORMAT_MACROS
#	include <cinttypes>		// PRIxPTR, for PRINT_POINTER
#endif

#include "char_helper.h"
#include "definitions.h"
#include "types.h"


#if defined(_WIN32)
#	include "utils_win.h"		// expose Windows utility functions
#elif defined(__linux__)
#	include "utils_linux.h"		// expose Linux utility functions
#else
#	warning "No utility functions available for the target Operating System"
#endif



BEGIN_NAMESPACE(APP_NAMESPACE)


// allows grabbing number of args passed in (build_string) so doesn't need doing manually
#define BUILD_STRING(...)			build_string(NUM_VA_ARGS_(,##__VA_ARGS__,8,7,6,5,4,3,2,1,0), __VA_ARGS__)
#define NUM_VA_ARGS_(z,a,b,c,d,e,f,g,h,cnt,...)	cnt


/* don't ask. I did this a while ago and had issues (I believe it was with
 * visual studio, as usual), this is what I came up with for a workaround, and
 * it works, so... */
#if defined(_WIN64)
#	define PRINT_POINTER	"%016p"
#elif defined(_WIN32)
#	define PRINT_POINTER	"%08p"
#elif defined(__x86_64__)
#	define PRINT_POINTER	"%016" PRIxPTR " "
#else
#	define PRINT_POINTER	"%08" PRIxPTR
#endif




/**
 * Creates a std::string from any supplied parameters.
 *
 * Much easier than putting a bunch of individual line appends or having to
 * stream it; works with input of std::string, char*, etc. - anything a normal
 * std::string assignment is capable of.
 *
 * Originally created so we could have a one-liner for std::runtime_error.
 *
 * @warning
 * Use the BUILD_STRING macro; this handles the automatic argument count, so
 * changes won't result in the counter being incorrect
 *
 * @retval std::string
 */
SBI_API
std::string
build_string(
	int16_t num_args,
	...
);



#ifdef __cplusplus
extern "C" {
#endif




/**
 * Generates a random string, with a minimum/maximum length as supplied. If the
 * @a seed is not supplied, @a rand() will not be seeded, using any existing
 * value assigned to it.
 *
 * The random string will only contain a-z and A-Z characters.
 *
 * The memory returned by the function must be freed with @a MemFree.
 *
 * @param min_chars The minimum number of characters the string will be. Must
 * be greater than 0.
 * @param max_chars The maximum number of characters the string will be. Must
 * be equal to or more than @a min_chars, and less than 250.
 * @param seed The seed value to pass to the srand library function, or 0 for
 * no seed.
 * @return Returns NULL if the above criteria are not met, or memory allocation
 * fails.
 * @return Otherwise, a pointer to the dynamically allocated random string will
 * be returned.
 */
SBI_API
char*
gen_random_string(
	uint32_t min_chars,
	uint32_t max_chars,
	uint32_t seed = 0
);



/**
 * Gets the current time in milliseconds.
 *
 * Used for timing operations - store the first call to this as a start_time,
 * and then use the second call to determine the duration.
 *
 * @return Returns the number of milliseconds since January 1, 1601 (UTC).
 */
SBI_API
uint64_t
get_ms_time();



SBI_API
char*
skip_whitespace(
	char* str
);



/**
 * Appends @a src into the buffer specified by @a dest, up to a limit of @a size
 * - 1.
 * nul termination is guaranteed if @a size is at least 1 - unlike the OpenBSD
 * version of strlcat, if no nul is found in @a dest, it is inserted at the
 * final position. This is a change for tiny safety sake.
 *
 * Is an exact match of the OpenBSD strlcat.
 *
 * @param dest The destination buffer
 * @param src The string to append
 * @param dest_size The size of the destination buffer
 * @return The length of the string that was attempted to be created. So, strlen
 * of @a dest + strlen of @a src. If this is greater than or equal to @a size,
 * truncation has occurred, and should be handled by the caller.
 */
SBI_API
uint32_t
strlcat(
	char* dest,
	const char* src,
	uint32_t dest_size
);



/**
 * Copies @a src into the buffer specified by @a dest, up to a limit of @a size
 * - 1. Always starts copying @a src and overwrites anything previously there.
 * nul termination is guaranteed if @a size is at least 1.
 *
 * Is an exact match of the OpenBSD strlcpy.
 *
 * @param dest The destination buffer
 * @param src The string to copy
 * @param dest_size The size of the destination buffer
 * @return The length of the string that was attempted to be created. So, strlen
 * of @a src. If this is greater than or equal to @a size, truncation has
 * occurred, and should be handled by the caller.
 */
SBI_API
uint32_t
strlcpy(
	char* dest,
	const char* src,
	uint32_t dest_size
);



/**
 * Formats a string into the buffer specified by @a dest.
 *
 * This function is identical to snprintf, only there is no need to concern with
 * the buffer size. Nul-termination is guaranteed if @a dest_size is at least 1.
 * Improper use of format strings can still result in security risks, so always
 * use as much safety as you would to normal statements.
 *
 * char	buf[24];
 * int32_t	num = 5;
 * str_format("The integer is: %i; amazing stuff!\\n"), num);
 *
 * In this case, the string is truncated to "The integer is: 5; amaz".
 *
 * @param dest The buffer to store the formatted string in
 * @param dest_size The size of the destination buffer
 * @param format The format of the input string
 * @param ... Variable arguments for the format string
 * @return Returns 0 if any  parameter is incorrect, or of not great enough
 * size, or 'format' is null or less than 2 characters in length.
 * @return Returns the number of characters (excluding the nul), or -1
 * (SIZE_MAX) if the text was truncated to fit in the buffer.
 */
SBI_API
uint32_t
str_format(
	char* dest,
	uint32_t dest_size,
	char* format,
	...
);



/**
 * Equivalent of strtok_r, cross-platform.
 */
SBI_API
char*
str_token(
	char* src,
	const char* delim,
	char** context
);



SBI_API
char*
str_trim(
	char* src
);



#ifdef __cplusplus
}	// extern "C"
#endif



/**
 * Takes the input std::string and converts it to the platform type string. Just
 * a helper to prevent code/preprocessor spam when dealing with Win32 vs *nix
 * builds.
 *
 * Where CHARSTRINGTYPE == std::string, this function performs no modifications.
 *
 * @return Returns the input string as the platform type specific string,
 * converting where needed.
 */
SBI_API
CHARSTRINGTYPE
mbstr_to_chartypestr(
	std::string& src
);



END_NAMESPACE

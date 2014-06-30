#if defined(__linux__)


#include "utils_linux.h"



uint32_t
get_current_binary_path(
	char* buffer,
	uint32_t buffer_size
)
{
	char*	r;

	if ( buffer_size < 2 )
		return 0;

	int32_t		res = readlink("/proc/self/exe", buffer, buffer_size);

	if ( res == -1 )
	{
		return 0;
	}

	// find the last trailing path separator
	if ( (r = strrchr(buffer, PATH_CHAR)) == nullptr )
	{
		//generate_error_str_arg(EC_ApiFunctionFailed, 0, "The buffer for the current path contained no path separators");
		return 0;
	}

	// nul out the character after it, ready for appending
	*++r = '\0';

	// return number of characters written, after taking into account the new nul
	return (r - &buffer[0]);
}



#endif	// __linux__

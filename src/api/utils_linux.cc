#if defined(__linux__)
/**
 * @file	utils_linux.cc
 * @author	James Warren
 * @copyright	James Warren, 2013-2014
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 */



#include <unistd.h>			// readlink
#include <execinfo.h>		// backtrace
#include <string.h>
#include <fcntl.h>			// open flags
#include "Terminal.h"
#include "utils_linux.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



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
	if (( r = strrchr(buffer, PATH_CHAR)) == nullptr )
	{
		std::cerr << fg_red << "The buffer for the current path contained no path separators\n";
		return 0;
	}

	// nul out the character after it, ready for appending
	*++r = '\0';

	// return number of characters written, after taking into account the new nul
	return (r - &buffer[0]);
}



void
segfault_handler(
    int32_t sig
)
{
	/* well this looks very nice...
	 * http://stackoverflow.com/questions/4636456/stack-trace-for-c-using-gcc
	 */

	const int32_t	array_size = 100;
	void*	array[array_size];
	char**	text;
	int32_t	num_ptrs;
	int32_t	i;
	int32_t	fd;

	num_ptrs = backtrace(array, array_size);
	std::cerr << fg_red 
		<< "\n********************\n Segmentation Fault\n********************\n\n"
		"Backtrace contains " << fg_magenta << num_ptrs << fg_red << " addresses:\n\n";

	// all yellow foreground from here on
	std::cerr << fg_yellow;

	text = backtrace_symbols(array, num_ptrs);
	if ( text == nullptr )
	{
		std::cerr << fg_yellow << "Nothing returned from backtrace_symbols\n";
		exit(EXIT_FAILURE);
	}

	fd = open(".backtrace_segfault", O_WRONLY|O_CREAT, 0664);

	for ( i = 0; i < num_ptrs; i++ )
	{
		if ( fd != -1 )
		{
			write(fd, text[i], strlen(text[i]));
			write(fd, "\n", 1);
		}

		std::cerr << "\t" << text[i] << "\n";
	}

	if ( fd != -1 )
		close(fd);

	free(text);

	// this should cause all streams to be flushed and closed cleanly..
	exit(EXIT_FAILURE);
}


END_NAMESPACE

#endif	// __linux__

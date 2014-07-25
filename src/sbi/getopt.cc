#if defined(_WIN32)
/**
 * @file	src/sbi/getopt.cc
 * @author	James Warren
 * @copyright	James Warren, 2013-2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */


#include <cstring>		// strcmp 
#include <cstdio>		// EOF

#include "getopt.h"		// prototypes


BEGIN_NAMESPACE(APP_NAMESPACE)


int32_t		getopt_ind = 0;		// global argv index
char*		getopt_arg;		// global argument pointer



int32_t
getopt(
	int32_t argc,
	char** argv,
	char* opt
)
{
	static char*	next = nullptr;
	char		c;
	char*		cp;

	if ( getopt_ind == 0 )
		next = nullptr;

	getopt_arg = nullptr;

	if ( next == nullptr || *next == '\0' )
	{
		if ( getopt_ind == 0 )
			getopt_ind++;

		if ( getopt_ind >= argc ||
		     argv[getopt_ind][0] != '-' ||
		     argv[getopt_ind][1] == '\0' )
		{
			getopt_arg = nullptr;
			if ( getopt_ind < argc )
				getopt_arg = argv[getopt_ind];
			return EOF;
		}

		if ( strcmp(argv[getopt_ind], "--") == 0 )
		{
			getopt_ind++;
			getopt_arg = nullptr;
			if ( getopt_ind < argc )
				getopt_arg = argv[getopt_ind];
			return EOF;
		}

		next = argv[getopt_ind];
		next++;     /* skip past '-' */
		getopt_ind++;
	}

	c = *next++;
	cp = strchr(opt, c);

	if ( cp == nullptr || c == ':' )
		return c;

	cp++;

	if ( *cp == ':' )
	{
		if ( *next != '\0' )
		{
			getopt_arg = next;
			next = nullptr;
		}
		else if ( getopt_ind < argc )
		{
			getopt_arg = argv[getopt_ind];
			getopt_ind++;
		}
		else
		{
			return *next;
		}
	}

	return c;
}



END_NAMESPACE

#endif	// _WIN32

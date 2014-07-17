
/**
 * @file	utils.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



// we're C++ using C functions, so headers are slightly different
#include <cstring>		// strlen
#include <cstdarg>		// va_*
#include <cstdio>		// vsnprintf
#include <ctime>		// time + date acquistion
#include <ctype.h>		// isspace
#include <cstdlib>		// rand

#if defined(_WIN32)
#	include <Windows.h>	// OS API
#else
#	include <sys/time.h>

	// for rename_thread
#	if defined(__linux__)
#		include <sys/prctl.h>
#	elif defined(BSD)
#		include <pthread_np.h>
#	endif
#endif

#include "utils.h"		// prototypes



BEGIN_NAMESPACE(APP_NAMESPACE)



std::string
build_string(
	int16_t num_args,
	...
)
{
	std::string	ret_str;
	va_list		args;

	va_start(args, num_args);

	while ( num_args-- >= 0 )
	{
		ret_str.append((char*)va_arg(args, char*));
	}

	va_end(args);

	return ret_str;
}



#if 0
char*
gen_random_string(
	uint32_t min_chars,
	uint32_t max_chars,
	uint32_t seed
)
{
	char	gen[MAX_LEN_GENERIC];
	char*	alloc_str = nullptr;
	uint32_t	rand_count = 0;
	uint32_t	i;
	const char	alpha[] = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ")
		"abcdefghijklmnopqrstuvwxyz");

	if ( max_chars >= min_chars
	    && min_chars > 0
	    && max_chars < MAX_LEN_GENERIC )
	{
		// set seed if specified
		if ( seed != 0 )
			srand(seed);

		// if the string length is hard-set, don't rand-gen
		if ( min_chars == max_chars )
			rand_count = min_chars;

		while ( rand_count < min_chars )
		{
			// loop until we get a number between min_chars & max_chars 
			rand_count = rand() % max_chars;
		}

		for ( i = 0; i < rand_count; i++ )
		{
			// fill buffer with random characters
			gen[i] = alpha[rand() % (sizeof(alpha) -1)];
		}

		gen[i] = '\0';

		alloc_and_copy(&alloc_str, gen);
	}

	return alloc_str;
}
#endif



// acquired from http://stackoverflow.com/questions/1861294/how-to-calculate-execution-time-of-a-code-snippet-in-c
uint64_t
get_ms_time()
{
#if defined(_WIN32)
	FILETIME	ft;
	LARGE_INTEGER	li;

	/* Get the amount of 100 nano seconds intervals elapsed since January 1,
	 * 1601 (UTC) and copy it to a LARGE_INTEGER structure. */
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	uint64_t	ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

	return ret;
#else
	struct timeval	tv;

	gettimeofday(&tv, NULL);

	uint64_t	ret = tv.tv_usec;
	/* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
	ret /= 1000;

	/* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
	ret += (tv.tv_sec * 1000);

	return ret;
#endif
}



char*
get_current_time_format(
	char* buf,
	const uint32_t buf_size,
	const char* format
)
{
	time_t		cur_time = time(NULL);
	tm		tms;

#if defined(_WIN32)
	localtime_s(&tms, &cur_time);
#else
	localtime_r(&cur_time, &tms);
#endif
	
	// requires C++11 (always multibyte)
	std::strftime(buf, buf_size, format, &tms);

	return &buf[0];
}



CHARSTRINGTYPE
mbstr_to_chartypestr(
	std::string& src
)
{
#if defined(_WIN32)
	CHARTYPE	x[4096];
	CHARSTRINGTYPE	ret;

	mb_to_utf8(x, src.c_str(), _countof(x));
	ret = x;
	return ret;
#else
	return src;
#endif
}



void
rename_thread(
	const char* name
)
{
#if IS_VISUAL_STUDIO	// is this safe to use _WIN32 instead?
	set_thread_name(GetCurrentThreadId(), name);
#elif defined(__linux__)
	::prctl(PR_SET_NAME, name, 0, 0, 0);
#elif defined(BSD)
	pthread_set_name_np(pthread_self(), name);
#else
	(void)name;
#endif
}



char*
skip_whitespace(
	char* str
)
{
	while ( isspace(*str) )
		++str;

	return str;
}



uint32_t
strlcat(
	char* dest,
	const char* src,
	uint32_t dest_size
)
{
	char*		d = dest;
	const char*	s = src;
	uint32_t	n = dest_size;
	uint32_t	len;

	// Find the end of dst and adjust bytes left but don't go past end
	while ( *d != '\0' && n-- != 0 )
		d++;

	len = d - dest;
	n = dest_size - len;

	if ( n == 0 )
		return (len + strlen(s));

	while ( *s != '\0' )
	{
		if ( n != 1 )
		{
			*d++ = *s;
			n--;
		}
		s++;
	}

	*d = '\0';

	// count does not include NUL
	return (len + (s - src));	
}



uint32_t
strlcpy(
	char* dest,
	const char* src,
	uint32_t dest_size
)
{
	char*		d = dest;
	const char*	s = src;
	uint32_t	n = dest_size;

	// Copy as many bytes as will fit
	if ( n != 0 && --n != 0 )
	{
		do
		{
			if ( (*d++ = *s++) == 0 )
				break;
		} while ( --n != 0 );
	}

	// Not enough room in dest, add NUL and traverse rest of src
	if ( n == 0 )
	{
		if ( dest_size != 0 )
		{
			// nul-terminate dest
			*d = '\0';
		}
		while ( *s++ );
	}

	// count does not include NUL
	return (s - src - 1);	
}



uint32_t
str_format(
	char* destination,
	uint32_t dest_size,
	char* format,
	...
)
{
	int32_t		res = 0;
	va_list		varg;

	if ( destination == nullptr )
		return 0;
	if ( format == nullptr )
		return 0;
	if ( dest_size <= 1 )
		return 0;

	va_start(varg, format);

#if MSVC_IS_VS8_OR_LATER
#	pragma warning ( push )
#	pragma warning ( disable : 4996 ) // vsnprintf - unsafe function
#endif
	/* always leave 1 for the nul terminator - this is the security complaint
	 * that visual studio will warn us about. Since we have coded round it,
	 * forcing each instance to include '-1' with a min 'dest_size' of 1, this
	 * is perfectly safe. */
	res = vsnprintf(destination, (dest_size - 1), format, varg);

#if MSVC_IS_VS8_OR_LATER
#	pragma warning ( pop )
#endif

	va_end(varg);

	if ( res == -1 )
	{
		// destination text has been truncated/error
		destination[dest_size - 1] = '\0';
		return 0;
	}
	else
	{
		// to ensure nul-termination
		destination[res] = '\0';
	}

	// will be positive as not an error 
	return (uint32_t) res;
}



char*
str_token(
	char* src,
	const char* delim,
	char** context
)
{
	char*	ret = nullptr;

	if ( src == nullptr )
	{
		src = *context;
	}

	// skip leading delimiters
	while ( *src && strchr(delim, *src) )
	{
		++src;
	}

	if ( *src == '\0' )
		return ret;

	ret = src;

	// break on end of string or upon finding a delimiter
	while ( *src && !strchr(delim, *src) )
	{
		++src;
	}

	// if a delimiter was found, nul it
	if ( *src )
	{
		*src++ = '\0';
	}

	*context = src;

	return ret;
}



char*
str_trim(
	char* src
)
{
	size_t	len;
	char*	e;

	if ( src == nullptr )
		return nullptr;

	src = skip_whitespace(src);
	len = strlen(src);

	if ( !len )
		return src;

	e = src + len - 1;
	while ( e >= src && isspace(*e) )
		e--;
	*(e + 1) = '\0';

	return src;
}



END_NAMESPACE


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

#include "Allocator.h"		// memory allocation macros
#include "Terminal.h"
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



const static char* b64="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// maps A=>0,B=>1..
const static unsigned char unb64[] = {
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //10
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //20
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //30
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //40
	 0,  0,  0, 62,  0,  0,  0, 63, 52, 53, //50
	54, 55, 56, 57, 58, 59, 60, 61,  0,  0, //60
	 0,  0,  0,  0,  0,  0,  1,  2,  3,  4, //70
	 5,  6,  7,  8,  9, 10, 11, 12, 13, 14, //80
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, //90
	25,  0,  0,  0,  0,  0,  0, 26, 27, 28, //100
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, //110
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48, //120
	49, 50, 51,  0,  0,  0,  0,  0,  0,  0, //130
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //140
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //150
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //160
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //170
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //180
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //190
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //200
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //210
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //220
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //230
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //240
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, //250
	 0,  0,  0,  0,  0,  0,
}; // This array has 255 elements



char*
base64(
	const void* data,
	int len,
	int* flen
)
{
	const unsigned char*	bin = (const unsigned char*)data;
	uint32_t	alloc;
	char*		res;
	int	rc = 0 ; // result counter
	int	byte_num; // needed after the loop
	int	modulus_len = len % 3;
	int	pad = ((modulus_len & 1) << 1) + ((modulus_len & 2) >> 1) ; // 2 gives 1 and 1 gives 2, but 0 gives 0.

	*flen = 4 * (len + pad) / 3;
	alloc = *flen + 1;
	res = (char*)MALLOC(alloc); // and one for the nul

	if ( !res )
	{
		std::cerr << fg_red << "Memory allocation failed (" << alloc << ") bytes\n";
		return 0;
	}

	for ( byte_num = 0; byte_num <= len-3; byte_num += 3 )
	{
		unsigned char	byte0 = bin[byte_num    ];
		unsigned char	byte1 = bin[byte_num + 1];
		unsigned char	byte2 = bin[byte_num + 2];

		res[rc++] = b64[ byte0 >> 2 ] ;
		res[rc++] = b64[ ((0x3 & byte0) << 4) + (byte1 >> 4) ];
		res[rc++] = b64[ ((0x0f & byte1) << 2) + (byte2 >> 6) ];
		res[rc++] = b64[ 0x3f & byte2 ];
	}

	if ( pad == 2 )
	{
		res[rc++] = b64[ bin[byte_num] >> 2 ];
		res[rc++] = b64[ (0x3 & bin[byte_num]) << 4 ];
		res[rc++] = '=';
		res[rc++] = '=';
	}
	else if ( pad == 1 )
	{
		res[rc++] = b64[ bin[byte_num] >> 2 ] ;
		res[rc++] = b64[ ((0x3 & bin[byte_num]) << 4) + (bin[byte_num+1] >> 4) ];
		res[rc++] = b64[ (0x0f & bin[byte_num+1]) << 2 ] ;
		res[rc++] = '=';
	}

	res[rc] = 0; // nul terminator
	return res;
}



unsigned char*
unbase64(
	const char* ascii,
	int len,
	int* flen
)
{
	const unsigned char*	input = (const unsigned char*)ascii;
	unsigned char*	bin;
	uint32_t	alloc;
	int		cb = 0;
	int		char_num;
	int		pad = 0 ;

	if ( len < 2 )
	{
		// 2 accesses below would be OOB.
		// catch empty string, return NULL as result.
		std::cerr << fg_red << "Invalid base64 string (too short).\n";
		*flen = 0;
		return nullptr;
	}

	if ( input[len-1] == '=' )
		++pad;
	if ( input[len-2] == '=' )
		++pad;

	*flen = 3 * len/4 - pad;
	alloc = *flen;

	if (( bin = (unsigned char*)MALLOC(alloc)) == nullptr )
	{
		std::cerr << fg_red << "Memory allocation failed (" << alloc << ") bytes\n";
		return nullptr;
	}

	for ( char_num = 0; char_num <= len - 4 - pad; char_num += 4 )
	{
		int	a = unb64[input[char_num    ]];
		int	b = unb64[input[char_num + 1]];
		int	c = unb64[input[char_num + 2]];
		int	d = unb64[input[char_num + 3]];

		bin[cb++] = (a << 2) | (b >> 4);
		bin[cb++] = (b << 4) | (c >> 2);
		bin[cb++] = (c << 6) | (d);
	}

	if ( pad == 1 )
	{
		int	a = unb64[input[char_num    ]];
		int	b = unb64[input[char_num + 1]];
		int	c = unb64[input[char_num + 2]];

		bin[cb++] = (a << 2) | (b >> 4);
		bin[cb++] = (b << 4) | (c >> 2);
	}
	else if ( pad == 2 )
	{
		int	a = unb64[input[char_num    ]];
		int	b = unb64[input[char_num + 1]];

		bin[cb++] = (a << 2) | (b >> 4) ;
	}

	return bin;
}



bool
wildcard_match(
	const char* psz,
	const char* mask
)
{
	for ( ;; )
	{
		switch ( *mask )
		{
		case '\0':
			return (*psz == '\0');
		case '*':
			return wildcard_match(psz, mask + 1) || (*psz && wildcard_match(psz + 1, mask));
		case '?':
			if ( *psz == '\0' )
				return false;
			break;
		default:
			if ( *psz != *mask )
				return false;
			break;
		}

		psz++;
		mask++;
	}
}



bool
wildcard_match(
	const std::string& str,
	const std::string& mask
)
{
	return wildcard_match(str.c_str(), mask.c_str());
}



END_NAMESPACE


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



std::vector<unsigned char>
decode_base32(
	const char* p,
	bool* pfInvalid
)
{
	unsigned char		mode = 0;
	unsigned char		left = 0;
	static const int	decode32_table[256] =
	{
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 0, 1, 2,
		3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
		23, 24, 25, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};

	if ( pfInvalid )
		*pfInvalid = false;

	std::vector<unsigned char>	vect;
	vect.reserve((strlen(p)) * 5 / 8);

	while ( 1 )
	{
		unsigned char	dec = decode32_table[(unsigned char) *p];

		if ( dec == -1 )
			break;

		p++;

		switch ( mode )
		{
		case 0: // we have no bits and get 5
			left = dec;
			mode = 1;
			break;
		case 1: // we have 5 bits and keep 2
			vect.push_back((left << 3) | (dec >> 2));
			left = dec & 3;
			mode = 2;
			break;
		case 2: // we have 2 bits and keep 7
			left = left << 5 | dec;
			mode = 3;
			break;
		case 3: // we have 7 bits and keep 4
			vect.push_back((left << 1) | (dec >> 4));
			left = dec & 15;
			mode = 4;
			break;
		case 4: // we have 4 bits, and keep 1
			vect.push_back((left << 4) | (dec >> 1));
			left = dec & 1;
			mode = 5;
			break;
		case 5: // we have 1 bit, and keep 6
			left = left << 5 | dec;
			mode = 6;
			break;
		case 6: // we have 6 bits, and keep 3
			vect.push_back((left << 2) | (dec >> 3));
			left = dec & 7;
			mode = 7;
			break;
		case 7: // we have 3 bits, and keep 0
			vect.push_back((left << 5) | dec);
			mode = 0;
			break;
		}
	}

	if ( pfInvalid )
	{
		switch ( mode )
		{
		case 0: // 8n base32 characters processed: ok
			break;
		case 1: // 8n+1 base32 characters processed: impossible
		case 3: //   +3
		case 6: //   +6
			*pfInvalid = true;
			break;
		case 2: // 8n+2 base32 characters processed: require '======'
			if ( left || p[0] != '=' || p[1] != '=' || p[2] != '=' || p[3] != '=' || p[4] != '=' || p[5] != '=' || decode32_table[(unsigned char) p[6]] != -1 )
				*pfInvalid = true;
			break;
		case 4: // 8n+4 base32 characters processed: require '===='
			if ( left || p[0] != '=' || p[1] != '=' || p[2] != '=' || p[3] != '=' || decode32_table[(unsigned char) p[4]] != -1 )
				*pfInvalid = true;
			break;
		case 5: // 8n+5 base32 characters processed: require '==='
			if ( left || p[0] != '=' || p[1] != '=' || p[2] != '=' || decode32_table[(unsigned char) p[3]] != -1 )
				*pfInvalid = true;
			break;
		case 7: // 8n+7 base32 characters processed: require '='
			if ( left || p[0] != '=' || decode32_table[(unsigned char) p[1]] != -1 )
				*pfInvalid = true;
			break;
		}
	}

	return vect;
}



std::string
decode_base32(
	const std::string& str
)
{
	std::vector<unsigned char>	vect = decode_base32(str.c_str());

	return std::string((const char*) &vect[0], vect.size());
}



std::vector<unsigned char>
decode_base64(
	const char* p,
	bool* pfInvalid
)
{
	unsigned char		mode = 0;
	unsigned char		left = 0;
	static const int	decode64_table[256] =
	{
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1,
		-1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28,
		29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
		49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};

	if ( pfInvalid )
	{
		*pfInvalid = false;
	}

	std::vector<unsigned char>	vect;
	vect.reserve(strlen(p) * 3 / 4);

	while ( 1 )
	{
		unsigned char	dec = decode64_table[(unsigned char) *p];

		if ( dec == -1 )
			break;

		p++;

		switch ( mode )
		{
		case 0: // we have no bits and get 6
			left = dec;
			mode = 1;
			break;
		case 1: // we have 6 bits and keep 4
			vect.push_back((left << 2) | (dec >> 4));
			left = dec & 15;
			mode = 2;
			break;
		case 2: // we have 4 bits and get 6, we keep 2
			vect.push_back((left << 4) | (dec >> 2));
			left = dec & 3;
			mode = 3;
			break;
		case 3: // we have 2 bits and get 6
			vect.push_back((left << 6) | dec);
			mode = 0;
			break;
		}
	}

	if ( pfInvalid )
	{
		switch ( mode )
		{
		case 0: // 4n base64 characters processed: ok
			break;
		case 1: // 4n+1 base64 character processed: impossible
			*pfInvalid = true;
			break;
		case 2: // 4n+2 base64 characters processed: require '=='
			if ( left || p[0] != '=' || p[1] != '=' || decode64_table[(unsigned char) p[2]] != -1 )
				*pfInvalid = true;
			break;
		case 3: // 4n+3 base64 characters processed: require '='
			if ( left || p[0] != '=' || decode64_table[(unsigned char) p[1]] != -1 )
				*pfInvalid = true;
			break;
		}
	}

	return vect;
}



std::string
decode_base64(
	const std::string& str
)
{
	std::vector<unsigned char>	vect = decode_base64(str.c_str());

	return std::string((const char*)&vect[0], vect.size());
}



std::string
encode_base32(
	const unsigned char* pch,
	size_t len
)
{
	static const char*	pbase32 = "abcdefghijklmnopqrstuvwxyz234567";
	static const int32_t	padding[5] = { 0, 6, 4, 3, 1 };
	std::string		ret = "";
	unsigned char		mode = 0;
	unsigned char		left = 0;
	const unsigned char*	endp = pch + len;

	ret.reserve((len + 4) / 5 * 8);

	while ( pch < endp )
	{
		int32_t	enc = *(pch++);

		switch ( mode )
		{
		case 0: // we have no bits
			ret += pbase32[enc >> 3];
			left = (enc & 7) << 2;
			mode = 1;
			break;
		case 1: // we have three bits
			ret += pbase32[left | (enc >> 6)];
			ret += pbase32[(enc >> 1) & 31];
			left = (enc & 1) << 4;
			mode = 2;
			break;
		case 2: // we have one bit
			ret += pbase32[left | (enc >> 4)];
			left = (enc & 15) << 1;
			mode = 3;
			break;
		case 3: // we have four bits
			ret += pbase32[left | (enc >> 7)];
			ret += pbase32[(enc >> 2) & 31];
			left = (enc & 3) << 3;
			mode = 4;
			break;
		case 4: // we have two bits
			ret += pbase32[left | (enc >> 5)];
			ret += pbase32[enc & 31];
			mode = 0;
		}
	}

	if ( mode )
	{
		ret += pbase32[left];

		for ( int32_t n = 0; n < padding[mode]; n++ )
			ret += '=';
	}

	return ret;
}



std::string
encode_base32(
	const std::string& str
)
{
	return encode_base32((const unsigned char*) str.c_str(), str.size());
}



std::string
encode_base64(
	const unsigned char* pch, 
	size_t len
)
{
	static const char*	pbase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string		ret = "";
	int32_t			mode = 0;
	int32_t			left = 0;
	const unsigned char*	endp = pch + len;

	ret.reserve((len + 2) / 3 * 4);

	while ( pch < endp )
	{
		int32_t	enc = *(pch++);

		switch ( mode )
		{
		case 0: // we have no bits
			ret += pbase64[enc >> 2];
			left = (enc & 3) << 4;
			mode = 1;
			break;
		case 1: // we have two bits
			ret += pbase64[left | (enc >> 4)];
			left = (enc & 15) << 2;
			mode = 2;
			break;
		case 2: // we have four bits
			ret += pbase64[left | (enc >> 6)];
			ret += pbase64[enc & 63];
			mode = 0;
			break;
		}
	}

	if ( mode )
	{
		ret += pbase64[left];
		ret += '=';

		if ( mode == 1 )
			ret += '=';
	}

	return ret;
}



std::string
encode_base64(
	const std::string& str
)
{
	return encode_base64((const unsigned char*)str.c_str(), str.size());
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



bool
wildcard_match(
	const char* psz,
	const char* mask
)
{
	while ( 1 )
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


/**
 * @file	nethelper.cc
 * @author	James Warren
 * @copyright	James Warren, 2013
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 * @todo	host_to_ipv6 & ipv6_to_host addition
 */


#include <string>		// string handling, char_traits

#if defined(_WIN32)
#	include <WS2tcpip.h>		// WinSock2 & sockaddr_in6, needed before openssl
#	if ( _WIN32_WINNT >= _WIN32_WINNT_VISTA )
#		include <in6addr.h>	// s6_addr
#	endif
#else
#	include <netdb.h>		// getnameinfo
#endif

#if defined(USING_OPENSSL_NET)
#	include <openssl/err.h>		// ERR_load_xxx_strings
#	include <openssl/ssl.h>		// SSL_library_init
#endif


#include <api/char_helper.h>
#include <api/utils.h>		// strlcpy functions
#include <api/Terminal.h>	// output
#include <api/Runtime.h>	// app runtime
#include <api/Log.h>		// logging
#include "nethelper.h"		// prototypes


BEGIN_NAMESPACE(APP_NAMESPACE)


int32_t
host_to_ipv4(
	const char* hostname,
	int32_t index,
	char* ip_dest,
	uint32_t dest_size
)
{
	struct addrinfo		hints = { 0 };
	struct addrinfo*	result = NULL;
	struct addrinfo*	rp = NULL;
	int32_t		ret = 0;
	int32_t		res;

	// obtain address(es) matching host

	// IPv4 only
	hints.ai_family		= AF_INET;


	if (( res = getaddrinfo(hostname, NULL, &hints, &result)) != 0 )
	{
		LOG(ELogLevel::Error) << fg_red 
			<< "Error " << res 
			<< " (" << gai_strerror(res) 
			<< ") looking up" << hostname 
			<< "\n";

		ret--;
	}
	else
	{
		for ( rp = result; rp != NULL; rp = rp->ai_next, ++ret )
		{
			struct sockaddr_in	*a = (struct sockaddr_in*)rp->ai_addr;

			if ( index < 0 )
			{
				// no index specified; copy the first result
				strlcpy(ip_dest, (char*)inet_ntoa(a->sin_addr), dest_size);
				index = 0;
			}
			else if ( index == ret )
			{
				// copy the specified index
				strlcpy(ip_dest, (char*)inet_ntoa(a->sin_addr), dest_size);
			}
		}

		if ( index >= ret )
		{
			LOG(ELogLevel::Error) << fg_red 
				<< "The specified index '"
				<< index << "' is out of range ("
				<< ret << ")\n";
		}
		else
		{
			LOG(ELogLevel::Info) << fg_green 
				<< hostname << " resolved to "
				<< ip_dest << "\n";
		}
	}

	if ( result != NULL )
		freeaddrinfo(result);

	// -1 on error
	// 0 is abnormal (lookup succeeded, but no results)
	// >=1 if 1 or more results
	return ret;
}



bool
ipv4_to_host(
	const char* ipv4_address,
	char* host_dest,
	uint32_t dest_size
)
{
	struct addrinfo		hints = { 0 };
	struct addrinfo*	result = NULL;
	int32_t			res;

	// IPv4 only
	hints.ai_family		= AF_INET;


	if (( res = getaddrinfo(ipv4_address, NULL, &hints, &result)) != 0 )
	{
		LOG(ELogLevel::Error) << fg_red 
			<< "Error " << res
			<< " (" << gai_strerror(res)
			<< ") looking up" << ipv4_address
			<< "\n";
		return false;
	}

	/* we don't want to fail if the name cannot be looked up, so supply no
	 * flags; if the lookup does not resolve,  */

	if (( res = getnameinfo(result->ai_addr, result->ai_addrlen, host_dest, dest_size, NULL, 0, 0)) != 0 )
	{
		LOG(ELogLevel::Error) << fg_red 
			<< "Error " << res
			<< " (" << gai_strerror(res)
			<< ") looking up" << ipv4_address
			<< "\n";
		freeaddrinfo(result);
		return false;
	}



	LOG(ELogLevel::Info) << ipv4_address << " resolved to " << host_dest << "\n";

	freeaddrinfo(result);
	return true;
}



bool
is_ip_address(
	const char* data
)
{
	struct ip_address	ipaddr;

	// inet_pton returns 1 on success, -1 on error, or 0 if input is invalid
	if ( inet_pton(AF_INET, data, &ipaddr.data) != 1 )
	{
		if ( inet_pton(AF_INET6, data, &ipaddr.data) != 1 )
		{
			// not an IPv4 or IPv6 address
			return false;
		}
	}

	// is an IPv4 or IPv6 address
	return true;
}



void
net_cleanup()
{
	/* no error checking to be done, since we only call this when the app
	 * is closing, and needless reporting will slow it down */

#if defined(USING_OPENSSL_NET)
	// free OpenSSLs loaded error strings
	ERR_free_strings();
#endif

#if defined(_WIN32)
	// unload winsock
	WSACleanup();
#endif
}



bool
net_startup(
	uint16_t winsock_version
)
{
#if defined(USING_OPENSSL_NET)
	// load the openssl components
	/** @todo - check library_init for errors; others are void */
	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_crypto_strings();
	ERR_load_BIO_strings();
	ERR_load_SSL_strings();
	ERR_load_ERR_strings();
	/* in case of an older version of OpenSSL being used; see man page */
	OpenSSL_add_all_algorithms();
#endif

#if defined(_WIN32)
	struct WSAData	wsa;

	if ( WSAStartup(winsock_version, &wsa) != 0 )
	{
		LOG(ELogLevel::Error) << fg_red 
			<< "Could not startup Winsock; Win32 error "
			<< GetLastError() << "\n";
		return false;
	}
#endif
	// cannot return false on non-Windows builds
	return true;
}



#if defined(USING_OPENSSL)

int32_t
openssl_err_callback(
	const char* str,
	size_t len,
	void* context
)
{
	context;
	len;

	/* fresh errors should already be logged on previous line, so tab along
	 * and output generic info */
	LOG(ELogLevel::Error) << fg_red << "OpenSSL Error: " << str;
	// returning 1 continues the callback chain
	return 1;
}

#endif



#if _WIN32_WINNT < _WIN32_WINNT_VISTA

/* This is from the BIND 4.9.4 release, modified to compile by itself */

/* Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#define IN6ADDRSZ	16
#define INADDRSZ	4
#define INT16SZ		2

/* disable all visual studio warnings that generate issues. This code is only
 * included if targetting Windows XP/2003, and is acquired from an external
 * source that should have taken all necessary precautions.
 * If you want max safety, set _WIN32_WINNT to Vista or newer. */

#pragma warning ( disable : 4996 )	// deprecated function, maybe unsafe

/* function declarations, required */

const char*
impl_inet_ntop4(
	const uint8_t* src,
	char* dst,
	size_t size
);

const char*
impl_inet_ntop6(
	const uint8_t* src,
	char* dst,
	size_t size
);

static int32_t
impl_inet_pton4(
	const char* src,
	uint8_t* dst
);

static int32_t
impl_inet_pton6(
	const char* src,
	uint8_t* dst
);



const char*
impl_inet_ntop(
	int32_t af,
	const void* src,
	char* dst,
	size_t size
)
{
	switch ( af )
	{
	case AF_INET:
		return (impl_inet_ntop4((const uint8_t*)src, dst, size));
	case AF_INET6:
		return (impl_inet_ntop6((const uint8_t*)src, dst, size));
	default:
		errno = EAFNOSUPPORT;
		return nullptr;
	}
}



const char*
impl_inet_ntop4(
	const uint8_t* src,
	char* dst,
	size_t size
)
{
	static char	fmt[] = "%u.%u.%u.%u";
	char		tmp[sizeof("255.255.255.255")];

	str_format(tmp, sizeof(tmp), fmt, src[0], src[1], src[2], src[3]);

	if ( strlcpy(dst, tmp, size) >= size )
	{
		errno = ENOSPC;
		return nullptr;
	}

	return dst;
}



const char*
impl_inet_ntop6(
	const uint8_t* src,
	char* dst,
	size_t size
)
{
	/*
	* Note that int32_t and int16_t need only be "at least" large enough
	* to contain a value of the specified size.  On some systems, like
	* Crays, there is no such thing as an integer variable with 16 bits.
	* Keep this in mind if you think this function should have been coded
	* to use pointer overlays.  All the world's not a VAX.
	*/
	char	tmp[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
	char*	tp;
	uint32_t	words[IN6ADDRSZ / INT16SZ];
	int32_t	i;

	struct { int base, len; } best, cur;


	/*
	* Preprocess:
	*  Copy the input (bytewise) array into a wordwise array.
	*  Find the longest run of 0x00's in src[] for :: shorthanding.
	*/
	memset(words, 0, sizeof words);
	for ( i = 0; i < IN6ADDRSZ; i++ )
		words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));

	best.base = -1;
	cur.base = -1;

	for ( i = 0; i < (IN6ADDRSZ / INT16SZ); i++ )
	{
		if ( words[i] == 0)
		{
			if ( cur.base == -1 )
				cur.base = i, cur.len = 1;
			else
				cur.len++;
		}
		else
		{
			if (cur.base != -1)
			{
				if ( best.base == -1 || cur.len > best.len )
					best = cur;
				cur.base = -1;
			}
		}
	}
	if ( cur.base != -1 )
	{
		if ( best.base == -1 || cur.len > best.len )
			best = cur;
	}
	if ( best.base != -1 && best.len < 2 )
		best.base = -1;

	/*
	* Format the result.
	*/
	tp = tmp;

	for ( i = 0; i < (IN6ADDRSZ / INT16SZ); i++ )
	{
		/* Are we inside the best run of 0x00's? */
		if (	best.base != -1 &&
			i >= best.base &&
			i < (best.base + best.len)
		)
		{
			if ( i == best.base )
				*tp++ = ':';
			continue;
		}
		/* Are we following an initial run of 0x00s or any real hex? */
		if ( i != 0 )
			*tp++ = ':';
		/* Is this address an encapsulated IPv4? */
		if (	i == 6 &&
			best.base == 0 &&
			(best.len == 6 || (best.len == 5 && words[5] == 0xffff))
		)
		{
			if ( !impl_inet_ntop4(src+12, tp, sizeof tmp - (tp - tmp)) )
				return nullptr;
			tp += strlen(tp);
			break;
		}
		sprintf(tp, "%x", words[i]);
		tp += strlen(tp);
	}
	/* Was it a trailing run of 0x00's? */
	if ( best.base != -1 && (best.base + best.len) == (IN6ADDRSZ / INT16SZ) )
		*tp++ = ':';
	*tp++ = '\0';

	if ( strlcpy(dst, tmp, size) >= size )
	{
		errno = ENOSPC;
		return nullptr;
	}

	return dst;
}



int32_t
impl_inet_pton(
	int32_t af,
	const char* src,
	void* dst
)
{
	switch ( af )
	{
	case AF_INET:
		return impl_inet_pton4(src, (uint8_t*)dst);
	case AF_INET6:
		return impl_inet_pton6(src, (uint8_t*)dst);
	default:
		errno = EAFNOSUPPORT;
		return -1;
	}
	/* NOTREACHED */
}



int32_t
impl_inet_pton4(
	const char* src,
	uint8_t* dst
)
{
	static const char	digits[] = "0123456789";
	int32_t	saw_digit;
	int32_t	octets;
	int32_t	ch;
	uint8_t	tmp[INADDRSZ];
	uint8_t	*tp;

	saw_digit = 0;
	octets = 0;
	tp = tmp;
	*tp = 0;

	while ( (ch = *src++) != '\0' )
	{
		const char*	pch;

		if ( (pch = strchr(digits, ch)) != NULL )
		{
			uint32_t	val = *tp * 10 + (uint32_t)(pch - digits);

			if ( saw_digit && *tp == 0 )
				return 0;
			if ( val > 255 )
				return 0;

			*tp = (uint8_t)val;
			if ( !saw_digit )
			{
				if ( ++octets > 4 )
					return 0;
				saw_digit = 1;
			}
		}
		else if ( ch == '.' && saw_digit )
		{
			if ( octets == 4 )
				return 0;
			*++tp = 0;
			saw_digit = 0;
		}
		else
			return 0;
	}

	if ( octets < 4 )
		return 0;

	memcpy(dst, tmp, INADDRSZ);

	return 1;
}



int32_t
impl_inet_pton6(
	const char* src,
	uint8_t* dst
)
{
	static const char	xdigits_l[] = "0123456789abcdef";
	static const char	xdigits_u[] = "0123456789ABCDEF";
	const char*	xdigits;
	const char*	curtok;
	uint8_t	tmp[IN6ADDRSZ];
	uint8_t	*tp;
	uint8_t	*endp;
	uint8_t	*colonp;
	int32_t	ch;
	int32_t	saw_xdigit;
	uint32_t	val;

	memset((tp = tmp), 0, IN6ADDRSZ);
	endp = tp + IN6ADDRSZ;
	colonp = NULL;
	/* Leading :: requires some special handling. */
	if ( *src == ':' )
	{
		if ( *++src != ':' )
			return 0;
	}

	curtok = src;
	saw_xdigit = 0;
	val = 0;

	while ( (ch = *src++) != '\0' )
	{
		const char*	pch;

		if ( (pch = strchr((xdigits = xdigits_l), ch)) == NULL )
			pch = strchr((xdigits = xdigits_u), ch);

		if ( pch != NULL )
		{
			val <<= 4;
			val |= (pch - xdigits);
			if ( ++saw_xdigit > 4 )
				return 0;
			continue;
		}
		if ( ch == ':' )
		{
			curtok = src;
			if ( !saw_xdigit )
			{
				if ( colonp )
					return 0;
				colonp = tp;
				continue;
			}
			if ( tp + INT16SZ > endp )
				return 0;

			*tp++ = (uint8_t) (val >> 8) & 0xff;
			*tp++ = (uint8_t) val & 0xff;

			saw_xdigit = 0;
			val = 0;
			continue;
		}
		if (	ch == '.' &&
			((tp + INADDRSZ) <= endp) &&
			impl_inet_pton4(curtok, tp) > 0
		)
		{
			tp += INADDRSZ;
			saw_xdigit = 0;
			break;    /* '\0' was seen by inet_pton4(). */
		}
		return 0;
	}

	if ( saw_xdigit )
	{
		if ( tp + INT16SZ > endp )
			return 0;
		*tp++ = (uint8_t) (val >> 8) & 0xff;
		*tp++ = (uint8_t) val & 0xff;
	}

	if ( colonp != NULL )
	{
		/*
		* Since some memmove()'s erroneously fail to handle
		* overlapping regions, we'll do the shift by hand.
		*/
		const long n = tp - colonp;
		long i;

		if ( tp == endp )
			return 0;
		for ( i = 1; i <= n; i++ )
		{
			endp[- i] = colonp[n - i];
			colonp[n - i] = 0;
		}
		tp = endp;
	}

	if ( tp != endp )
		return 0;

	memcpy(dst, tmp, IN6ADDRSZ);

	return 1;
}

#endif	// _WIN32_WINNT


END_NAMESPACE

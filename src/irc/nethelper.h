#pragma once

/**
 * @file	nethelper.h
 * @author	James Warren
 * @brief	Helper functions for IPv4 and IPv6 connections
 */



#if defined(_WIN32)
#	include <WS2tcpip.h>	// WinSock2 & sockaddr_in6
#	if _WIN32_WINNT >= _WIN32_WINNT_VISTA
		// moved out of WS2tcpip.h if Windows >= Vista */
#		include <in6addr.h>
#	else
		// inet_ntop/pton don't exist pre-Vista, redirect to our own
#		define inet_ntop	impl_inet_ntop
#		define inet_pton	impl_inet_pton
#	endif
#elif defined(__linux__) || defined(BSD)
#	include <sys/types.h>	// data type definitions
#	include <sys/socket.h>	// socket constants and structs
#	include <netinet/in.h>	// constants and structs for internet addresses
#	include <arpa/inet.h>	// inet_ntop, inet_pton
#endif

#include <api/char_helper.h>



BEGIN_NAMESPACE(APP_NAMESPACE)


/**
 * Stores an IP address for the family type
 *
 * @struct ip_address
 */
struct ip_address
{
	int32_t	family;		/**< AF_INET or AF_INET6 */

	union
	{
		struct in_addr	ip4;    /**< IPv4 address */
		struct in6_addr	ip6;    /**< IPv6 address */
	} data;
};


/**
 * A union of the different sockaddr structure possibilities.
 *
 * @struct sockaddr_union
 */
union sockaddr_union
{
	struct sockaddr		sa;
	struct sockaddr_in	sin;
	struct sockaddr_in6	sin6;
};



/**
 * Validates and looks up the supplied @a hostname. On success, the first IP
 * address (or overriden by index) is copied into @a ip_dest. This allows a
 * different address to be returned in case the first one tried is unreachable.
 *
 * If the index is out of range, an error is generated.
 *
 * Useful on systems that have a lookup cache, otherwise, you'll be doing a
 * fresh lookup every time you need to use an alternate address - hopefully
 * this shouldn't come up too often anyway.
 *
 * Example usage:
 *
 * @code
	char	buf[NI_MAXHOST];
	int		num_res;
	char	ip_dest[INET_ADDRSTRLEN];

	// fgets into buf

	if (( num_res = host_to_ipv4(buf, -1, ip_dest, sizeof(ip_dest))) >= 0 )
	{
		printf("Primary Index: %s\n", ip_dest);

		// do work on address, otherwise enter loop

		for ( int i = 1; i < num_res; i++ )
		{
			host_to_ipv4(buf, i, ip_dest, sizeof(ip_dest));
			printf("Secondary Index [%i]: %s\n", i, ip_dest);

			// attempt work on alternate address
		}
	}

 // Example Output::
 //
 // google.com
 // Primary Index: 173.194.34.65
 // Secondary Index [1]: 173.194.34.68
 // Secondary Index [2]: 173.194.34.70
 // Secondary Index [3]: 173.194.34.69
 // Secondary Index [4]: 173.194.34.66
 // Secondary Index [5]: 173.194.34.64
 // Secondary Index [6]: 173.194.34.78
 // Secondary Index [7]: 173.194.34.67
 // Secondary Index [8]: 173.194.34.73
 // Secondary Index [9]: 173.194.34.72
 // Secondary Index [10]: 173.194.34.71
 * @endcode
 *
 * @param[in] hostname The hostname to lookup
 * @param[in] index The 0-based index within the results to return
 * @param[out] ip_dest The buffer to copy the IP address to
 * @param[in] dest_size The size of @a ip_dest
 * @return The function returns the number of addresses found for the specified
 * @a hostname.
 * @return -1 is returned if the input data is invalid; 0 is returned if the
 * data was valid, but there were no results (this is abnormal).
 * @sa ipv4_to_host
 */
SBI_IRC_API
int32_t
host_to_ipv4(
	const char* hostname,
	int32_t index,
	char* ip_dest,
	uint32_t dest_size
);


/**
 * Performs a reverse lookup on the input ipv4_address.
 *
 * e.g. 173.194.34.67 reverse lookup: lhr14s19-in-f3.1e100.net
 *
 * @param[in] ipv4_address The IPv4 address to reverse-lookup
 * @param[out] host_dest The destination buffer to copy the hostname to
 * @param[in] dest_size The size of host_dest
 * @return The function returns true if the IP address is valid, and a lookup
 * is performed. If the name could not be found, true is still returned; only
 * on an error is false returned.
 * @sa host_to_ipv4
 */
SBI_IRC_API
bool
ipv4_to_host(
	const char* ipv4_address,
	char* host_dest,
	uint32_t dest_size
);


/**
 * Checks if the supplied string is an IPv4 or IPv6 address. Mostly used for
 * determining if a server in the config is an IP address or hostname.
 */
SBI_IRC_API
bool
is_ip_address(
	const char* data
);


/**
 * Cleans up all aspects of networking loaded within the application. Should
 * only ever be called before the application closes, as a chance to free any
 * dynamically allocated memory not yet freed.
 *
 * We use the opportunity to cleanup things that don't 'really' need to be
 * called, such as WSACleanup and ERR_free_strings; but it's good practice to
 * do so.
 *
 * @sa net_startup
 */
SBI_IRC_API
void
net_cleanup();


/**
 * Loads OpenSSL networking components (and non-SSL networking) - as Windows
 * utilizes (and requires) WinSock, this will also call WSAStartup on
 * Windows builds. winsock_version is ignored on non-Windows builds, and it
 * can therefore be omitted.
 *
 * @param[in] winsock_version The version of Winsock to load and initialize
 * @return Returns true on success, false on failure
 * @sa net_cleanup
 */
SBI_IRC_API
bool
net_startup(
	uint16_t winsock_version = 0
);



/**
 * Receives OpenSSL errors, ready to be output as desired
 */
SBI_IRC_API
int32_t
openssl_err_callback(
	const char* str,
	size_t len,
	void* context
);



#if _WIN32_WINNT < _WIN32_WINNT_VISTA

/**
 * Custom version of inet_ntop, when compiling for < _WIN32_WINNT_VISTA.
 * Originally acquired from BIND 4.9.4; may have some modifications to suit our
 * security/datatype requirements, otherwise the functionality is identical.
 */
SBI_IRC_API
const char*
impl_inet_ntop(
	int32_t af,
	const void* src,
	char* dst,
	size_t size
);



/**
 * Custom version of inet_pton, when compiling for < _WIN32_WINNT_VISTA.
 * Originally acquired from BIND 4.9.4; may have some modifications to suit our
 * security/datatype requirements, otherwise the functionality is identical.
 */
SBI_IRC_API
int32_t
impl_inet_pton(
	int32_t af,
	const char* src,
	void* dst
);

#endif	// _WIN32_WINNT



END_NAMESPACE

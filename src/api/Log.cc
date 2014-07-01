
/**
 * @file	Log.cc
 * @author	James Warren
 * @copyright	James Warren, 2013-2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include <iostream>		// std::cout in IS_DEBUG_BUILD
#include <ctime>		// time + date acquistion
#include <cassert>		// debug assertions

#if defined(__linux__)
#	include <sys/stat.h>			// file ops
#	include <fcntl.h>			// open() options
#	include <string.h>  			// strrchr
#endif

#include "Terminal.h"		// colour output
#include "Log.h"		// prototypes
#include "utils.h"		// string formatting for reporting



BEGIN_NAMESPACE(APP_NAMESPACE)




/** number of bytes the stream can contain before it will be flushed 
 * @bug threshold appears to make no difference - see if fixable */
#define STREAM_FLUSH_THRESHOLD		1024




Log::Log()
{
	 // operations append to the stream, and don't replace it
	 //_next_log.setstate(std::ios_base::app);
	 // logging at debug level by default 
	_log_level = ELogLevel::Warn;
}



Log::~Log()
{
	// if it hasn't already been closed, do it
	if ( _file != nullptr )
		Close();
}



void
Log::Append(
	const CHARSTRINGTYPE& append_string
)
{
	_next_log << append_string;

	if ( _next_log.gcount() > STREAM_FLUSH_THRESHOLD )
		Flush();
}



void
Log::Close()
{
	if ( _file != nullptr )
	{
		// don't print file/line info, call direct
		LogWithLevel(ELogLevel::Force) << "*** Log file closed ***\n";
		Flush();

		fclose(_file);
		_file = nullptr;
	}
}



void
Log::Flush()
{
	if ( _file == nullptr )
	{
		// don't keep the data
		RESET_STREAM(_next_log);
		return;
	}

	CHARSTRINGTYPE	writing = _next_log.str();
	size_t		written;
	int32_t		flush_res;

	if ( writing.empty() )
		return;

	written = fwrite(writing.c_str(), sizeof(CHARTYPE), writing.length(), _file);

	if ( (flush_res = fflush(_file)) != 0 )
	{
		char	errmsg[256];

#if defined(_WIN32)
		strerror_s(errmsg, sizeof(errmsg), ferror(_file));
#else
		strerror_r(errno, errmsg, sizeof(errmsg));
#endif
		std::cerr << fg_red << "fflush failed; " << errmsg << "\n";
	}

	// also output to console
	/// @todo temp; implement with chain of responsibility instead
	std::cout << writing.c_str();

	RESET_STREAM(_next_log);
}



CHARSTREAMTYPE&
Log::LogWithLevel(
	ELogLevel log_level,
	const char* file,
	const char* function,
	const uint32_t line
)
{
	if ( log_level != ELogLevel::Force && log_level > _log_level )
	{
		/* ugly, ugly, nasty hack. Returns another stringstream that is
		 * used temporarily, and cleared at every opportunity. */
		_null_log.clear();
		return _null_log;
	}

	// force log flushing if too much data is buffered
	if ( _next_log.gcount() > STREAM_FLUSH_THRESHOLD )
		Flush();

	if ( file != nullptr )
	{	
		/** @todo provide a utility function for time acquisition, this gets so nasty inline */
		time_t		cur_time = time(NULL);
		tm		tms;
#if defined(_WIN32)
		localtime_s(&tms, &cur_time);
#else
		localtime_r(&cur_time, &tms);
#endif
		const char*	p;
		char		cur_datetime[32];
		
		// requires C++11 (always multibyte)
		// ISO 8601 : %F %T (invalid format on Win7,VS2013)
#if defined(_WIN32)
		std::strftime(cur_datetime, sizeof(cur_datetime), "%Y-%m-%d %H:%M:%S", &tms);
#else
		std::strftime(cur_datetime, sizeof(cur_datetime), "%F %T", &tms);
#endif

		_next_log << cur_datetime << "\t";

		switch ( log_level )
		{
		case ELogLevel::Debug:	_next_log << "[DEBUG]    "; break;
		case ELogLevel::Error:	_next_log << "[ERROR]    "; break;
		case ELogLevel::Warn:	_next_log << "[WARNING]  "; break;
		case ELogLevel::Info:	_next_log << "[INFO]     "; break;
		case ELogLevel::Force:	_next_log << "[FORCED]   "; break;
		default:
			// no custom text by default
			break;
		}

		// we don't want the full path that some compilers set
		if ( (p = strrchr(file, PATH_CHAR)) != nullptr )
			file = (p + 1);

		_next_log << function << " (" << file << ":" << line << "): ";
	}

	return _next_log;
}



bool
Log::Open(
	const char* filename
)
{
	/* allow others to open the log file at runtime, but not for writing.
	 * This is implementation and operating system dependant as to what
	 * action is taken, if errors are forwarded (e.g. cpp streams setting
	 * errno, and win32 being able to use GetLastError()), and what errors
	 * we can actually retrieve (beyond just knowing an error occurred).
	 * Until there is a standard for doing this, back to the nice C-style
	 * methods here */
#if defined(_WIN32)
	if ( (_file = _fsopen(filename, "wb", _SH_DENYWR)) == nullptr )
	{
		std::cerr << fg_red << "Failed to open log file:\n\n" << filename << "\n\nerrno = " << errno << "\n";
		return false;
	}
#else
	int	fd;
	if ( (fd = open(filename, O_WRONLY | O_CREAT, O_NOATIME | S_IRWXU | S_IRGRP | S_IROTH)) == -1 )
	{
		std::cerr << fg_red << "Could not open the log file " << filename << "; errno " << errno << "\n";
		return false;
	}

	if ( (_file = fdopen(fd, "w")) == nullptr )
	{
		std::cerr << fg_red << "fdopen failed on the file descriptor for " << filename << "; errno " << errno << "\n";
		return false;
	}
#endif

	// Default log message (verifies path used); don't print file/line info, call direct
	LogWithLevel(ELogLevel::Force) << "*** Log File '" << filename << "' opened ***\n";

	return true;
}


END_NAMESPACE

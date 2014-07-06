
/**
 * @file	Interprocess.cc
 * @author	James Warren
 * @copyright	James Warren, 2014
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



// if we decide to use libev/libevent/boost::asio, they'll go here

#if defined(_WIN32)
#	include <process.h>
#elif defined(__linux__)
#	include <pthread.h>
#endif

#include "Interprocess.h"
#include "Ipc.h"
#include "Log.h"
#include "Terminal.h"
#include "utils.h"



BEGIN_NAMESPACE(APP_NAMESPACE)



Interprocess::Interprocess()
{
	
}



Interprocess::~Interprocess()
{

}



EIPCStatus
Interprocess::Close(
	char* name
)
{
	return EIPCStatus::Ok;
}



std::shared_ptr<Ipc>
Interprocess::Connect(
	char* name,
	EIPCAction action,
	void* unused
)
{
	return nullptr;
}



uint32_t
#if defined(_WIN32)
__stdcall
#endif
Interprocess::ExecWaitForClient(
	void* params
)
{
	wfc_params*	tp = reinterpret_cast<wfc_params*>(params);

	return tp->thisptr->WaitForClient(tp);
}



EIPCStatus
Interprocess::Open(
	char* name,
	EIPCAction action,
	void* unused
)
{
	ipc_map::iterator	iter;
	wfc_params		tparam;

	// search for it before creating it (which operator[] does)
	iter = _ipc_map.find(name);

	if ( iter != _ipc_map.end() )
	{
		// found [ (*iter).first ]; return exists
		return EIPCStatus::Exists;
	}

	std::shared_ptr<Ipc>	nipc(new Ipc);

	// create it
	_ipc_map[name] = nipc;
	// set it up

#if defined(_WIN32)
	tparam.ipc	= nipc;
	tparam.thisptr	= this;

	nipc->_name = name;
	nipc->_thread_handle = _beginthreadex(
		nullptr, 0, 
		ExecWaitForClient, 
		(void*)&tparam, CREATE_SUSPENDED,
		&tparam.ipc->_thread_id
	);
	if ( nipc->_thread_handle == -1 )
	{
		std::cerr << fg_red << "_beginthreadex failed\n";
		LOG(ELogLevel::Error) << "_beginthreadex failed\n";
		return EIPCStatus::ThreadCreateFailed;
	}

	ResumeThread((HANDLE)nipc->_thread_handle);

	/* wait for the thread to reset these; we don't want to exit scope
	 * before the thread has a chance to acquire their contents */
	while ( tparam.ipc != nullptr && tparam.thisptr != nullptr )
		SLEEP_MILLISECONDS(21);
	
#elif defined(__linux__)

#endif

	return Ok;
}



EIPCStatus
Interprocess::WaitForClient(
	wfc_params* tparam
)
{
	// input pointer won't live forever, copy the contents
	ipc_map::iterator	iter;
	std::shared_ptr<Ipc>	ipc = tparam->ipc;
	Interprocess*		thisptr = tparam->thisptr;
	char			fmt[MAX_PATH];
	wchar_t			w[MAX_PATH];

	// let the caller know we're done
	tparam->ipc	= nullptr;
	tparam->thisptr = nullptr;

	str_format(fmt, sizeof(fmt), "\\\\.\\pipe\\%s", ipc->_name);
	mb_to_utf8(w, fmt, _countof(w));

	// main body, lets go
	while ( ipc != nullptr )
	{
		BOOL	connected;

		ipc->_read = CreateNamedPipe(w,
		      PIPE_ACCESS_DUPLEX,
		      PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
		      PIPE_UNLIMITED_INSTANCES, // max instances
		      sizeof(ipc->_read_buffer),
		      sizeof(ipc->_write_buffer),
		      50, // timeout
		      NULL // sec_attrib
		);
		if ( ipc->_read == INVALID_HANDLE_VALUE )
		{
			DWORD	last_err = GetLastError();

			std::cerr << fg_red << "CreateNamedPipe failed: "
				<< error_code_as_string(last_err) << "\n";
			LOG(ELogLevel::Error) << "CreateNamedPipe failed; error "
				<< last_err << " ("
				<< error_code_as_string(last_err) << ")\n";

			// don't forget to delete the map entry we created
			iter = thisptr->_ipc_map.find(ipc->_name);
			_ipc_map.erase(iter);

			return EIPCStatus::CreateFailed;
		}
		// blocks until client connects
		connected = ConnectNamedPipe(ipc->_read, NULL);
				
		LOG(ELogLevel::Debug) << "New pipe connection received\n";
	}

	return EIPCStatus::Ok;
}



END_NAMESPACE

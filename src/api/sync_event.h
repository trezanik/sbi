#pragma once

/**
 * @file	sync_event.h
 * @author	James Warren
 * @brief	Linux/Unix equivalent of Windows' CreateEvent synchronization
 */



#if defined(_WIN32)
#	error "This file is for non-Windows builds only; you have included this by error"
#endif


#include <pthread.h>
#include <string.h>
#include "definitions.h"


BEGIN_NAMESPACE(APP_NAMESPACE)


/**
 * Holds the data to support Win32's Event style synchronization.
 *
 * Like pthreads, we have the struct created on the stack/heap as desired, and
 * when needed, you call sync_event_construct to make it ready for usage.
 *
 * Passing a sync_event into a function other than sync_event_construct when
 * uninitialized will result in undefined behaviour.
 *
 * For good practice, set flag to '2' when initially declared, so in your
 * cleanup code, if its 2 then it was never initialized, and does not need to be
 * destroyed, avoiding errors.
 *
 * @struct sync_event
 */
struct sync_event
{
	pthread_mutex_t		mutex;
	pthread_cond_t		condition;
	unsigned		flag:2;

	sync_event()
	{
		/* 'uninitialize' so we know what to expect (pthread types are
		 * unions) - this constructor makes this object a non-POD. */
		memset(&condition, 0, sizeof(condition));
		memset(&mutex, 0, sizeof(mutex));
		flag	= 2;
	}
};


/**
 * Initializes the sync_event, creating the mutex and wait condition. It will
 * initially be unsignalled.
 *
 * Like calling CreateEvent() in Windows.
 *
 * @param[in] evt A pointer to a pre-made sync_event to initialize
 * @retval true if the sync event is created
 * @retval false if the sync event is not created
 */
bool
sync_event_construct(
	sync_event* evt
);


/**
 * Destroys the sync_event contents, causing the mutex and wait condition to be
 * erased. If the event was allocated dynamically, it must still be freed.
 *
 * Like calling CloseHandle(evt) in Windows.
 *
 * @param[in] evt A pointer to a previously constructed sync_event
 * @retval true if the event is destroyed
 * @retval false if the event is invalid, or was failed to be destroyed
 */
bool
sync_event_destroy(
	sync_event* evt
);


/**
 * Waits for the supplied sync_event to be signalled (through sync_event_set),
 * before resuming with the current threads execution.
 *
 * Just like calling WaitForSingleObject(evt, INFINITE) on an Event in Windows.
 *
 * Returns immediately if event is a nullptr, and logs an error.
 *
 * @param[in] evt The sync_event to wait on
 */
void
sync_event_wait(
	sync_event* evt
);


/**
 * Signals the supplied sync_event; a thread waiting for this signal will block
 * until this is received,
 *
 * Just like calling SetEvent(evt) on an Event in Windows.
 *
 * Returns immediately if event is a nullptr, and logs an error.
 *
 * @param[in] evt The sync_event to signal
 */
void
sync_event_set(
	sync_event* evt
);



END_NAMESPACE

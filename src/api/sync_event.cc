
/**
 * @file	sync_event.cc
 * @author	James Warren
 * @copyright	James Warren, 2013
 * @license	Zlib (see license.txt or http://opensource.org/licenses/Zlib)
 */



#include "sync_event.h"		// prototypes
#include "Terminal.h"


BEGIN_NAMESPACE(APP_NAMESPACE)


bool
sync_event_construct(
	sync_event* evt
)
{
	if ( evt == nullptr )
	{
		std::cerr << fg_red << "The event passed in to " << __FUNCTION__ << " was a nullptr\n";
		return false;
	}

	if ( pthread_mutex_init(&evt->mutex, nullptr) != 0 )
	{
		std::cerr << fg_red << "pthread_mutex_init() failed; errno " << errno << "\n";
		return false;
	}
	if ( pthread_cond_init(&evt->condition, nullptr) != 0 )
	{
		std::cerr << fg_red << "pthread_cond_init() failed; errno " << errno << "\n";

		pthread_mutex_destroy(&evt->mutex);
		return false;
	}

	evt->flag = 0;

	return true;
}



bool
sync_event_destroy(
	sync_event* evt
)
{
    int32_t	rc;

	if ( evt == nullptr )
	{
		std::cerr << fg_red << "The event passed in to " << __FUNCTION__ << " was a nullptr\n";
		return false;
	}

	/* if another thread is blocking, errno is set to EBUSY */
	if ( pthread_mutex_destroy(&evt->mutex) != 0 )
	{
		std::cerr << fg_red << "pthread_mutex_destroy() failed; errno " << errno << "\n";
		return false;
	}

	/* should never fail since pthread_mutex_destroy succeeded */
	if (( rc = pthread_cond_destroy(&evt->condition)) != 0 )
	{
		std::cerr << fg_red << "pthread_cond_destroy() failed; errno " << rc << "\n";
		return false;
	}

	evt->flag = 0;

	return true;
}



void
sync_event_set(
	sync_event* evt
)
{
	if ( evt == nullptr )
	{
		std::cerr << fg_red << "The event passed in to " << __FUNCTION__ << " was a nullptr\n";
		return;
	}

	pthread_mutex_lock(&evt->mutex);

	/* change to signalled state */
	evt->flag = 1;

	/* release the mutex before signalling the condition */
	pthread_mutex_unlock(&evt->mutex);

	/* signal the condition */
	pthread_cond_signal(&evt->condition);
}



void
sync_event_wait(
	sync_event *evt
)
{
	if ( evt == nullptr )
	{
		std::cerr << fg_red << "The event passed in to " << __FUNCTION__ << " was a nullptr\n";
		return;
	}

	pthread_mutex_lock(&evt->mutex);

	/* wait for the conditional signal (yes, this function releases the
	 * mutex so another thread can 'set' it..) */
	while ( !evt->flag )
		pthread_cond_wait(&evt->condition, &evt->mutex);

	/* signalled and executing - remove the flag/reset the event */
	evt->flag = 0;

	pthread_mutex_unlock(&evt->mutex);
}


END_NAMESPACE

#pragma once

/**
 * @file	IrcPool.h
 * @author	James Warren
 * @brief	Memory pools for the IRC objects
 */



#include <mutex>			// C++11 mutex
#include <stack>			// stl stack
#include <cstdio>			// FILE, fprintf

#include <api/Log.h>
#include <api/utils.h>			// BUILD_STRING, strlcpy
#include <api/Allocator.h>		// memory allocation macros

#include "IrcChannel.h"
#include "IrcConnection.h"
#include "IrcNetwork.h"
#include "IrcUser.h"


#if defined(USING_MEMORY_DEBUGGING)


BEGIN_NAMESPACE(APP_NAMESPACE)


#define IRCPOOL_GET_DECL		const char* function, const char* file, const uint32_t line
#define IRCPOOL_GET_ARGS		__func__, __FILE__, __LINE__


/**
 * A struct that is stored at the end of an Object when acquired from a Pool, if
 * USING_MEMORY_DEBUGGING is defined.
 *
 * For simplicity, this uses the same style as Allocator, with the needless bits
 * removed.
 *
 * @struct pool_object_meminfo
 */
struct pool_object_meminfo
{
	/** A pointer to the allocated object */
	void*		object;
	/** The file this object was created in */
	char		file[MEM_MAX_FILENAME_LENGTH + 1];
	/** The function name this object was created in */
	char		function[MEM_MAX_FUNCTION_LENGTH + 1];
	/** The line in the file this object was created in */
	uint32_t	line;
};

END_NAMESPACE

#else

#define IRCPOOL_GET_DECL
#define IRCPOOL_GET_ARGS

#endif	// USING_MEMORY_DEBUGGING


#if defined(_WIN32) && defined(GetObject)
	// combat windows pollution
#	undef GetObject
#endif



BEGIN_NAMESPACE(APP_NAMESPACE)


/**
 * A simple memory pool
 *
 * @class ObjectPool
 */
template <class T>
class ObjectPool
{
private:
	NO_CLASS_ASSIGNMENT(ObjectPool);
	NO_CLASS_COPY(ObjectPool);

	/** The address of the allocated memory */
	void*		_pool;

	/**  */
	std::vector<std::shared_ptr<T>>	_objects;

	std::vector<std::shared_ptr<T>>	_delete_later;
	
	/** A stack of addresses used from the _pool */
	std::stack<T*>	_used_list;
	/** A stack of addresses available to use from the _pool */
	std::stack<T*>	_free_list;

	/** Stats; the number of objects requested via Get() in total */
	uint32_t	_get_count;
	/** Stats; the maximum number of objects requested and still used via
	 * Get() at a single point in time */
	uint32_t	_max_alive;

	/** The lock for making modifications per pool */
	std::mutex	_mutex;



	/**
	* Same style as the OutputMemoryInfo() from Allocator, where this was
	* adapted from originally.
	*/
	void
	OutputMemoryInfo(
		const char* out_filename
	)
	{
			FILE*	out_file;
			bool	close_file = true;

			if (( out_file = _fsopen(out_filename, "wb", _SH_DENYWR)) == nullptr )
			{
				out_file = stdout;
				close_file = false;
			}

			_mutex.lock();

			fprintf(out_file,
				"# Details\n"
				"Object Type...: %s\n"
				"Object Size...: %lu\n"
				"\n"
				"# Code Stats\n"
				"Created.......: %lu\n"
				"Requested.....: %u\n"
				"Released......: %lu\n"
				"Unreleased....: %lu\n"
				"Most Alive....: %u\n"
				"\n"
				"##################\n",
				typeid(T).name(),
				sizeof(T),
				// free_list size is valid only if there's no leaks!
				(_used_list.size() + _free_list.size()),
				_get_count,
				(_get_count - _used_list.size()),
				_used_list.size(),
				_max_alive
				);

#if 0	// Code Removed: no manual reference tracking done anymore, using std::shared_ptr
#if defined(USING_MEMORY_DEBUGGING)
			for ( auto meminfo : _infovect )
			{
				fprintf(out_file, "%s object at " PRINT_POINTER " (%s @ %s:%u, %lu bytes)\n",
					destruction ? "Unfreed" : "Allocated",
					(uintptr_t)meminfo.object,
					meminfo.function,
					meminfo.file,
					meminfo.line,
					sizeof(T));
			}
#endif	// USING_MEMORY_DEBUGGING
#endif

		_mutex.unlock();

		if ( close_file )
		{
			fclose(out_file);
		}
	}



	void
	ReallyFreeObject(
		std::shared_ptr<T> object
	)
	{
		LOG(ELogLevel::Debug) << "Object " << object << " given back to the pool\n";

		// add the address back to the free pool
		_free_list.push(object.get());
		// pop it off the used list
		_used_list.pop();
		// and remove our reference to it
		_objects.erase(std::find(_objects.begin(), _objects.end(), object));
	}


public:

	ObjectPool() : _pool(nullptr), _get_count(0)
	{
	}
	~ObjectPool()
	{
		/* Mandatory output; allows us to see the amount of allocations
		 * per object, useful for future optimization - such as a more
		 * suitable amount for GrandAlloc */
		OutputMemoryInfo(BUILD_STRING(typeid(T).name(), ".log").c_str());
		// ensure we're not leaving anything allocated
		TotalErase();
	}


	/**
	 * Returns the list of allocated objects currently live
	 */
	std::vector<std::shared_ptr<T>>&
	Allocated()
	{
		return _objects;
	}


	/**
	 * Returns the supplied object to the pool, and calls the destructor.
	 */
	bool
	FreeObject(
		std::shared_ptr<T> object
	)
	{
		// validate it came from our pool?

		_mutex.lock();

		/* If the object is still referenced by more than just this
		 * current function (and our _object list), we cannot add it 
		 * back to the free list, as it could be reallocated and screw 
		 * things up. 
		 * At the same time, we can't keep checking references in caller
		 * code or waiting for objects to die, so we add it to the 
		 * 'delete-later' list, so it can be reclaimed later.
		 */
		if ( object.use_count() > 2 )
		{
			// still referenced; reattempt delete later
			_delete_later.push_back(object);
			return true;
		}
		
		/* release the actual resources it's using and provide the
		 * pointers back to the lists */
		ReallyFreeObject(object);

		_mutex.unlock();

		// IrcObject destructor called on this functions return!!
		return true;
	}


	/**
	 * Returns the next available pointer
	 */
	std::shared_ptr<T>
	GetObject(
		IRCPOOL_GET_DECL
	)
	{
		_mutex.lock();

		/* seems an opportune time to check the delete_later
		 * list and actually delete the old objects */
		{
		}

		if ( _free_list.empty() )
		{
			/** @todo implement GrandAlloc dynamic expansion */
			/* argh.. we've breached our allocation limit - we need to do a
			 * fresh set to bring us up - the policy is 50% of the current
			 * max (i.e. GrandAlloc(10) means if we breach, an extra 5
			 * elements will be added - a second breach adds 8, and so on) */
			//uint32_t	additional = _used_list.size() / 2;
		}
		
		T*	object = _free_list.top();

		// remove one of the free elements from the stack
		_free_list.pop();

#if 0	// Code Removed: no manual reference tracking done anymore, using std::shared_ptr
#if defined(USING_MEMORY_DEBUGGING)
		pool_object_meminfo	info;

#	if IS_DEBUG_BUILD && IS_VISUAL_STUDIO
		// we don't want the full path information that VS sets in debug mode
		file = (strrchr(file, PATH_CHAR) + 1);
#	endif

		info.object	= object;
		info.line	= line;
		strlcpy(info.file, file, sizeof(info.file));
		strlcpy(info.function, function, sizeof(info.function));

		_infovect.push_back(info);
#endif
#endif

		// push our created object onto the used elements
		_used_list.push(object);

		// update stats
		_get_count++;
		if ( _used_list.size() > _max_alive )
			_max_alive = _used_list.size();

		LOG(ELogLevel::Debug) << "Object " << object << " acquired from the pool\n";

		std::shared_ptr<T>	ptr;
		/* assign the object pointer as shared; no constructors yet! The
		 * caller, factory, will utilize placement new for this task */
		ptr.reset(object);
		/* store this pointer until requested to free it; by default,
		 * nothing retains a pointer to one of these objects, so it will
		 * hit a reference count of 0 upon scope exit where created */
		_objects.push_back(ptr);

		_mutex.unlock();

		return ptr;
	}


	/**
	 * Allocates the requested number of objects.
	 *
	 * The goal is not to call this at all, while the game is active; if the
	 * requested amount exceeds the current available, a large memory 
	 * allocation will be performed, potentially causing contention.
	 *
	 * Designed to be called once, based on the game specs and options,
	 * until the game finishes, where all objects are released.
	 *
	 * If USING_MEMORY_DEBUGGING, space will be reserved at the end of each
	 * object (essentially no-mans land), which will store the details of
	 * the caller of Get()
	 *
	 * @param num_objects The number of Objects of type T to allocate
	 * @return Returns true if all of the objects were allocated; if num_objects
	 * is 0, or allocation fails, the function returns false
	 */
	bool
	GrandAlloc(
		uint32_t num_objects
	)
	{
		T*		offset;
		uint32_t	to_alloc = sizeof(T) * num_objects;

		if ( num_objects == 0 )
			return false;

		_mutex.lock();

#if defined(USING_MEMORY_DEBUGGING)
		// since we know how many objects there will be
		_infovect.reserve(num_objects);
#endif

		// one 'huge' heap allocation
		_pool = MALLOC(to_alloc);

		if ( _pool == nullptr )
		{
			/** @todo throw nullptr or runtime_error on malloc failure? */
			_mutex.unlock();
			return false;
		}

		offset = static_cast<T*>(_pool);

		for ( uint32_t i = 0; i < num_objects; i++ )
		{
			// push each object offset into the free list
			_free_list.push(offset);
			offset++;
		}

		_mutex.unlock();
		return true;
	}



	/**
	 * Used for wiping out any existing pool items (which yes, will cause a
	 * crash if these are still referenced and valid objects); generally
	 * shouldn't be called out of the destructor, but this leaves the option
	 * open for cleanup functions.
	 */
	void
	TotalErase()
	{
		uint32_t	cnt = 0;

		/* recursive mutex, so we can have the same thread locking
		 * multiple times without a problem */
		_mutex.lock();

		while ( !_used_list.empty() )
		{
			for ( auto ptr : _objects )
			{
				if ( ptr.get() == _used_list.top() )
					FreeObject(ptr);
			}
		}

		// wait until all shared pointers are actually destroyed
		while ( !_delete_later.empty() )
		{
			SLEEP_MILLISECONDS(100);
			cnt++;

			if ( cnt > 50 )
			{
				throw std::runtime_error("Potential deadlock; shared pointers still referenced, not releasing");
			}
		}


		if ( !_objects.empty() )
		{
			throw std::runtime_error("Objects still exist on pool deletion");
		}


		if ( _pool != nullptr )
		{
			FREE(_pool);
			_pool = nullptr;
		}

		_mutex.unlock();
	}
};




/**
 * Resides within the IRC Engine and provides access to pre-allocated memory
 * usable for well-known/common objects.
 *
 * Reusable for specific implementations, with updating of the relevant types.
 *
 * @class Pool
 */
class IrcPool
{
	/** we are created on the stack in IrcEngine::Pools() */
	friend class IrcEngine;
private:
	NO_CLASS_ASSIGNMENT(IrcPool);
	NO_CLASS_COPY(IrcPool);

	// private constructor; we want one instance that is controlled.
	IrcPool();


	ObjectPool<IrcUser>		_users;
	ObjectPool<IrcChannel>		_channels;
	ObjectPool<IrcConnection>	_connections;
	ObjectPool<IrcNetwork>		_networks;
	
public:
	~IrcPool();


	ObjectPool<IrcChannel>* const
	IrcChannels()
	{ 
		return &_channels;
	}

	ObjectPool<IrcConnection>* const
	IrcConnections()
	{
		return &_connections;
	}
	
	ObjectPool<IrcNetwork>* const
	IrcNetworks()
	{
		return &_networks;
	}

	ObjectPool<IrcUser>* const
	IrcUsers()
	{ 
		return &_users; 
	}


	std::shared_ptr<IrcChannel>
	GetChannel(
		const uint32_t connection_id,
		const char* channel_name
	);

	std::shared_ptr<IrcConnection>
	GetConnection(
		const uint32_t connection_id
	);

	std::shared_ptr<IrcNetwork>
	GetNetwork(
		const char* network_name
	);

	std::shared_ptr<IrcUser>
	GetUser(
		const uint32_t connection_id,
		const char* channel_name,
		const char* user_name
	);
};



END_NAMESPACE

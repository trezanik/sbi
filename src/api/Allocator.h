#pragma once

/**
 * @file	src/api/Allocator.h
 * @author	James Warren
 * @brief	Memory Allocation tracking for the application
 */



#if defined(USING_MEMORY_DEBUGGING)

#include <mutex>			// C++11 mutex, locking
#include <vector>			// storage container

#include "Runtime.h"			// technical dependency for macros
#include "char_helper.h"		// defintions, text handling



BEGIN_NAMESPACE(APP_NAMESPACE)


// forward declarations
class Object;

// required definitions
#define MEM_LEAK_LOG_NAME		"memdynamic.log"
#define MEM_MAX_FILENAME_LENGTH		31
#define MEM_MAX_FUNCTION_LENGTH		31




/**
 * This structure is added to the end of an allocated block of memory, when
 * USING_MEMORY_DEBUGGING is defined.
 *
 * This ensures no data has been written beyond the boundary of an allocated bit
 * of memory, resulting in heap/stack corruption.
 *
 * @struct memblock_footer
 */
struct memblock_footer
{
	unsigned	magic;		/**< footer magic number */
};


/**
 * This structure is added at the start of a block of allocated memory, when
 * USING_MEMORY_DEBUGGING is defined.
 *
 * @struct memblock_header
 */
struct memblock_header
{
	/**
	 * The header magic number is used to detect if an operation on memory has
	 * written into this structure. Corrupt headers usually mean something else
	 * has written into the block - an exception being if an operation has
	 * stepped too far back, to the extent of overwriting the magic number */
	unsigned		magic;

	/** A pointer to this memory blocks footer */
	memblock_footer*	footer;

	/** A pointer to the object that allocated the memory. Requires manual
	 * calling, may aid in debugging certain scenarios but not needed in the
	 * majority of cases. Left here as an example of associating a block of
	 * memory with a calling Class. */
	const Object*		owner;

	/**
	 * The file this memory block was created in; is not allocated dynamically,
	 * and so is bound by the MEM_MAX_FILENAME_LENGTH definition.
	 */
	CHARTYPE	file[MEM_MAX_FILENAME_LENGTH + 1];

	/** The function name this memory block was created in; like the file
	 * member, is not allocated dynamically, and so is bound by the
	 * MEM_MAX_FUNCTION_LENGTH definition */
	CHARTYPE	function[MEM_MAX_FUNCTION_LENGTH + 1];

	/** The line in the file this memory block was created in */
	uint32_t	line;

	/** The size, in bytes, the original request desired */
	uint32_t	requested_size;

	/** The size, in bytes, of the total allocation (header+data+footer) */
	uint32_t	real_size;
};




/**
 * Memory-specific error codes. Used only as the return value from CheckBlock().
 *
 * @enum E_MEMORY_ERROR
 */
enum E_MEMORY_ERROR
{
	EC_NoError = 0,
	EC_NoMemoryBlock,
	EC_CorruptHeader,
	EC_CorruptFooter,
	EC_SizeMismatch
};



/**
 * Resides within the application runtime and tracks all memory allocations and
 * frees, ensuring data is not corrupt or the memory leaked.
 *
 * Will not exist if USING_MEMORY_DEBUGGING is not defined.
 *
 * Most interaction with this class should be done with the special macros:
 * - MALLOC
 * - REALLOC
 * - FREE
 * These handle providing the line number, file, and function, and will define
 * to the real malloc/realloc/free automatically if not debugging memory.
 *
 * Naturally, anything allocated/freed by new/delete/malloc/realloc/free will
 * not be tracked.
 *
 * @class Allocator
 */
class SBI_API Allocator
{
	/* we are created on the stack in Runtime::Memory(); needs access to
	 * call our private constructor */
	friend class Runtime;
private:
	// no class assignment or copy
	NO_CLASS_ASSIGNMENT(Allocator);
	NO_CLASS_COPY(Allocator);

	// private constructor; we want one instance that is controlled.
	Allocator();


	uint32_t	_allocs;		/**< The amount of times new has been called successfully */
	uint32_t	_frees;			/**< The amount of times delete has been called successfully */
	uint32_t	_current_allocated;	/**< Currently allocated amount of bytes */
	uint32_t	_total_allocated;	/**< The total amount of allocated bytes */

	/**
	 * The lock for making modifications to _memblocks. Mutable to allow
	 * constness for retrieval functions, recursive for multi-locking.
	 */
	mutable std::recursive_mutex	_mutex;

	/**
	 * Performance benchmarks have shown that a vector is fastest when the
	 * size of the element is known, for insert, remove, and search!
	 * http://www.baptiste-wicht.com/2012/12/cpp-benchmark-vector-list-deque/
	 *
	 * The small data type (a pointer) is most beneficial here.
	 */
	std::vector<memblock_header*>	_memblocks;


	/**
	 * Checks a block of memory, ensuring both the header and footer are not
	 * corrupt, and the rest of the block matches the original requestors
	 * specifications.
	 *
	 * Define DISABLE_MEMORY_CHECK_TO_STDOUT to prevent stage-by-stage
	 * output of the checking process.
	 *
	 * @param[in] memory_block The block of memory to check
	 * @retval E_MEMORY_ERROR The relevant memory error code as to the block
	 * status; should hopefully always be EC_NoError
	 */
	E_MEMORY_ERROR
	CheckBlock(
		memblock_header* memory_block
	) const;



	/**
	 * Validates a tracked bit of memory; if no pointer is supplied, so 
	 * nullptr is passed in, every tracked block currently present in the
	 * class is validated.
	 *
	 * If present, the memory passed in must be at the pointer to the memory
	 * returned by the TrackedAlloc/TrackedRealloc caller.
	 *
	 * Internally calls CheckBlock().
	 *
	 * @param[in] memory A pointer to the memory to validate
	 * @retval true if the memory is not corrupt and usable
	 * @retval false if the memory failed one of the checks
	 */
	bool
	ValidateMemory(
		void* memory
	) const;


public:
	~Allocator();


	/**
	 * Called only in the destructor, but available for calling manually if
	 * desired; will always output the memory stats for the application run,
	 * but will also write out the information on any unfreed memory.
	 *
	 * Outputs to MEM_LEAK_LOG_NAME, but if it's not writable, it is printed
	 * to stderr instead.
	 */
	void
	OutputMemoryInfo();


	/**
	 * Tracked version of malloc - use the MALLOC macro to call this, as it
	 * will setup the parameters for you, barring the num_bytes.
	 *
	 * This function will dynamically alter the requested amount in order to
	 * leave space for a header and footer; the client code does not need to
	 * handle, or be aware, of this fact.
	 *
	 * @param[in] num_bytes The number of bytes to allocate
	 * @param[in] file The file this method was called in
	 * @param[in] function The function this method was called in
	 * @param[in] line The line number in the file this method was called in
	 * @param[in] owner (Optional) The owner of the memory block
	 * @return A pointer to the allocated memory, or a nullptr if the
	 * allocation failed.
	 */
	void*
	TrackedAlloc(
		const uint32_t num_bytes,
		const char* file,
		const char* function,
		const uint32_t line,
		const Object* owner = nullptr
	);


	/**
	 * Tracked version of free - use the FREE macro to call this, for
	 * consistency and potential future changes.
	 *
	 * In compliance with the C standard, if memory is a nullptr, this
	 * function performs no action. No action is also performed if a
	 * memory validation check fails (i.e. the supplied block is corrupt or
	 * otherwise invalid), as doing so could crash the application, and
	 * miss logging the information.
	 *
	 * @param[in] memory A pointer to the memory previously allocated by
	 * TrackedAlloc (which should be called via MALLOC)
	 */
	void
	TrackedFree(
		void* memory
	);


	/**
	 * Tracked version of realloc - use the REALLOC macro to call this.
	 *
	 * In compliance with the C standard, if memory_block is a nullptr,
	 * the end result is the same as calling TrackedAlloc (i.e. malloc). 
	 * The same applies if new_num_bytes is 0 - TrackedFree (i.e. free) 
	 * will be called on the memory_block.
	 *
	 * No validation is performed on the original block of memory, and
	 * unlike the real realloc, we call TrackedAlloc regardless of size
	 * differences and other parameters. The previous memory is then moved
	 * into this newly allocated block, and the original freed.
	 *
	 * As a result, a TrackedRealloc guarantees that the returned pointer
	 * will never be the same as the one passed in.
	 *
	 * @param[in] memory The pointer to memory returned by TrackedAlloc()
	 * @param[in] new_num_bytes The new number of bytes to allocate
	 * @param[in] file The file this method was called in
	 * @param[in] function The function this method was called in
	 * @param[in] line The line number in the file this method was called in
	 * @param[in] owner (Optional) The owner of the memory block
	 * @retval nullptr if the function fails due to invalid parameters, or
	 * the call to realloc fails
	 * @return A pointer to the usable block of memory allocated
	 */
	void*
	TrackedRealloc(
		void* memory,
		const uint32_t new_num_bytes,
		const char* file,
		const char* function,
		const uint32_t line,
		const Object* owner = nullptr
	);
};



/* I like C-styles memory allocation. It's something you can rely on to always
 * behave in a certain way, no surprises, and not wrought with stupid limits on
 * what you can and can't do - ala C++.
 *
 * Only thing I don't like is the need to cast everything returned.
 *
 * While I can just redefine 'malloc' and 'free', depending on inclusion order
 * this may pick up more than we desire, and potentially break some other code
 * that we don't maintain. Style guidelines also decree a macro is ALL CAPS!
 */

/** Macro to create tracked memory */
	#define MALLOC(size)		runtime.Memory()->TrackedAlloc(size, __FILE__, __FUNCTION__, __LINE__)

/** Macro to reallocate tracked memory */
	#define REALLOC(ptr, size)	runtime.Memory()->TrackedRealloc(ptr, size, __FILE__, __FUNCTION__, __LINE__)

/** Macro to delete tracked memory */
	#define FREE(varname)		runtime.Memory()->TrackedFree(varname)

/** The amount of bytes (limit) printed to output under the Data field */
	#define MEM_OUTPUT_LIMIT	1024


END_NAMESPACE

#else
	/* if we're here, we don't want to debug the memory, so just set the
	 * macros to call the original, non-hooked functions. */


//#pragma message ("Using original memory allocators")
#	define MALLOC(size)		malloc(size)
#	define REALLOC(ptr, size)	realloc(ptr, size)
#	define FREE(varname)		free(varname)



#endif	// USING_MEMORY_DEBUGGING




/**
 * @file	Allocator.cc
 * @author	James Warren
 * @copyright	James Warren, 2013
 * @license	Zlib (see LICENCE or http://opensource.org/licenses/Zlib)
 */



#include "Allocator.h"			// prototypes, definitions

// This file is only valid if USING_MEMORY_DEBUGGING is enabled
#if defined(USING_MEMORY_DEBUGGING)

#if !defined(DISABLE_MEMORY_CHECK_TO_STDOUT) || !defined(DISABLE_MEMORY_OP_TO_STDOUT)
#	include <cstdio>		// printf
#endif

#if defined(__linux__) || defined(BSD)
#	include <string.h>		// memcmp, memset, memmove
#	include <algorithm>		// std::find
#endif

#include "char_helper.h"		// text handling
#include "utils.h"			// strlcpy



BEGIN_NAMESPACE(APP_NAMESPACE)


// definitions that can be replaced or implemented elsewhere
#define MAX_LEN_GENERIC		250

// Magic values, assigned and checked with memory operations
#define MEM_HEADER_MAGIC	0xCAFEFACE
#define MEM_FOOTER_MAGIC	0xDEADBEEF
// Memory-fill values, used before and after alloc/free
#define MEM_ON_INIT		0x0F
#define MEM_AFTER_FREE		0xFF


#define block_offset_header(real_mem)           \
		(memblock_header*)((uint8_t*)real_mem - sizeof(memblock_header))
#define block_offset_realmem(memblock)          \
		(void*)((uint8_t*)memblock + sizeof(memblock_header))
#define block_offset_footer(memblock, num_bytes)\
		(memblock_footer*)((uint8_t*)memblock + (sizeof(memblock_header) + num_bytes))
#define HEADER_FOOTER_SIZE			\
		(sizeof(memblock_header) + sizeof(memblock_footer))


// usage as variables allow them to be easily inserted into memcmp's
const unsigned	mem_header_magic = MEM_HEADER_MAGIC;
const unsigned	mem_footer_magic = MEM_FOOTER_MAGIC;



Allocator::Allocator() :
	_allocs(0), _frees(0), _current_allocated(0), _total_allocated(0)
{
	/* reserve 100 internal allocations. Will breach, but should cause the
	 * expansions to be 200, 400, 800, etc. */
	_memblocks.reserve(100);
}



Allocator::~Allocator()
{
	if ( !_memblocks.empty() )
	{
		printf("Memory Leak Detected\n\nCheck '%s' for details\n",
		       MEM_LEAK_LOG_NAME);
	}

	OutputMemoryInfo();
}



E_MEMORY_ERROR
Allocator::CheckBlock(
	memblock_header* memory_block
) const
{
	uint32_t	block_size = 0;

	if ( memory_block == nullptr )
		goto null_block;

#if !defined(DISABLE_MEMORY_CHECK_TO_STDOUT)
	printf("\tChecking Memory Block %p..\n"), memory_block);
	printf("\t\tGlobal Header magic number is %lu bytes in size :: %#x\n"), sizeof(mem_header_magic), mem_header_magic);
	printf("\t\tGlobal Footer magic number is %lu bytes in size :: %#x\n"), sizeof(mem_footer_magic), mem_footer_magic);
#endif

	if ( memcmp(&memory_block->magic,
		&mem_header_magic,
		sizeof(mem_header_magic)) != 0 )
	{
		/* memory_block magic number has been modified; block contents
		 * are unstable */
		goto corrupt_header;
	}

#if !defined(DISABLE_MEMORY_CHECK_TO_STDOUT)
	printf("\t\tHas a valid header (%lu bytes, %#x)...\n"),
		sizeof(memory_block->magic), memory_block->magic);
	printf("\t\tHeader Info:: %u (%u requested) bytes, line %u in %s\n"),
		memory_block->real_size, memory_block->requested_size,
		memory_block->line, memory_block->file);
#endif

	if ( memory_block->footer == nullptr
	    || memcmp(&memory_block->footer->magic, &mem_footer_magic,	sizeof(mem_footer_magic)) != 0 )
	{
		/* memory_block footer magic has been modified - as the header
		 * is not corrupt, we can retrieve the allocation info safely */
		goto corrupt_footer;
	}

#if !defined(DISABLE_MEMORY_CHECK_TO_STDOUT)
	printf("\t\tHas a valid footer (%lu bytes, %#x)...\n",
		sizeof(memory_block->footer->magic),
		memory_block->footer->magic);
#endif

	// calculate the size requested by removing the header + footer
	block_size = ((uint8_t*)memory_block->footer) - ((uint8_t*)memory_block + sizeof(memblock_header));

#if !defined(DISABLE_MEMORY_CHECK_TO_STDOUT)
	printf("\t\tCalculated block_size is %u bytes\n"), block_size);
#endif

	if ( memory_block->requested_size != block_size )
	{
		/* The size stored by the memory_block header is different from
		 * that calculated (remember we've already validated the magic
		 * numbers) */
		goto invalid_size;
	}

#if !defined(DISABLE_MEMORY_CHECK_TO_STDOUT)
	printf("\t\tHas a valid app_mem size (%u bytes)\n\t\tValidated!\n"),
	       block_size);
#endif

	// memory_block is as expected

	return E_MEMORY_ERROR::EC_NoError;

null_block:
	// Optional: Raise error
	return E_MEMORY_ERROR::EC_NoMemoryBlock;
corrupt_header:
	// Optional: Raise error
	return E_MEMORY_ERROR::EC_CorruptHeader;
corrupt_footer:
	// Optional: Raise error
	return E_MEMORY_ERROR::EC_CorruptFooter;
invalid_size:
	// Optional: Raise error
	return E_MEMORY_ERROR::EC_SizeMismatch;
}



void
Allocator::OutputMemoryInfo()
{
	FILE*		leak_file;
	bool		close_file = true;
	uint32_t	i = 0;
	E_MEMORY_ERROR	result;
	// we don't store/track the user-requested amounts, only the real
	uint32_t	requested_alloc;
	uint32_t	requested_unfreed;

	if ( (leak_file = _fsopen(MEM_LEAK_LOG_NAME, "wb", _SH_DENYWR)) == nullptr )
	{
		leak_file = stdout;
		close_file = false;
	}

	/* Remove memory block sizes, multiplied by the number of allocations,
	 * which is taken away from the total amount allocated. */
	requested_alloc		= _total_allocated - (HEADER_FOOTER_SIZE * _allocs);
	/* Remove memory block sizes, multiplied by the number of pending frees,
	 * which is to be taken away from the current amount still allocated. */
	requested_unfreed	= _current_allocated - (HEADER_FOOTER_SIZE * (_allocs - _frees));

	fprintf(leak_file,
		"# Details\n"
		"Header+Footer Size......: %lu\n"
		"\n"
		"# Code Stats\n"
		"Allocations.............: %u\n"
		"Frees...................: %u\n"
		"Pending Frees...........: %u\n"
		"\n"
		"# Totals, Real\n"
		"Bytes Allocated.........: %u\n"
		"Unfreed Bytes...........: %u\n"
		"\n"
		"# Totals, Requested\n"
		"Bytes Allocated.........: %u\n"
		"Unfreed Bytes...........: %u\n"
		"\n"
		"##################\n"
		"  Unfreed Blocks  \n",
		HEADER_FOOTER_SIZE,
		_allocs, _frees, (_allocs - _frees),
		_total_allocated, _current_allocated,
		requested_alloc, requested_unfreed
		);


	for ( auto block_ptr : _memblocks )
	{
		i++;

		fprintf(leak_file,
			"##################\n"
			"%u)\n"
			"Block...: " PRINT_POINTER
			"\n",
			i, (uintptr_t)block_ptr
		);

		result = CheckBlock(block_ptr);
		switch ( result )
		{
		case E_MEMORY_ERROR::EC_NoMemoryBlock:
			{
				fprintf(leak_file,
					"Error...: Block Pointer was NULL\n"
				);
				break;
			}
		case E_MEMORY_ERROR::EC_CorruptFooter:
			{
				fprintf(leak_file,
					"Error...: Corrupt Footer\n"
				);
				break;
			}
		case E_MEMORY_ERROR::EC_CorruptHeader:
			{
				fprintf(leak_file,
					"Error...: Corrupt Header\n"
				);
				break;
			}
		case E_MEMORY_ERROR::EC_SizeMismatch:
			{
				fprintf(leak_file,
					"Error...: Size Mismatch (%u actual bytes)\n",
					block_ptr->requested_size
				);
				break;
			}
		default:
			break;
		}

		/* can't fall through in switch, so have to do a secondary check
		 * as we can't print data that's corrupt or a nullptr */
		if ( result != EC_NoMemoryBlock && result != EC_CorruptHeader )
		{
			fprintf(leak_file,
				"Size....: %u\n"
				"Function: %s\n"
				"File....: %s\n"
				"Line....: %u\n"
				"Owner...: %p\n",
				block_ptr->requested_size,
				block_ptr->function,
				block_ptr->file,
				block_ptr->line,
				block_ptr->owner
			);
			fprintf(leak_file, "Data....: ");
			for ( uint32_t j = 0;
				j < block_ptr->requested_size && j < MEM_OUTPUT_LIMIT;
				j++ )
			{
				/** @todo causes warning 'cast to pointer from integer of different size' */
				fprintf(leak_file,
					"%02x ",
					block_offset_realmem(block_ptr)[j]);
			}
			fprintf(leak_file, "\n");
		}
	}

	if ( close_file )
		fclose(leak_file);

	/* try to free whatever we didn't during runtime; if any of these are
	 * screwed (heap corruption) then this will probably trigger a crash */
	while ( _memblocks.size() > 0 )
	{
		free(_memblocks.back());
		_memblocks.pop_back();
	}
}



void*
Allocator::TrackedAlloc(
	const uint32_t num_bytes,
	const char* file,
	const char* function,
	const uint32_t line,
	const Object* owner
)
{
	memblock_header*	mem_block = nullptr;
	memblock_footer*	mem_footer = nullptr;
	void*			mem_return = nullptr;
	const char*		p = nullptr;
	uint32_t		patched_alloc = 0;	// num_bytes + memblocks

	// allocate the requested amount, plus the size of the header & footer memblocks
	patched_alloc = num_bytes + HEADER_FOOTER_SIZE;

	// the actual, real, physical allocation of memory
	mem_block = (memblock_header*)malloc(patched_alloc);

	if ( mem_block == nullptr )
		goto alloc_failure;

	// initialize the value for the new memory
	memset(mem_block, MEM_ON_INIT, patched_alloc);

	// we don't want the full path information that compilers set
	if ( (p = strrchr(file, PATH_CHAR)) != nullptr)
		file = p++;

	// calculate the offsets of the return memory and the footer
	mem_return = block_offset_realmem(mem_block);
	mem_footer = block_offset_footer(mem_block, num_bytes);

	// prepare the structure internals
	mem_footer->magic	= mem_footer_magic;
	mem_block->footer	= mem_footer;
	mem_block->magic	= mem_header_magic;
	mem_block->owner	= owner;
	mem_block->line		= line;
	mem_block->real_size		= patched_alloc;
	mem_block->requested_size	= num_bytes;

#if !defined(_WIN32)
	strlcpy(mem_block->file, file, sizeof(mem_block->file));
	strlcpy(mem_block->function, function, sizeof(mem_block->function));
#else
	mb_to_utf8(mem_block->file, file, _countof(mem_block->file));
	mb_to_utf8(mem_block->function, function, _countof(mem_block->function));
#endif

	/* lock this class, only 1 thread to update sensitive internals at a
	 * time - lock for as little time as possible! */
	_mutex.lock();

	// update the stats, using patched values
	_allocs++;
	_current_allocated += patched_alloc;
	_total_allocated += patched_alloc;
	// append it to the vector
	_memblocks.push_back(mem_block);

	// unlock the allocator, other threads can now allocate from this class
	_mutex.unlock();

	return mem_return;

alloc_failure:
	return nullptr;
}



void
Allocator::TrackedFree(
	void* memory
)
{
	memblock_header*	mem_block = nullptr;

	// as per the C standard, if it's a nullptr, do nothing
	if ( memory == nullptr )
		return;

	if ( !ValidateMemory(memory) )
		return;

	mem_block = block_offset_header(memory);

#if !defined(DISABLE_MEMORY_OP_TO_STDOUT)
	printf( "free [%s (%u bytes) line %u]\n")
		"\tBlock: %p | Usable Block: %p\n"),
		mem_block->file, mem_block->requested_size, mem_block->line,
		mem_block, memory);
#endif

	// stop other class modifications
	_mutex.lock();

	// remove the mem_block from the vector
	_memblocks.erase(std::find(_memblocks.begin(), _memblocks.end(), mem_block));
	// update the context stats
	_frees++;
	_current_allocated -= (mem_block->real_size);

	// we're done with the class internals, open it up again
	_mutex.unlock();

	// fill the app-allocated memory (highlights use after free)
	memset(mem_block, MEM_AFTER_FREE, mem_block->real_size);
	// perform the actual freeing of memory, including our header + footer
	free(mem_block);

	return;
}



void*
Allocator::TrackedRealloc(
	void* memory,
	const uint32_t new_num_bytes,
	const char* file,
	const char* function,
	const uint32_t line,
	const Object* owner
)
{
	memblock_header*	mem_block = nullptr;
	void*			mem_return = nullptr;

	if ( memory == nullptr )
	{
		// if the memory is NULL, call malloc [ISO C]
		return TrackedAlloc(new_num_bytes, file, function, line, owner);
	}

	if ( new_num_bytes == 0 )
	{
		/* if new_num_bytes is 0 and the memory is not null, call
		 * free() [ISO C] */
		TrackedFree(memory);
		/* whether it works or fails, the memory is still unusable, so
		 * return a nullptr */
		return nullptr;
	}

	_mutex.lock();

#if !defined(DISABLE_MEMORY_OP_TO_STDOUT)
	// since we call TrackedAlloc, make the log info accurate
	printf( "realloc [%s (%u bytes) line %u]\n")
		"\tTo be allocated in the following malloc; using memory at: %p\n"),
		file, new_num_bytes, line,
		memory);
#endif

	mem_return = (void*)TrackedAlloc(new_num_bytes, file, function, line, owner);

	if ( mem_return != nullptr )
	{
		mem_block = block_offset_header(memory);

		// move the original data into the new allocation
		mem_block->requested_size < new_num_bytes ?
			memmove(mem_return, memory, new_num_bytes) :
			memmove(mem_return, memory, mem_block->requested_size);

		// free the original block
		TrackedFree(memory);
	}

	_mutex.unlock();

	return mem_return;
}



bool
Allocator::ValidateMemory(
	void* memory
) const
{
	bool	ret = true;

	_mutex.lock();

	// if no pointer was specified, check the entire list
	if ( memory == nullptr )
	{
		for ( auto block : _memblocks )
		{
			// bail if a block is invalid
			if ( CheckBlock(block) != E_MEMORY_ERROR::EC_NoError )
			{
				ret = false;
				break;
			}
		}
	}
	else
	{
		memblock_header*	mem_block = block_offset_header(memory);

		ret = (CheckBlock(mem_block) == E_MEMORY_ERROR::EC_NoError);
	}

	_mutex.unlock();

	return ret;
}


END_NAMESPACE


#endif	// USING_MEMORY_DEBUGGING

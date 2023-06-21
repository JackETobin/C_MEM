#ifndef C_MEM_H
#define C_MEM_H

#include <stdlib.h>

#ifndef _TYPES_
#define _TYPES_

typedef char				int8;
typedef unsigned char			uint8;

typedef short				int16;
typedef unsigned short			uint16;

typedef int				int32;
typedef unsigned int			uint32;

typedef long long			int64;
typedef unsigned long long		uint64;

#endif
#ifndef NULL
#define NULL ((void*)0)
#endif

#define _SUCCESS_ 1
#define _FAILURE_ 0

#define _SIZE_VP_ sizeof(void*)
#define _SIZE_KB_ 1024
#define _SIZE_MB_ (1024 * 1024)
#define _SIZE_INT8_ sizeof(char);
#define _SIZE_INT64_ sizeof(int64)

#define _POOL_TRACK_CHUNK_ 24

#define _BLOCK_FREE_ (1 << 0)

#define _LOWER_FREE_ 1
#define _UPPER_FREE_ 2
#define _BOTH_FREE_ 3
#define _NONE_FREE_ 0

typedef void* handle;
typedef void** handle_container;

typedef struct _pool
{
	uint32 lenMB;
	handle data;
	handle next;
	handle voids;
} *pool_handle;

typedef struct _pool_tracker
{
	handle_container		data;
	uint32				length;
	uint32				size;
}pool_tracker;

struct _sv_props
{
	uint64				voidSize;
	handle				voidPtr;
};

typedef struct _pool_void_properties
{
	uint32				numVoids;
	uint64				totalSpace;
	struct _sv_props		largestVoid;
}void_info, *void_info_handle;

typedef struct _block
{
	handle_container		self;
	uint8				flags;
	pool_handle			pool;	
	handle				elements;
} *block_handle;
#define _SIZE_BLOCK_MIN_ sizeof(struct _block) + (4 * _SIZE_VP_)

pool_handle OpenPool(uint32 sizeMB);
uint8 ClosePool(pool_handle* pool);
uint8 ConsolidatePool(pool_handle pool);
void_info PoolVoidInfo(pool_handle pool);

block_handle BuildBlock(pool_handle pool, uint32 numElements, uint64 elementSizeByes);
uint8 FreeBlock(block_handle* block);

uint64 BlockSize(block_handle block);
uint32 NumElements(block_handle block);
uint64 ElementSize(block_handle block);
handle GetElementAt(block_handle block, uint32 element);

uint8 Push(block_handle block, handle data);
uint8 Pop(block_handle block);
uint8 Set(block_handle block, handle data, uint32 position);

uint8 ZeroElement(block_handle block, uint32 position);
uint8 ZeroBlock(block_handle block);

uint8 RotateFirst(block_handle block);
uint8 RotateLast(block_handle block);

#endif

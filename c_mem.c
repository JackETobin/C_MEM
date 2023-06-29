#include "c_mem.h"

static pool_tracker _all_pools;

void __pool__clear()
{
	if (!_all_pools.data)
		return;
	for (uint32 i = 0; i < _all_pools.size; i++)
		if (*((handle_container)_all_pools.data + i))
		{
			free(*((handle_container)_all_pools.data + i));
			*((handle_container)_all_pools.data + i) = NULL;
		}
	free(_all_pools.data);
}

void __pool__poolremove()
{
	if (!_all_pools.data)
		return;
	for (uint32 i = 0; i < _all_pools.size; i++)
		if (!*((handle_container)_all_pools.data + i))
		{
			while (i++ < _all_pools.size)
				*((handle_container)_all_pools.data + i - 1) = *((handle_container)_all_pools.data + i);
			_all_pools.size--;
		}
	return;
}

uint8 __pool__close(pool_handle pool)
{
	if (!_all_pools.data)
		return  _FAILURE_;
	for (uint32 i = 0; i < _all_pools.size; i++)
		if (*((handle_container)_all_pools.data + i) == pool)
		{
			free(*((handle_container)_all_pools.data + i));
			*((handle_container)_all_pools.data + i) = NULL;
			__pool__poolremove();
			return _SUCCESS_;
		}
	return _FAILURE_;
}

void __pool__set()
{
	atexit(__pool__clear);
	_all_pools.data = malloc(_POOL_TRACK_CHUNK_);
	if (!_all_pools.data)
		return;
	_all_pools.length = _POOL_TRACK_CHUNK_;
	_all_pools.size = 0;
}

void __pool__expand()
{
	handle_container temp = malloc(_all_pools.length + _POOL_TRACK_CHUNK_);
	if (!temp)
		return;
	for (int32 i = 0; i < _all_pools.size; i++)
		*((handle_container)temp + i) = *((handle_container)_all_pools.data + i);
	free(_all_pools.data);
	_all_pools.data = temp;
	_all_pools.length += _POOL_TRACK_CHUNK_;
}

void __pool__track(handle poolHandle)
{
	if (!_all_pools.data)
		__pool__set();
	if ((_all_pools.size * _SIZE_VP_) >= _all_pools.length)
		__pool__expand();

	*((handle_container)_all_pools.data + _all_pools.size) = poolHandle;
	_all_pools.size++;
}

block_handle __pool__voidcombine(handle_container currentBlock, uint32 voidIndex)
{
	pool_handle onPool = ((block_handle)currentBlock)->pool;
	handle_container freeBlock = *((handle_container)onPool->voids + voidIndex);
	if (currentBlock < freeBlock)
	{
		*currentBlock = *freeBlock;
		*(handle_container)*currentBlock = currentBlock;
		*((handle_container)onPool->voids + voidIndex) = currentBlock;
		return (block_handle)currentBlock;
	}
	else
	{
		*freeBlock = *currentBlock;
		*(handle_container)*freeBlock = freeBlock;
		return (block_handle)freeBlock;
	}
	return NULL;
}

uint8 __pool__voidadd(block_handle block)
{
	pool_handle onPool = block->pool;
	onPool->voids -= _SIZE_VP_;
	*(handle_container)onPool->voids = *(handle_container)block->self;
	return _SUCCESS_;
}

uint32 __pool__voidindex(block_handle block)
{
	uint32 i = 0;
	handle_container voidHandle = ((pool_handle)(block->pool))->voids;

	while (((handle)voidHandle + i) < (handle)(((pool_handle)(block->pool))->next))
	{
		if (*((handle_container)voidHandle + i) == (handle_container)block)
			return i;
		i++;
	}
	return _FAILURE_;
}

uint8 __pool__voidremove(block_handle block)
{
	uint32 voidIndex = __pool__voidindex(block);
	pool_handle onPool = block->pool;

	uint32 i = voidIndex + 1;
	while (i-- > 1)
		*((handle_container)onPool->voids + i) = *((handle_container)onPool->voids + (i - 1));
	onPool->voids += _SIZE_VP_;
	return _SUCCESS_;
}

uint8 __pool__voidgenerator(handle_container voidHandle)
{
	uint8 flags = 0;
	handle currentHandle;
	
	if ((handle)voidHandle < *voidHandle) currentHandle = voidHandle;
	else currentHandle = *voidHandle;
	pool_handle onPool = ((block_handle)currentHandle)->pool;

	block_handle previousHandle = *((handle_container)currentHandle - 1);
	if((handle)previousHandle > (handle)onPool && (handle)previousHandle != (handle)(onPool->voids)) flags |= (previousHandle->flags);
	block_handle nextHandle = (*(handle_container)currentHandle) + _SIZE_VP_;
	if((handle)nextHandle < (handle)(onPool->voids)) flags |= (nextHandle->flags << 1);

	switch (flags)
	{
	case _LOWER_FREE_:
		__pool__voidcombine((handle_container)currentHandle, __pool__voidindex(previousHandle));
		break;
	case _UPPER_FREE_:
		__pool__voidcombine((handle_container)currentHandle, __pool__voidindex(nextHandle));
		break;
	case _BOTH_FREE_:
		__pool__voidremove(__pool__voidcombine((handle_container)previousHandle, __pool__voidindex(nextHandle)));
		break;
	case _NONE_FREE_:
		__pool__voidadd((block_handle)currentHandle);
		break;
	default:
		return _FAILURE_;
	}
	return _SUCCESS_;
}

uint64 __pool__voidsize(handle_container voidHandle)
{
	int64 voidSize = *(handle_container)voidHandle - (handle)voidHandle;
	if (voidSize < 0) voidSize *= -1;
	return (uint64)voidSize + _SIZE_VP_;
}

struct _sv_props __pool__voidprops(handle_container voidHandle)
{
	struct _sv_props voidProperties = { 0 };
	if (!*voidHandle)
		return voidProperties;
	voidProperties.voidPtr = voidHandle;
	voidProperties.voidSize = __pool__voidsize(voidHandle);

	return voidProperties;
}

uint8 __pool__voidshrink(struct _sv_props voidProps, uint64 size)
{
	if (size >= voidProps.voidSize)
		return _FAILURE_;
	
	uint32 voidIndex = __pool__voidindex(voidProps.voidPtr);
	pool_handle onPool = ((block_handle)voidProps.voidPtr)->pool;
	handle voidBegin = *((handle_container)onPool->voids + voidIndex);
	handle voidEnd = *(handle_container)*((handle_container)onPool->voids + voidIndex);

	voidBegin += size;
	((block_handle)(voidBegin))->flags |= _BLOCK_FREE_;
	((block_handle)(voidBegin))->pool = onPool;
	*((handle_container)onPool->voids + voidIndex) = voidBegin;
	*(handle_container)* ((handle_container)onPool->voids + voidIndex) = voidEnd;

	return _SUCCESS_;
}

block_handle __pool__voidfill(pool_handle pool, uint64 size)
{
	struct _sv_props voidProps = { 0 };
	for (uint32 i = 0; ((handle_container)pool->voids + i) < (handle_container)pool->next; i++)
	{
		voidProps = __pool__voidprops(*((handle_container)pool->voids + i));
		if (voidProps.voidSize >= size)
			break;
		voidProps.voidPtr = 0;
	}
	if (voidProps.voidSize == size || voidProps.voidSize - size < _SIZE_BLOCK_MIN_)
		__pool__voidremove((block_handle)voidProps.voidPtr);
	else
		__pool__voidshrink(voidProps, size);
	return voidProps.voidPtr;
}

handle __pool__getfirstvoid(pool_handle pool)
{
	handle firstVoid = *(handle_container)pool->voids;
	uint32 numVoids = (pool->next - pool->voids) / _SIZE_VP_;

	for (uint32 i = 1; i < numVoids; i++)
		if (*((handle_container)pool->voids + i) < firstVoid)
			firstVoid = *((handle_container)pool->voids + i);

	return firstVoid;
}

handle __pool__shiftblock(handle writeTarget, block_handle_container blockAddrContainer, uint32 numElements, uint64 sizeElements)
{
	handle blockAddr = *blockAddrContainer;
	uint64 blockSize = BlockSize((block_handle)blockAddr);

	uint64 dataBegin = ((block_handle)blockAddr)->elements - blockAddr - 1;
	uint64 dataEnd = dataBegin + (numElements * sizeElements);

	*(handle_container)((block_handle)blockAddr)->container = writeTarget;
	while (dataBegin++ < dataEnd)
		*((uint8*)writeTarget + dataBegin) = *((uint8*)blockAddr + dataBegin);

	*blockAddrContainer = writeTarget;
	writeTarget += blockSize;
	return writeTarget;
}

uint8 __pool__rebuildblock(block_handle block, uint32 numElements, uint64 sizeElements)
{
	handle selfPtr = (handle)&block->elements + ((numElements)*_SIZE_VP_) + (numElements * sizeElements);
	handle dataPtr = (handle)&block->elements + ((numElements)*_SIZE_VP_);
	if (selfPtr == &block->elements)
		selfPtr += _SIZE_VP_;

	block->self = selfPtr;
	*(handle_container)block->self = &block->self;
	block->elements = dataPtr;
	for (uint32 i = 1; i < numElements; i++)
		*(&block->elements + i) = dataPtr + (i * sizeElements);

	return _SUCCESS_;
}

pool_handle OpenPool(uint32 sizeMB)
{
	uint64 sizeBytes = _SIZE_MB_ * sizeMB;
	uint64 zeroIterator = sizeBytes / _SIZE_INT64_;

	handle poolHandle = malloc(sizeBytes);
	if (!poolHandle)
		return poolHandle;

	while (zeroIterator--)
		*((uint64*)poolHandle + zeroIterator) = 0;
	
	struct _pool temp;
	temp.lenMB = sizeMB;
	temp.data = poolHandle + sizeof(struct _pool);
	temp.voids = temp.next = poolHandle + sizeBytes - _SIZE_VP_;
	*(handle_container)temp.next = (handle)temp.data;
	
	*(pool_handle)poolHandle = temp;
	__pool__track(poolHandle);
	
	return (pool_handle)poolHandle;
}

uint8 ClosePool(pool_handle* pool)
{
	__pool__close(*pool);
	*pool = NULL;
	return 0;
}

uint8 ConsolidatePool(pool_handle pool)
{
	uint64 sizeElements;
	uint32 numElements;
	block_handle nextBlock;

	handle writeTarget = __pool__getfirstvoid(pool);
	block_handle currentBlock = (block_handle)((*(handle_container)writeTarget) + _SIZE_VP_);
	
	while ((handle)currentBlock < *((handle_container)pool->next))
	{
		nextBlock = (block_handle)((*(handle_container)currentBlock) + _SIZE_VP_);
		while ((handle)nextBlock < *((handle_container)pool->next) && nextBlock->flags & _BLOCK_FREE_)
			nextBlock = (block_handle)((*(handle_container)nextBlock) + _SIZE_VP_);
		
		sizeElements = ElementSize(currentBlock);
		numElements = NumElements(currentBlock);

		writeTarget = __pool__shiftblock(writeTarget, (block_handle_container)&currentBlock, numElements, sizeElements);
		__pool__rebuildblock(currentBlock, numElements, sizeElements);
		currentBlock = nextBlock;
	}
	*((handle_container)pool->next) = writeTarget;
	pool->voids = pool->next;
	return _SUCCESS_;
}

void_info PoolVoidInfo(pool_handle pool)
{
	uint64 totalSpace = 0;
	void_info voidInfo = { 0 };	
	struct _sv_props largestVoid = { 0 };

	if (pool->voids == pool->next)
		return voidInfo;

	voidInfo.numVoids = (pool->next - pool->voids) / _SIZE_VP_;
	for (uint32 i = 0; i < voidInfo.numVoids; i++)
	{
		totalSpace += __pool__voidsize(*((handle_container)pool->voids + i));
		if (__pool__voidsize(*((handle_container)pool->voids + i)) > largestVoid.voidSize)
			largestVoid = __pool__voidprops(*((handle_container)pool->voids + i));
	}
	voidInfo.totalSpace = totalSpace;
	voidInfo.largestVoid = largestVoid;

	return voidInfo;
}

block_handle BuildBlock(handle containerHandle, pool_handle pool, uint32 numElements, uint64 eleSizeBytes)
{
	uint64 blockSize = sizeof(struct _block) + (numElements * _SIZE_VP_) + (numElements * eleSizeBytes);
	if (!numElements)
		blockSize = sizeof(struct _block) + _SIZE_VP_;

	block_handle temp = __pool__voidfill(pool, blockSize);
	if (!temp)
	{
		temp = *(handle_container)pool->next;
		*(handle_container)pool->next += blockSize;
	}
	handle selfPtr = (handle)&temp->elements + ((numElements) * _SIZE_VP_) + (numElements * eleSizeBytes);
	handle dataPtr = (handle)&temp->elements + ((numElements) * _SIZE_VP_);
	if (selfPtr == &temp->elements)
		selfPtr += _SIZE_VP_;

	temp->self = selfPtr;
	*(handle_container)temp->self = &temp->self;
	temp->flags = 0;
	temp->pool = pool;
	temp->container = containerHandle;
	temp->elements = dataPtr;
	for (uint32 i = 1; i < numElements; i++)
		*(&temp->elements + i) = dataPtr + (i * eleSizeBytes);

	return temp;
}

uint8 FreeBlock(block_handle* block)
{
	__pool__voidgenerator((handle_container)*block);
	(*block)->flags |= _BLOCK_FREE_;
	*block = NULL;
	return 0;
}

uint64 BlockSize(block_handle block)
{
	uint64 blockSize = (handle)block->self - (handle)block + _SIZE_VP_;
	return blockSize;
}

uint32 NumElements(block_handle block)
{
	uint32 numElements = ((void*)block->elements - (void*)(&block->elements)) / _SIZE_VP_;
	return numElements;
}

uint64 ElementSize(block_handle block)
{
	if (!NumElements(block))
		return (uint64)0;
	uint64 elementSize = ((handle)block->self - (handle)block->elements) / NumElements(block);
	return elementSize;
}

handle GetElementAt(block_handle block, uint32 element)
{
	if (element < 0 || element > (NumElements(block) - 1))
		return NULL;
	handle elementHandle = *(handle_container)(&block->elements + element);
	return elementHandle;
}

handle GetData(block_handle block)
{
	return block->elements;
}

uint8 Push(block_handle block, handle data)
{
	uint64 elementSize = ElementSize(block);
	handle shiftStart = **(handle_container*)block->self;

	uint64 i = 0;
	while ((shiftStart - elementSize - i++ > block->elements))
		*((uint8*)shiftStart - i) = *((uint8*)shiftStart - elementSize - i);

	for (i = 0; i < elementSize; i++)
		*((uint8*)block->elements + i) = *((uint8*)data + i);

	return _SUCCESS_;
}

uint8 Pop(block_handle block)
{
	uint64 elementSize = ElementSize(block);
	handle shiftStop = **(handle_container*)block->self;

	uint64 i = 0;
	while ((block->elements + elementSize + i < shiftStop))
	{
		*((uint8*)block->elements + i) = *((uint8*)block->elements + elementSize + i);
		i++;
	}
	i -= elementSize;
	while((uint8*)block->elements + (i++) < (uint8*)shiftStop - 1)
		*((uint8*)block->elements + i) = 0;

	return _SUCCESS_;
}

uint8 Set(block_handle block, handle data, uint32 position)
{
	if (position < 0 || position > (NumElements(block) - 1))
		return _FAILURE_;
	uint64 elementSize = ElementSize(block);
	for (uint64 i = 0; i < elementSize; i++)
		*(uint8*)(*(&block->elements + position) + i) = *((uint8*)data + i);
	return 0;
}

uint8 ZeroElement(block_handle block, uint32 position)
{
	if (position < 0 || position >(NumElements(block) - 1))
		return _FAILURE_;
	uint64 elementSize = ElementSize(block);
	for (uint64 i = 0; i < elementSize; i++)
		*(uint8*)(*(&block->elements + position) + i) = 0;
	return 0;
}

uint8 ZeroBlock(block_handle block)
{
	handle shiftStop = **(handle_container*)block->self;

	uint64 i = 0;
	while ((block->elements + i < shiftStop))
	{
		*((uint8*)block->elements + i) = 0;
		i++;
	}
	return _SUCCESS_;
}

uint8 RotateFirst(block_handle block)
{
	pool_handle onPool = block->pool;
	uint64 elementSize = ElementSize(block);
	if ((onPool->voids - *(handle_container)onPool->next) < elementSize)
		return _FAILURE_;
	
	uint64 i;
	handle shiftStop = **(handle_container*)block->self;

	for (i = 0; i < elementSize; i++)
		*((uint8*)(*(handle_container)onPool->next) + i) = *((uint8*)block->elements + i);

	i = 0;
	while ((block->elements + elementSize + i < shiftStop))
	{
		*((uint8*)block->elements + i) = *((uint8*)block->elements + elementSize + i);
		i++;
	}

	uint64 iter = 0;
	while ((uint8*)block->elements + i < (uint8*)shiftStop - 1)
	{
		*((uint8*)block->elements + i) = *((uint8*)(*(handle_container)onPool->next) + iter);
		i++, iter++;
	}

	for (i = 0; i < elementSize; i++)
		*((uint8*)(*(handle_container)onPool->next) + i) = 0;

	return _SUCCESS_;
}

uint8 RotateLast(block_handle block)
{
	pool_handle onPool = block->pool;
	uint64 elementSize = ElementSize(block);
	if ((onPool->voids - *(handle_container)onPool->next) < elementSize)
		return _FAILURE_;

	uint64 i;
	handle shiftStart = **(handle_container*)block->self;

	for (i = 0; i < elementSize; i++)
		*((uint8*)(*(handle_container)onPool->next) + i) = *((uint8*)shiftStart - elementSize + i);

	i = 0;
	while ((shiftStart - elementSize - i++ > block->elements))
		*((uint8*)shiftStart - i) = *((uint8*)shiftStart - elementSize - i);

	for (i = 0; i < elementSize; i++)
		*((uint8*)block->elements + i) = *((uint8*)(*(handle_container)onPool->next) + i);

	for (i = 0; i < elementSize; i++)
		*((uint8*)(*(handle_container)onPool->next) + i) = 0;

	return _SUCCESS_;
}
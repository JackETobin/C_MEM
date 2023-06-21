# C_MEM
A memory management system that's build around calls to malloc(). This system automatically frees any active memory pools that exist at the end of the program.
-

Currently data allocation is first-fit only since the goal of the project was speed and not necessarily efficiency. If need be, or if anyone asks, I will add best-fit data allication, or any other style of data allocation. As of present there is no defrag algorithem either, and so that takes priority.

I had two objectives while writing this system: speed and simplicity of use. Since I wrote this, I do find it to be easy to use, and so I a a bit biased. If anyone would like to try to use this system and has trouble, do feel free to reach out. I'm happy to answer any questions.




Types
=
**handle**

- void*

**handle_container**

- void**

**pool_handle**

- Handle to a pool structure, is returned by the OpenPool() function, and contains the length of the pool in megabytes, the begining of data, a double pointer to the next available address, and a double pointer to the addresses of all voids. Voids are defined in the description of the PoolVoidInfo() function.

**block_handle**

- Handle to a block that is located in a pool. Contains a double pointer to self, status flags, a pointer to its parent pool, a pointer to the container of the block_handle, and a double pointer to the elements.

**void_info**

- Structure containing information about the voids located on a pool, more about ths in the description of the PoolVoidInfo() function.

**void_info_handle**

- Pointer to the void_info struct.

**pool_tracker**

- A struct containing a list of all active pools, is used to free all pools on exit.

Functionality:
=

pool_handle **OpenPool(uint32 sizeMB)**

- Takes a desired size in megabytes and opens up a memory pool at the specified size, calls malloc to do so. Returns a handle to the pool.

uint8 __ClosePool(pool_handle* pool)__

- Frees the pool associated with the handle fed into the function. Note that the function takes a pointer to the handle, this is so that the handle can be zero'd out after having been freed.
      
uint8 **ConsolidatePool(pool_handle pool)**

- Consolidates blocks such that all blocks are moved toward the front of the pool to remove voids that might exist between the blocks. This function is taking the place of a proper defrag function since the data types are not stored in the block structures, and because there is no reason to sort the blocks for my current application. If anyone who might use this would like to see a version of this that organizes the blocks in a particular way, I'm happy to accommodate.
  
void_info **PoolVoidInfo(pool_handle pool)**

- Query's the input memory pool for information on voids in mememory. Voids are defined as areas inn memory where blocks which have been freed and the space has not yet been reallocated. Will return a void_info struct containing the total number of voids in the pool, the total free space located between blocks, the largest contiguous void, and the address of the largest contiguous void.

block_handle **BuildBlock(pool_handle pool, uint32 numElements, uint64 elementSizeByes)**

- Sets up a block in the desired pool, takes the number of desired elements and the size of the elements. Note that the elements are all the same size, and the element size is NOT the total size of the data block. Returns a handle to the block in memory.

uint8 __FreeBlock(block_handle* block)__

- Takes a block handle and marks the input block as no longer in use. Note that the function takes a pointer to the handle, this is so that the handle can be zero'd out after having been marked as free.

uint64 **BlockSize(block_handle block)**

- Takes a block handle, and returns the total size of the structure in memory in bytes.

uint64 **ElementSize(block_handle block)**

- Takes a block handle and returns the size of the elements of the input block.

uint32 **NumElements(block_handle block)**

- Takes a block handle and returns the number of elements in the structure.

handle **GetElementAt(block_handle block, uint32 element)**

- Takes a block handle and a desired postition, returns a pointer to the data at the desired position. Any input positions less than 0 or greater than or equal to the number of elements in the structure will return a NULL pointer.

uint8 **Push(block_handle block, handle data)**

- Takes a block handle and a pointer to the data that you wish to be added into the block. Pushes all other elements back by 1 and adds the desired data to the first element in the structure. Note that this is not a true push, DATA WILL FALL OFF THE END OF THE STRUCTURE. As of right now blocks are not resizeable, and so the last element in the block will be overwritten in the event that new data is pushed in.

uint8 **Pop(block_handle block)**

- Takes a block handle. Removes the first element data and shifts all elements up by 1 element, then zeros out the last element.

uint8 **Set(block_handle block, handle data, uint32 position)**

- Takes a block handle, a pointer to the data that you wish to be added into the block, and the position at which you'd like the data to be written. Note that if the position is less than 0 or greater than or equal to the number of elements in the structure, nothing will be written.

uint8 **ZeroElement(block_handle block, uint32 position)**

- Takes a block handle and the position which is to be zero'd out.

uint8 **ZeroBlock(block_handle block)**

- Takes a block handle and zeros out all elements, overwriting any present data.

uint8 **RotateFirst(block_handle block)**

- Takes a block handle and rotates the data forward, such that the first element data ends up in the last element, and all other elements are pushed up by 1.

uint8 **RotateLast(block_handle block)**

- Takes a block handle and rotates the data backward, such that the last element data ends up in the first element, and all other elements are pushed back by 1.

Plans for the future:
=

At some point I'd like to add multi-threading, however, since this system was written with a specific purpose in mind, I will first implement this system and make sure that it works the way that it's expected to work. 

As this project continues to expand and become more practical I will add items to the addition and possible addition lists.

**Future Additions:**
- ~~Indexing from zero instead of 1~~
- ~~Defragmentation function~~
- Code annotations

**Possible Additions:**
- Resizeable blocks
- Resizeable pools

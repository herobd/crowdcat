/* block.h */
/*
	Template classes Block and DBlock
	Implement adding and deleting items of the same type in blocks.

	If there there are many items then using Block or DBlock
	is more efficient than using 'new' and 'delete' both in terms
	of memory and time since
	(1) On some systems there is some minimum amount of memory
	    that 'new' can allocate (e.g., 64), so if items are
	    small that a lot of memory is wasted.
	(2) 'new' and 'delete' are designed for items of varying size.
	    If all items has the same size, then an algorithm for
	    adding and deleting can be made more efficient.
	(3) All Block and DBlock functions are inline, so there are
	    no extra function calls.

	Differences between Block and DBlock:
	(1) DBlock allows both adding and deleting items,
	    whereas Block allows only adding items.
	(2) Block has an additional operation of scanning
	    items added so far (in the order in which they were added).
	(3) Block allows to allocate several consecutive
	    items at a time, whereas DBlock can add only a single item.

	Note that no constructors or destructors are called for items.

	Example usage for items of type 'MyType':

	///////////////////////////////////////////////////
	#include "block.h"
	#define BLOCK_SIZE 1024
	typedef struct { int a, b; } MyType;
	MyType *ptr, *array[10000];

	...

	Block<MyType> *block = new Block<MyType>(BLOCK_SIZE);

	// adding items
	for (int i=0; i<sizeof(array); i++)
	{
		ptr = block -> New();
		ptr -> a = ptr -> b = rand();
	}

	// reading items
	for (ptr=block->ScanFirst(); ptr; ptr=block->ScanNext())
	{
		printf("%d %d\n", ptr->a, ptr->b);
	}

	delete block;

	...

	DBlock<MyType> *dblock = new DBlock<MyType>(BLOCK_SIZE);
	
	// adding items
	for (int i=0; i<sizeof(array); i++)
	{
		array[i] = dblock -> New();
	}

	// deleting items
	for (int i=0; i<sizeof(array); i+=2)
	{
		dblock -> Delete(array[i]);
	}

	// adding items
	for (int i=0; i<sizeof(array); i++)
	{
		array[i] = dblock -> New();
	}

	delete dblock;

	///////////////////////////////////////////////////

	Note that DBlock deletes items by marking them as
	empty (i.e., by adding them to the list of free items),
	so that this memory could be used for subsequently
	added items. Thus, at each moment the memory allocated
	is determined by the maximum number of items allocated
	simultaneously at earlier moments. All memory is
	deallocated only when the destructor is called.
*/

#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <stdlib.h>

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

template <class Type> class Block
{
public:
	/* Constructor. Arguments are the block size and
	   (optionally) the pointer to the function which
	   will be called if allocation failed; the message
	   passed to this function is "Not enough memory!" */
	Block(int size, void (*err_function)(char *) = NULL); 

	/* Destructor. Deallocates all items added so far */
	~Block() ; 

	/* Allocates 'num' consecutive items; returns pointer
	   to the first item. 'num' cannot be greater than the
	   block size since items must fit in one block */
	Type *New(int num = 1);

	/* Returns the first item (or NULL, if no items were added) */
	Type *ScanFirst();

	/* Returns the next item (or NULL, if all items have been read)
	   Can be called only if previous ScanFirst() or ScanNext()
	   call returned not NULL. */
	Type *ScanNext();

	/* Marks all elements as empty */
	void Reset();

/***********************************************************************/

private:

	typedef struct block_st
	{
		Type					*current, *last;
		struct block_st			*next;
		Type					data[1];
	} block;

	int		block_size;
	block	*first;
	block	*last;

	block	*scan_current_block;
	Type	*scan_current_data;

	void	(*error_function)(char *);
};

/***********************************************************************/
/***********************************************************************/
/***********************************************************************/

template <class Type> class DBlock
{
public:
	/* Constructor. Arguments are the block size and
	   (optionally) the pointer to the function which
	   will be called if allocation failed; the message
	   passed to this function is "Not enough memory!" */
	DBlock(int size, void (*err_function)(char *) = NULL);

	/* Destructor. Deallocates all items added so far */
	~DBlock(); 

	/* Allocates one item */
	Type *New();

	/* Deletes an item allocated previously */
	void Delete(Type *t);

/***********************************************************************/

private:

	typedef union block_item_st
	{
		Type			t;
		block_item_st	*next_free;
	} block_item;

	typedef struct block_st
	{
		struct block_st			*next;
		block_item				data[1];
	} block;

	int			block_size;
	block		*first;
	block_item	*first_free;

	void	(*error_function)(char *);
};


#endif


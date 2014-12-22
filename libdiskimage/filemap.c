
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "filemap.h"


/* Statically cached page size. */
static int pagesize = -1;

struct filemap {
	void *pointer;
	size_t length;
	size_t padding_start;
};

/*
 * Initializes the static pagesize variable.
 */
static void
init_pagesize()
{
	pagesize = getpagesize();
}

/* 
 * Helper method that page aligns the specified range.
 */
static void
align(size_t *offset, size_t *length)
{
	size_t padding_start, padding_end;
	size_t end;

	/* If this is the first call, pagesize is not yet initialized. */
	if (pagesize == -1)
		init_pagesize();

	/* Calculate the padding needed to page align the range. */
	end = *offset + *length;
	padding_start = *offset%pagesize;
	padding_end = (end + pagesize - 1)%pagesize;

	/* Update the offset and length to the page aligned range. */
	*offset = *offset - padding_start;
	*length = *length + padding_start + padding_end;
}

/*
 * Memory maps the requested file range into memory. The returned filemap
 * object must be destroyed using filemap_destroy when it is no longer needed.
 * Handles page aligning the range. While not strictly needed on FreeBSD, it 
 * is needed for POSIX compliance and when running in valgrind.
 */
struct filemap *
filemap_create(int fd, size_t offset, size_t length)
{
	void *ptr;
	size_t original_offset = offset;

	/* Allocate memory for the filemap. */
	struct filemap *result = malloc(sizeof(struct filemap));

	/* Make the range page aligned. */
	align(&offset, &length);

	/* Create the memory map. It will be unmapped in filemap_destroy. */
	ptr = mmap(NULL, length, PROT_READ, 0, fd, offset);

	/* Save all the information needed in the filemap object. */
	result->padding_start = original_offset - offset;
	result->pointer = ptr + result->padding_start;
	result->length = length;

	return result;
}

/*
 * Destroys the filemap object and unmaps the memory. Any pointer
 * that has been returned by filemap_pointer is invalid after calling
 * this function.
 */
void
filemap_destroy(struct filemap **map) {
	munmap((*map)->pointer - (*map)->padding_start, (*map)->length);
	free(*map);
	*map = NULL;
}

/*
 * Returns the pointer to the memory mapped by this filemap.
 */
void *
filemap_pointer(struct filemap *map) {
	return map->pointer;
}

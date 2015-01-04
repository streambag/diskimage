
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#include "filemap.h"
#include "diskimage.h"
#include "log.h"


/* Statically cached page size. */
static int pagesize = -1;

/* Details about the mapped memory region. */
struct filemap_internal {
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
LDI_ERROR
filemap_create(int fd, size_t offset, size_t length, struct filemap **map, struct logger logger)
{
	void *ptr;
	size_t original_offset = offset;

	/* Allocate memory for the filemap. */
	*map = malloc(sizeof(struct filemap));

    (*map)->internal = malloc(sizeof(struct filemap_internal));

	/* Make the range page aligned. */
	align(&offset, &length);

	/* Create the memory map. It will be unmapped in filemap_destroy. */
	ptr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

	if (ptr == MAP_FAILED) {
		/* We failed to create a map. */
		LOG_ERROR(logger, "Failed to map memory: %i\n", errno);
		/* Clean up */
		free(*map);
		*map = NULL;
		return LDI_ERR_UNKNOWN;
	}

	/* Save all the information needed in the filemap object. */
	(*map)->internal->padding_start = original_offset - offset;
	(*map)->pointer = ptr + (*map)->internal->padding_start;
	(*map)->internal->length = length;

	return LDI_ERR_NOERROR;
}

/*
 * Destroys the filemap object and unmaps the memory. Any pointer
 * that has been returned by filemap_pointer is invalid after calling
 * this function.
 */
void
filemap_destroy(struct filemap **map) {
	munmap((*map)->pointer - (*map)->internal->padding_start, (*map)->internal->length);
    free((*map)->internal);
	free(*map);
	*map = NULL;
}


#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "filemap.h"


static int pagesize = -1;

struct filemap {
	void *pointer;
	size_t length;
	size_t padding_start;
};

static void
init_pagesize()
{
	pagesize = getpagesize();

}

static void
align(size_t *offset, size_t *length)
{
	if (pagesize == -1)
		init_pagesize();

	size_t padding_start, padding_end;
	size_t end;

	end = *offset + *length;
	padding_start = *offset%pagesize;
	padding_end = (end + pagesize - 1)%pagesize;

	*offset = *offset - padding_start;
	*length = *length + padding_start + padding_end;
}

struct filemap *
filemap_create(int fd, size_t offset, size_t length)
{
	void *ptr;
	size_t original_offset = offset;
	struct filemap *result = malloc(sizeof(struct filemap));

	align(&offset, &length);

	ptr = mmap(NULL, length, PROT_READ, 0, fd, offset);
	result->padding_start = original_offset - offset;

	result->pointer = ptr + result->padding_start;
	result->length = length;

	return result;
}

void
filemap_destroy(struct filemap **map) {
	munmap((*map)->pointer - (*map)->padding_start, (*map)->length);
	free(*map);
	*map = NULL;
}


void *
filemap_pointer(struct filemap *map) {
	return map->pointer;
}

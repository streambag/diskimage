
#ifndef FILEMAP_H
#define FILEMAP_H

#include "diskimage.h"

struct filemap_internal;

/* Represents a filemap object, used to map chunks of a file into memory. */
struct filemap {
    char *pointer;
    struct filemap_internal *internal;
};

/*
 * Memory maps the requested file range into memory. The returned filemap
 * object must be destroyed using filemap_destroy when it is no longer needed.
 * Handles page aligning the range. While not strictly needed on FreeBSD, it 
 * is needed for POSIX compliance and when running in valgrind.
 */
LDI_ERROR filemap_create(int fd, size_t offset, size_t length, struct filemap **map, struct logger logger);

/*
 * Destroys the filemap object and unmaps the memory. Any pointer
 * that has been returned by filemap_pointer is invalid after calling
 * this function.
 */
void filemap_destroy(struct filemap **map);

#endif /* FILEMAP_H */

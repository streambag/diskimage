
#ifndef FILEINTERFACE_H
#define FILEINTERFACE_H

#include "diskimage.h"
#include "filemap.h"

/*
 * A struct representing interface that is used to open files.
 */
struct fileinterface;

/*
 * A struct representing a file. This is used for all file operations
 * in the library.
 */
struct file;

/*
 * Creates a new file interface.
 */
void fileinterface_create(struct fileinterface **fi);

/*
 * Frees the file interface and zeros the pointer.
 */
void fileinterface_destroy(struct fileinterface **fi);

/*
 * Opens a file with the given path.
 */
void file_open(struct fileinterface *fi, char *path, struct file **file);

/*
 * Closes the given file and sets the pointer to zero.
 */
void file_close(struct file **f);

/*
 * Returns the current size of the given file.
 */
size_t file_getsize(struct file *f);

/*
 * Changes the size of the given file.
 */
void file_setsize(struct file *f, size_t newsize);

/*
 * Returns a filemap struct with a chunk of the file mapped to memory.
 */
LDI_ERROR file_getmap(struct file *f, size_t offset, size_t length, struct filemap **map, struct logger logger);

#endif /* FILEINTERFACE_H */

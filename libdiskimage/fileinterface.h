
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
LDI_ERROR	fileinterface_create(struct fileinterface **fi);

/*
 * Frees the file interface and zeros the pointer.
 */
void	fileinterface_destroy(struct fileinterface **fi);

/*
 * Returns a path for the given filename in the given directory.
 */
LDI_ERROR	fileinterface_getpath(struct fileinterface *fi, char *directory, char *filename, char **result);

/*
 * Opens a file with the given path.
 */
LDI_ERROR	file_open(struct fileinterface *fi, char *path, struct file **file);

/*
 * Closes the given file and sets the pointer to zero.
 */
void	file_close(struct file **f);

/*
 * Returns the current size of the given file.
 */
LDI_ERROR	file_getsize(struct file *f, size_t *size);

/*
 * Changes the size of the given file.
 */
LDI_ERROR	file_setsize(struct file *f, size_t newsize);

/*
 * Returns a filemap struct with a chunk of the file mapped to memory.
 */
LDI_ERROR file_getmap(struct file *f, size_t offset, size_t length, struct filemap **map, struct logger logger);

/*
 * Returns the directory of the file.
 */
char   *file_getdirectory(struct file *f);

#endif					/* FILEINTERFACE_H */

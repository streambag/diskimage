
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <errno.h>
#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>

#include "fileinterface.h"
#include "filemap.h"
#include "internal.h"

/*
 * A struct representing interface that is used to open files.
 */
struct fileinterface {
};

/*
 * A struct representing a file. This is used for all file operations
 * in the library.
 */
struct file {
	int	fd;
	char   *path;
};

/*
 * Creates a new file interface.
 */
LDI_ERROR
fileinterface_create(struct fileinterface **fi)
{
	*fi = malloc(sizeof(struct fileinterface));
	if (!*fi) {
		return ERROR(LDI_ERR_NOMEM);
	}

	return NO_ERROR;
}

/*
 * Frees the file interface and zeros the pointer.
 */
void
fileinterface_destroy(struct fileinterface **fi)
{
	free(*fi);
	*fi = NULL;
}

/*
 * Returns a new string with the path for the given filename.
 */
LDI_ERROR
fileinterface_getpath(struct fileinterface *fi, char *directory, char *filename, char **result)
{
	asprintf(result, "%s/%s", directory, filename);
	if (!result) 
		return ERROR(LDI_ERR_NOMEM);

	return NO_ERROR;
}

/*
 * Opens a file with the given path.
 */
LDI_ERROR
file_open(struct fileinterface *fi, char *path, struct file **file)
{
	/* Create the file structure. */
	*file = malloc(sizeof(struct file));
	if (!*file) {
		return ERROR(LDI_ERR_NOMEM);
	}
	(*file)->path = NULL;

	/* Try to open the file. */
	(*file)->fd = open(path, O_RDWR | O_DIRECT | O_FSYNC);
	if ((*file)->fd == -1) {
		file_close(file);
		return ERROR2(LDI_ERR_FILEERROR, errno);
	}

	/* Keep a copy of the path to the file. */
	(*file)->path = strdup(path);
	if (!(*file)->path) {
		file_close(file);
		return ERROR(LDI_ERR_NOMEM);
	}

	return NO_ERROR;
}

/*
 * Closes the given file and sets the pointer to zero.
 */
void
file_close(struct file **f)
{
	if ((*f)->fd >= 0) {
		close((*f)->fd);
	}

	if ((*f)->path != NULL) {
		free((*f)->path);
	}
	free(*f);
	*f = NULL;
}

/*
 * Returns the current size of the given file.
 */
LDI_ERROR
file_getsize(struct file *f, size_t *size)
{
	struct stat sb;
	int res;

	res = fstat(f->fd, &sb);
	if (res == -1) {
		return ERROR2(LDI_ERR_IO, errno);
	}
	*size=sb.st_size;

	return NO_ERROR;
}

/*
 * Writes zeros to the fd at the specified position.
 */
LDI_ERROR
write_zeros(int fd, off_t pos, size_t nbytes)
{
	char *buffer;
	int bytes_written;

	buffer = malloc(512);
	if (!buffer) {
		/* Failed to allocate buffer. */
		return ERROR(LDI_ERR_NOMEM);
	}

	/* Zero out the buffer. */
	bzero(buffer, 512);

	while (nbytes > 0) {
		bytes_written = pwrite(fd, buffer, MIN(512, nbytes), pos);

		if (bytes_written < 0) {
			free(buffer);
			/* Something went wrong. */
			return ERROR(LDI_ERR_IO);
		}
		/* Update pos, nbytes */
		pos += bytes_written;
		nbytes -= bytes_written;
	}

	free(buffer);
	return NO_ERROR;
}

/*
 * Changes the size of the given file.
 */
LDI_ERROR
file_setsize(struct file *f, size_t newsize)
{
	size_t oldsize;
	LDI_ERROR res;
	
	res = file_getsize(f, &oldsize);
	if (IS_ERROR(res)) {
		return res;
	}

	if (newsize > oldsize) {
		/* Fill the new space with zeros. */
		res = write_zeros(f->fd, oldsize, newsize - oldsize);
		if (IS_ERROR(res)) {
			return res;
		}
	} else {
		if (ftruncate(f->fd, newsize) == -1) {
			return ERROR2(LDI_ERR_IO, errno);
		}
	}

	return NO_ERROR;
}

/*
 * Returns a filemap struct with a chunk of the file mapped to memory.
 */
LDI_ERROR
file_getmap(struct file *f, size_t offset, size_t length, struct filemap **map, struct logger logger)
{
	return filemap_create(f->fd, offset, length, map, logger);
}

char   *
file_getdirectory(struct file *f)
{
	char *res;

	/* THREAD SAFETY */
	res = dirname(f->path);
	return strdup(res);
}

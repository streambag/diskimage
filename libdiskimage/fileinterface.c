
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <libgen.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>

#include "fileinterface.h"
#include "filemap.h"

/*
 * A struct representing interface that is used to open files.
 */
struct fileinterface { };

/*
 * A struct representing a file. This is used for all file operations
 * in the library.
 */
struct file {
    int fd;
    char *path;
};

/*
 * Creates a new file interface.
 */
void
fileinterface_create(struct fileinterface **fi)
{
    *fi = malloc(sizeof(struct fileinterface));
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

char *
fileinterface_getpath(struct fileinterface *fi, char *directory, char *filename)
{
    char *res;
    asprintf(&res, "%s/%s", directory, filename);
    return res;
}

/*
 * Opens a file with the given path.
 */
void
file_open(struct fileinterface *fi, char *path, struct file **file)
{
    *file = malloc(sizeof(struct file));
    (*file)->fd = open(path, O_RDWR | O_DIRECT | O_FSYNC);
    (*file)->path = strdup(path);
}

/*
 * Closes the given file and sets the pointer to zero.
 */
void
file_close(struct file **f)
{
    close((*f)->fd);
    free((*f)->path);
    free(*f);
    *f = NULL;
}

/*
 * Returns the current size of the given file.
 */
size_t
file_getsize(struct file *f)
{
    struct stat sb;
    fstat(f->fd, &sb);
    return sb.st_size;
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
    bzero(buffer, 512);

    while (nbytes > 0) {
        bytes_written = pwrite(fd, buffer, MIN(512, nbytes), pos);

        if (bytes_written < 0) {
            /* Something went wrong. */
            return LDI_ERR_IO;
        }
    
        /* Update pos, nbytes */
        pos += bytes_written;
        nbytes -= bytes_written;
    }

    return LDI_ERR_NOERROR;
}

/*
 * Changes the size of the given file.
 */
void 
file_setsize(struct file *f, size_t newsize)
{
    size_t oldsize = file_getsize(f);
    if (newsize > oldsize) {
        /* Fill the new space with zeros. */
        write_zeros(f->fd, oldsize, newsize-oldsize);
    } else {
        ftruncate(f->fd, newsize);
    }
}

/*
 * Returns a filemap struct with a chunk of the file mapped to memory.
 */
LDI_ERROR
file_getmap(struct file *f, size_t offset, size_t length, struct filemap **map, struct logger logger)
{
    return filemap_create(f->fd, offset, length, map, logger);
}

char *
file_getdirectory(struct file *f)
{
    char *res;
    /* THREAD SAFETY */
    res = dirname(f->path);
    return strdup(res);
}

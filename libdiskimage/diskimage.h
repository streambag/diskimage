#ifndef DISKIMAGE_H
#define DISKIMAGE_H

#include <sys/types.h>

typedef enum {
	/* No error */
	LDI_ERR_NOERROR = 0,
	/* Out of memory */
	LDI_ERR_NOMEM,
	/* File format is unknown */
	LDI_ERR_FORMATUNKNOWN,
	/* File format is not supported */
	LDI_ERR_FILENOTSUP,
	/* A read or write outside the valid range */
	LDI_ERR_OUTOFRANGE

} LDI_ERROR;

struct diskimage;

struct diskinfo {
	size_t disksize;
};

LDI_ERROR diskimage_open(char *path, char *format, struct diskimage **di);
void diskimage_destroy(struct diskimage **di);

struct diskinfo diskimage_diskinfo(struct diskimage *di);
LDI_ERROR diskimage_read(struct diskimage *di, char *buf, size_t nbytes, off_t offset);

#endif /* DISKIMAGE_H */

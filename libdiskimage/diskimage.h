#ifndef DISKIMAGE_H
#define DISKIMAGE_H

#include <sys/types.h>

/* Defines all errors that can be returned by functions in libdiskimage */
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
	LDI_ERR_OUTOFRANGE,
	/* An unknown error occured */
	LDI_ERR_UNKNOWN,
} LDI_ERROR;

/* Describes the type (not the format) of a disk */
enum disk_type {
	DISK_TYPE_UNKNOWN = 0,
	DISK_TYPE_FIXED,
	DISK_TYPE_DYNAMIC,
	DISK_TYPE_DIFFERENCING
};

/*
 * The internal state of the diskimage object. Created using diskimage_open.
 * Needs to be passed to all other diskimage_* functions.
 */
struct diskimage;

/*
 * Defines the logger interface. Users of the diskimage library can
 * implement this to get log messages from the library.
 */
struct logger {
	/* The logger function. Set to NULL to disable logging completely. */
	void	(*write)(int level, void *privarg, char *fmt, ...);
	/*
	 * A user defined argument that is always sent to the logger
	 * function. Can be used to track the configured log level.
	 */
	void	*privarg;
};

/* Information about a diskimage */
struct diskinfo {
	size_t disksize;
};

/*
 * Opens the disk image at the supplied path with the given format.
 * Allocates the diskimage structure that is passed to all successive calls.
 * The diskimage structure must be deallocated using diskimage_destroy.
 */
LDI_ERROR diskimage_open(char *path, char *format, struct logger logger, struct diskimage **di);

/*
 * Deallocates and sets the diskimage pointer ot zero.
 */
void diskimage_destroy(struct diskimage **di);

/*
 * Resturns a structure containing disk information.
 */
struct diskinfo diskimage_diskinfo(struct diskimage *di);

/*
 * Reads nbytes of data at offset into the supplied buffer.
 */
LDI_ERROR diskimage_read(struct diskimage *di, char *buf, size_t nbytes, off_t offset);

/*
 * Writes nbytes from the buffer into the diskimage at the specified offset.
 */
LDI_ERROR diskimage_write(struct diskimage *di, char *buf, size_t nbytes, off_t offset);

#endif /* DISKIMAGE_H */


#ifndef VHDFOOTER_H
#define VHDFOOTER_H

#include <sys/types.h>
#include <stdbool.h>

#include "diskimage.h"

/* Represents a VHD file footer. */
struct vhdfooter;

/*
 * Possible errors releated to the VHD file footer.
 */
enum vhdfooter_status {
	VHDFOOTER_OK = 0,
	VHDFOOTER_BADCOOKIE = 1,
	VHDFOOTER_BADCHECKSUM = 2
};

/*
 * Creates a new vhd footer structure by reading from source.
 */
LDI_ERROR
vhdfooter_new(void *source, struct vhdfooter **footer, struct logger logger);

/*
 * Writes the footer to the destination buffer.
 */
LDI_ERROR
vhdfooter_write(struct vhdfooter *footer, void *dest);

/*
 * Deallocates the memory for the footer and sets the pointer to NULL.
 */
void 
vhdfooter_destroy(struct vhdfooter **footer);

/*
 * Returns true if the checksum for the footer is valid.
 */
enum vhdfooter_status
vhdfooter_getstatus(struct vhdfooter *footer);

/*
 * Returns the disk type that this footer represents.
 */
enum disk_type
vhdfooter_getdisktype(struct vhdfooter *footer);

/*
 * Returns the current size of the disk.
 */
long
vhdfooter_disksize(struct vhdfooter *footer);

/*
 * Returns the offset to the beginning of the data.
 */
off_t
vhdfooter_offset(struct vhdfooter *footer);

#endif					/* VHDFOOTER_H */

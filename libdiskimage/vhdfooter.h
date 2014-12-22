
#ifndef VHDFOOTER_H
#define VHDFOOTER_H

#include <sys/types.h>
#include <stdbool.h>

#include "diskimage.h"

/* Represents a VHD file footer. */
struct vhd_footer;

/*
 * Creates a new vhd footer structure by reading from source.
 */
LDI_ERROR
vhd_footer_new(void *source, struct vhd_footer **footer);

/*
 * Deallocates the memory for the footer and sets the pointer to NULL.
 */
void
vhd_footer_destroy(struct vhd_footer **footer);

/*
 * Prints all the values in the footer, for debug purposes.
 */
void
vhd_footer_printf(struct vhd_footer *footer);

/*
 * Returns true if the checksum for the footer is valid.
 */
bool
vhd_footer_isvalid(struct vhd_footer *footer);

/*
 * Returns the disk type that this footer represents.
 */
enum disk_type
vhd_footer_disk_type(struct vhd_footer *footer);

/* 
 * Returns the current size of the disk.
 */
long
vhd_footer_disksize(struct vhd_footer *footer);

/*
 * Returns the offset to the beginning of the data.
 */
off_t
vhd_footer_offset(struct vhd_footer *footer);

#endif /* VHDFOOTER_H */

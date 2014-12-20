
#ifndef VHDFOOTER_H
#define VHDFOOTER_H

#include <sys/types.h>
#include <uuid.h>
#include <stdbool.h>

#include "diskimage.h"

struct vhd_footer;


/* TODO: Move to a common header */
enum disk_type {
	DISK_TYPE_UNKNOWN = 0,
	DISK_TYPE_FIXED,
	DISK_TYPE_DYNAMIC,
	DISK_TYPE_DIFFERENCING
};

LDI_ERROR
vhd_footer_new(void *source, struct vhd_footer **footer);

void
vhd_footer_destroy(struct vhd_footer **footer);

void
vhd_footer_printf(struct vhd_footer *footer);

bool
vhd_footer_isvalid(struct vhd_footer *footer);

enum disk_type
vhd_footer_disk_type(struct vhd_footer *footer);

long
vhd_footer_disksize(struct vhd_footer *footer);

off_t
vhd_footer_offset(struct vhd_footer *footer);



#endif /* VHDFOOTER_H */

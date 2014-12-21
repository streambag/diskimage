
#ifndef VHDFOOTER_H
#define VHDFOOTER_H

#include <sys/types.h>
#include <stdbool.h>

#include "diskimage.h"

struct vhd_footer;

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

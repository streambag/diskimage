
#ifndef _VHDTYPES_H_
#define _VHDTYPES_H_

#include <sys/types.h>

/* The geometry of a disk, as it is stored in the footer. */
struct vhd_disk_geometry {
	uint16_t cylinder;
	uint8_t	heads;
	uint8_t	sectors_per_track;
};

/* File version, as it is stored in the footer. */
struct vhd_version {
	uint16_t major;
	uint16_t minor;
};

#endif



#ifndef _VHDBAT_H_
#define _VHDBAT_H_

#include "diskimage.h"

struct vhd_bat;

/*
 * Creates a new block allocation table by reading it from disk.
 */
LDI_ERROR vhd_bat_new(void *source, struct vhd_bat **bat, int numblocks, struct logger logger);

/*
 * Returns the block offset for the given block.
 */
uint32_t vhd_bat_get_block_offset(struct vhd_bat *bat, int block);

#endif /* _VHDBAT_H_ */

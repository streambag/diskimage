#ifndef _VHDINSTANCE_H_
#define _VHDINSTANCE_H_

#include "fileinterface.h"
#include "diskimage.h"

struct vhdinstance;

/*
 * Creates the instance state.
 */
LDI_ERROR vhdinstance_new(struct fileinterface *fi, char *path, struct vhdinstance **instance, struct logger logger);

/*
 * Deallocates the instance state and sets the pointer to NULL.
 */
void vhdinstance_destroy(struct vhdinstance **instance);

/*
 * Returns a diskinfo struct with properties for the disk.
 */
struct diskinfo vhdinstance_diskinfo(struct vhdinstance *instance);

/*
 * Reads nbytes at offset into the buffer.
 */
LDI_ERROR vhdinstance_read(struct vhdinstance *instance, char *buf, size_t nbytes, off_t offset);

/*
 * Writes nbytes from the buffer into the diskimage at the specified offset.
 */
LDI_ERROR vhdinstance_write(struct vhdinstance *instance, char *buf, size_t nbytes, off_t offset);

#endif					/* _VHDINSTANCE_H_ */

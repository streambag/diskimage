
#ifndef _VMDKDESCRIPTORFILE_H_
#define _VMDKDESCRIPTORFILE_H_

#include "diskimage.h"

#include "vmdkextentdescriptor.h"

/*
 * The different types of vmdk files.
 */
enum vmdktype {
	MONOLITHIC_SPARSE,
	VMFS_SPARSE,
	MONOLITHIC_FLAT,
	VMFS,
	TWO_GB_MAX_EXTENT_SPARSE,
	TWO_GB_MAX_EXTENT_FLAT,
	FULL_DEVICE,
	VMFS_RAW,
	PARTITIONED_DEVICE,
	VMFS_RAW_DEVICE_MAP,
	VMFS_PASSTHROUGH_RAW_DEVICE_MAP,
	STREAM_OPTIMIZED
};

/*
 * The parsed form of a vmdk descriptor file.
 */
struct vmdkdescriptorfile {
	uint32_t cid;
	uint32_t parentcid;
	uint16_t version;
	enum vmdktype filetype;
	struct vmdkextentdescriptor **extents;
	uint16_t numextents;
};

/*
 * Creates a new descriptorfile given the description in source.
 */
LDI_ERROR vmdkdescriptorfile_new(void *source, struct vmdkdescriptorfile **descriptorfile, size_t length, struct logger logger);

/*
 * Frees mamory and zeros the descriptorfile.
 */
void	vmdkdescriptorfile_destroy(struct vmdkdescriptorfile **descriptorfile);

#endif					/* _VMDKDESCRIPTORFILE_H_ */

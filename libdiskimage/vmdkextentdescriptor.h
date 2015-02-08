#ifndef _VMDKEXTENTDESCRIPTOR_H_
#define _VMDKEXTENTDESCRIPTOR_H_

/*
 * The possible access restrictions for an extent.
 */
enum vmdk_access {
	VMDK_ACCESS_RW,
	VMDK_ACCESS_RDONLY,
	VMDK_ACCESS_NOACCESS
};

/*
 * The possible types of extent.
 */
enum vmdk_extent_type {
	VMDK_EXTENT_FLAT,
	VMDK_EXTENT_SPARSE,
	VMDK_EXTENT_ZERO,
	VMDK_EXTENT_VMFS,
	VMDK_EXTENT_VMFSSPARSE,
	VMDK_EXTENT_VMFSRDM,
	VMDK_EXTENT_VMFSRAW
};

/*
 * The parsed form of a vmdk extent.
 */
struct vmdkextentdescriptor {
	enum vmdk_access access;
	size_t	sectors;
	enum vmdk_extent_type type;
	char   *filename;
	off_t	offset;
};

/*
 * Creates a new extentdescriptor given the description in source.
 */
int	vmdkextentdescriptor_new(char *source, struct vmdkextentdescriptor **descriptor);

/*
 * Frees memory and zeros the descriptor.
 */
void	vmdkextentdescriptor_destroy(struct vmdkextentdescriptor **descriptor);

#endif					/* _VMDKEXTENTDESCRIPTOR_H_ */

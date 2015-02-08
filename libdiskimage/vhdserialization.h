
#ifndef _VHDSERIALIZATION_H_
#define _VHDSERIALIZATION_H_

#include <uuid.h>
#include <stdbool.h>
#include "vhdtypes.h"

/*
 * Read a string from the source.
 */
void	read_chars(void *source, void *dest, uint32_t count);

/*
 * Write a string to the destination.
 */
void	write_chars(void *source, void *dest, uint32_t count);

/*
 * Read a uint32 from the source.
 */
uint32_t read_uint32(void *source);

/*
 * Write a uint32 to the destination.
 */
void	write_uint32(uint32_t value, void *dest);

/*
 * Read an int32 from the source.
 */
int32_t	read_int32(void *source);

/*
 * Write an int32 to the destination.
 */
void	write_int32(int32_t value, void *dest);

/*
 * Read a uint64 from the source.
 */
uint64_t read_uint64(void *source);

/*
 * Write a uint64 to the destination.
 */
void	write_uint64(uint64_t value, void *dest);

/*
 * Read a uuid from the source.
 */
void	read_uuid(void *source, uuid_t *dest);

/*
 * Write a uuid to the destination.
 */
void	write_uuid(uuid_t *source, void *dest);

/*
 * Read a bool from the source.
 */
bool	read_bool(void *source);

/*
 * Write a bool to the destination.
 */
void	write_bool(bool value, void *dest);

/*
 * Read the file version from the source.
 */
void	read_version(void *source, struct vhd_version *dest);

/*
 * Write the file version to the destination.
 */
void	write_version(struct vhd_version *source, void *dest);

/*
 * Read the disk geometry from the source.
 */
void	read_disk_geometry(void *source, struct vhd_disk_geometry *dest);

/*
 * Write the disk geometry to the destination.
 */
void	write_disk_geometry(struct vhd_disk_geometry *source, void *dest);

#endif					/* _VHDSERIALIZATION_H_ */

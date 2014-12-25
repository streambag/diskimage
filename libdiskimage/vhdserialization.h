
#ifndef _VHDSERIALIZATION_H_
#define _VHDSERIALIZATION_H_

#include "vhdtypes.h"

/*
 * Read a string from the source.
 */
void read_chars(void *source, void *dest, uint32_t count);

/*
 * Read a uint32 from the source.
 */
uint32_t read_uint32(void *source);

/*
 * Read an int32 from the source.
 */
int32_t read_int32(void *source);

/*
 * Read a uint64 from the source.
 */
uint64_t read_uint64(void *source);

/*
 * Read a uuid from the source.
 */
void read_uuid(void *source, uuid_t * dest);

/*
 * Read a bool from the source.
 */
bool read_bool(void *source);

/*
 * Read the file version from the source.
 */
void read_version(void *source, struct vhd_version *dest);

/*
 * Read the disk geometry from the source.
 */
void read_disk_geometry(void *source, struct vhd_disk_geometry *dest);

#endif /* _VHDSERIALIZATION_H_ */


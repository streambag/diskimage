
#include <sys/endian.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <uuid.h>

#include "vhdtypes.h"

/*
 * Read a string from the source.
 */
void
read_chars(void *source, void *dest, uint32_t count)
{
	memcpy(dest, source, count);
	((char *)dest)[count] = 0;
}

/*
 * Read a uint32 from the source.
 */
uint32_t
read_uint32(void *source) {
	return (be32dec(source));
}

/*
 * Read an int32 from the source.
 */
int32_t
read_int32(void *source)
{
	uint32_t us = be32dec(source);
	return *((int32_t *) &us);
}

/*
 * Read a uint64 from the source.
 */
uint64_t
read_uint64(void *source)
{
	return (be64toh(*(uint64_t *) source));
}

/*
 * Read a uuid from the source.
 */
void
read_uuid(void *source, uuid_t * dest)
{
	uuid_dec_be(source, dest);
}

/*
 * Read a bool from the source.
 */
bool
read_bool(void *source)
{
	return *(uint8_t *) source == 1;
}

/*
 * Read the file version from the source.
 */
void
read_version(void *source, struct vhd_version *dest)
{
	dest->major = be16toh(((uint16_t *) source)[0]);
	dest->minor = be16toh(((uint16_t *) source)[1]);
}

/*
 * Read the disk geometry from the source.
 */
void
read_disk_geometry(void *source, struct vhd_disk_geometry *dest)
{
	uint8_t *bytes = (uint8_t *) source;

	dest->cylinder = be16toh(*(uint16_t *) bytes);
	dest->heads = *(uint8_t *) (bytes + 2);
	dest->sectors_per_track = *(uint8_t *) (bytes + 3);
}


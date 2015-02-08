
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
 * Write a string to the destination.
 */
void 
write_chars(void *source, void *dest, uint32_t count)
{
	memcpy(dest, source, count);
}

/*
 * Read a uint32 from the source.
 */
uint32_t
read_uint32(void *source)
{
	return (be32dec(source));
}

/*
 * Write a uint32 to the destination.
 */
void 
write_uint32(uint32_t value, void *dest)
{
	be32enc(dest, value);
}

/*
 * Read an int32 from the source.
 */
int32_t
read_int32(void *source)
{
	uint32_t us = be32dec(source);

	return *((int32_t *)&us);
}

/*
 * Write an int32 to the destination.
 */
void 
write_int32(int32_t value, void *dest)
{
	be32enc(dest, *(uint32_t *)(&value));
}

/*
 * Read a uint64 from the source.
 */
uint64_t
read_uint64(void *source)
{
	return (be64dec(source));
}

/*
 * Write a uint64 to the destination.
 */
void 
write_uint64(uint64_t value, void *dest)
{
	be64enc(dest, value);
}

/*
 * Read a uuid from the source.
 */
void
read_uuid(void *source, uuid_t *dest)
{
	uuid_dec_be(source, dest);
}

/*
 * Write a uuid to the destination.
 */
void 
write_uuid(uuid_t *source, void *dest)
{
	uuid_enc_be(dest, source);
}

/*
 * Read a bool from the source.
 */
bool
read_bool(void *source)
{
	return *(uint8_t *)source == 1;
}

/*
 * Write a bool to the destination.
 */
void 
write_bool(bool value, void *dest)
{
	*(uint8_t *)dest = value ? 1 : 0;
}

/*
 * Read the file version from the source.
 */
void
read_version(void *source, struct vhd_version *dest)
{
	dest->major = be16dec(source);
	dest->minor = be16dec(source + 2);
}

/*
 * Write the file version to the destination.
 */
void 
write_version(struct vhd_version *source, void *dest)
{
	be16enc(dest, source->major);
	be16enc(dest + 2, source->minor);
}

/*
 * Read the disk geometry from the source.
 */
void
read_disk_geometry(void *source, struct vhd_disk_geometry *dest)
{
	uint8_t *bytes = (uint8_t *)source;

	dest->cylinder = be16dec(bytes);
	dest->heads = *(uint8_t *)(bytes + 2);
	dest->sectors_per_track = *(uint8_t *)(bytes + 3);
}

/*
 * Write the disk geometry to the destination.
 */
void 
write_disk_geometry(struct vhd_disk_geometry *source, void *dest)
{
	be16enc(dest, source->cylinder);
	*((uint8_t *)dest + 2) = source->heads;
	*((uint8_t *)dest + 3) = source->sectors_per_track;
}

#include <sys/types.h>
#include <stdbool.h>
#include <uuid.h>

#include "vhdchecksum.h"
#include "vhdtypes.h"

/*
 * Calculate the checksum for a uint8 array.
 */
uint32_t
checksum_uint8_array(uint8_t *source, uint32_t count)
{
	int i;
	uint32_t result = 0;

	for (i = 0; i < count; i++) {
		result += *(source + i);
	}

	return result;
}

/*
 * Calculate the checksum for a string.
 */
uint32_t
checksum_chars(char *source, uint32_t count) 
{
	return checksum_uint8_array((uint8_t*)source, count);
}

/*
 * Calculate the checksum for a uint32.
 */
uint32_t
checksum_uint32(uint32_t source)
{
	return checksum_uint8_array((uint8_t*)&source, 4);
}

/*
 * Calculate the checksum for an int32.
 */
uint32_t
checksum_int32(int32_t source)
{
	return checksum_uint8_array((uint8_t*)&source, 4);
}

/*
 * Calculate the checksum for a uint8.
 */
uint32_t
checksum_uint8(uint8_t source)
{
	return source;
}

/*
 * Calculate the checksum for a uint16.
 */
uint32_t
checksum_uint16(uint16_t source)
{
	return checksum_uint8_array((uint8_t*)&source, 2);
}

/*
 * Calculate the checksum for a uint64.
 */
uint32_t
checksum_uint64(uint64_t source)
{
	return checksum_uint8_array((uint8_t*)&source, 8);
}

/*
 * Calculate the checksum for a version struct.
 */
uint32_t
checksum_version(struct vhd_version source)
{
	return checksum_uint16(source.major) + checksum_uint16(source.minor);
}

/*
 * Calculate the checksum for a disk geometry struct.
 */
uint32_t
checksum_disk_geometry(struct vhd_disk_geometry source)
{
	return checksum_uint16(source.cylinder) +
	    checksum_uint8(source.heads) +
	    checksum_uint8(source.sectors_per_track);
}

/*
 * Calculate the checksum for a uuid.
 */
uint32_t
checksum_uuid(uuid_t *source)
{
	uint8_t buf[16];
	uuid_enc_be(buf, source);
	
	return checksum_uint8_array(buf, 16);
}

/*
 * Calculate the checksum for a bool.
 */
uint32_t
checksum_bool(bool source)
{
	return source ? 1 : 0;
}

#ifndef _VHDCHECKSUM_H_
#define _VHDCHECKSUM_H_

#include <sys/types.h>
#include <stdbool.h>
#include <uuid.h>

#include "vhdtypes.h"

/*
 * Calculate the checksum for a uint8 array.
 */
uint32_t checksum_uint8_array(uint8_t *source, uint32_t count);

/*
 * Calculate the checksum for a string.
 */
uint32_t checksum_chars(char *source, uint32_t count);

/*
 * Calculate the checksum for a uint32.
 */
uint32_t checksum_uint32(uint32_t source);

/*
 * Calculate the checksum for an int32.
 */
uint32_t checksum_int32(int32_t source);

/*
 * Calculate the checksum for a uint8.
 */
uint32_t checksum_uint8(uint8_t source);

/*
 * Calculate the checksum for a uint16.
 */
uint32_t checksum_uint16(uint16_t source);

/*
 * Calculate the checksum for a uint64.
 */
uint32_t checksum_uint64(uint64_t source);

/*
 * Calculate the checksum for a version struct.
 */
uint32_t checksum_version(struct vhd_version source);

/*
 * Calculate the checksum for a disk geometry struct.
 */
uint32_t checksum_disk_geometry(struct vhd_disk_geometry source);

/*
 * Calculate the checksum for a uuid.
 */
uint32_t checksum_uuid(uuid_t *source);

/*
 * Calculate the checksum for a bool.
 */
uint32_t checksum_bool(bool source);

#endif					/* _VHDCHECKSUM_H_ */

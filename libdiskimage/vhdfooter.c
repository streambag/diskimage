
#include <sys/endian.h>
#include <string.h>
#include <stdlib.h>
#include <uuid.h>
#include <errno.h>

#include "internal.h"
#include "log.h"
#include "vhdfooter.h"
#include "vhdchecksum.h"
#include "vhdserialization.h"

/* Defines the offsets used in VHD file footer. */
#define COOKIE_OFFSET 0
#define FEATURES_OFFSET 8
#define FILE_FORMAT_VERSION_OFFSET 12
#define DATA_OFFSET_OFFSET 16
#define TIME_STAMP_OFFSET 24
#define CREATOR_APPLICATION_OFFSET 28
#define CREATOR_VERSION_OFFSET 32
#define CREATOR_HOST_OS_OFFSET 36
#define ORIGINAL_SIZE_OFFSET 40
#define CURRENT_SIZE_OFFSET 48
#define DISK_GEOMETRY_OFFSET 56
#define DISK_TYPE_OFFSET 60
#define CHECKSUM_OFFSET 64
#define UNIQUE_ID_OFFSET 68
#define SAVED_STATE_OFFSET 84

void	log_footer(struct vhd_footer *footer);

/* Features defined in the footer. */
enum vhd_features {
	VHD_FEATURES_NONE = 0,
	VHD_FEATURES_TEMPORARY = 1,
	VHD_FEATURES_RESERVED = 2
};

/* Defines the type of VHD disk this is. */
enum vhd_disk_type {
	VHD_TYPE_NONE = 0,
	VHD_TYPE_RESERVED,
	VHD_TYPE_FIXED,
	VHD_TYPE_DYNAMIC,
	VHD_TYPE_DIFFERENCING,
	VHD_TYPE_RESERVED2,
	VHD_TYPE_RESERVED3
};

/* The entire structure of the VHD footer. */
struct vhd_footer {
	/* The actual data read from the file. */
	char	cookie[9];
	enum vhd_features features;
	struct vhd_version file_format_version;
	uint64_t data_offset;
	int32_t	time_stamp;
	char	creator_application[5];
	struct vhd_version creator_version;
	uint32_t creator_host_os;
	uint64_t original_size;
	uint64_t current_size;
	struct vhd_disk_geometry disk_geometry;
	enum vhd_disk_type disk_type;
	uint32_t checksum;
	uuid_t	unique_id;
	bool	saved_state;

	/* Other helper objects. */
	uint32_t calculated_checksum;
	struct logger logger;
};


/*
 * Calculate the checksum for the entire footer.
 */
static uint32_t
calculate_checksum(struct vhd_footer *footer)
{
	uint32_t result = 0;

	result += checksum_chars(footer->cookie, 8);
	result += checksum_uint32(footer->features);
	result += checksum_version(footer->file_format_version);
	result += checksum_uint64(footer->data_offset);
	result += checksum_int32(footer->time_stamp);
	result += checksum_chars(footer->creator_application, 4);
	result += checksum_version(footer->creator_version);
	result += checksum_uint32(footer->creator_host_os);
	result += checksum_uint64(footer->original_size);
	result += checksum_uint64(footer->current_size);
	result += checksum_disk_geometry(footer->disk_geometry);
	result += checksum_uint32(footer->disk_type);
	result += checksum_uuid(&footer->unique_id);
	result += checksum_bool(footer->saved_state);

	/* The checksum is the 1's complement of the sum of bytes. */
	return ~result;
}

/*
 * Reads the entire footer structure from the source.
 */
static void
read_footer(void *source, struct vhd_footer *footer)
{
	uint8_t *bytes = (uint8_t *)source;

	read_chars(bytes + COOKIE_OFFSET, &footer->cookie, 8);
	footer->features = read_uint32(bytes + FEATURES_OFFSET);
	read_version(bytes + FILE_FORMAT_VERSION_OFFSET,
	    &footer->file_format_version);
	footer->data_offset = read_uint64(bytes + DATA_OFFSET_OFFSET);
	footer->time_stamp = read_int32(bytes + TIME_STAMP_OFFSET);
	read_chars(bytes + CREATOR_APPLICATION_OFFSET,
	    &footer->creator_application, 4);
	read_version(bytes + CREATOR_VERSION_OFFSET, &footer->creator_version);
	footer->creator_host_os = read_uint32(bytes + CREATOR_HOST_OS_OFFSET);
	footer->original_size = read_uint64(bytes + ORIGINAL_SIZE_OFFSET);
	footer->current_size = read_uint64(bytes + CURRENT_SIZE_OFFSET);
	read_disk_geometry(bytes + DISK_GEOMETRY_OFFSET,
	    &footer->disk_geometry);
	footer->disk_type = read_uint32(bytes + DISK_TYPE_OFFSET);
	footer->checksum = read_uint32(bytes + CHECKSUM_OFFSET);
	read_uuid(bytes + UNIQUE_ID_OFFSET, &footer->unique_id);
	footer->saved_state = read_bool(bytes + SAVED_STATE_OFFSET);

	footer->calculated_checksum = calculate_checksum(footer);
}

/*
 * Get the string representatino of a uuid. Must be freed.
 */
static char *
get_uuid_string(uuid_t uuid)
{
	char *uuid_str;

	uint32_t status;

	uuid_to_string(&uuid, &uuid_str, &status);
	return uuid_str;
}

/*
 * Creates a new vhd footer structure by reading from source.
 */
LDI_ERROR
vhd_footer_new(void *source, struct vhd_footer **footer, struct logger logger)
{
	errno = 0;
	*footer = malloc((unsigned int)sizeof(struct vhd_footer));
	if (footer == NULL) {
		return ERROR(LDI_ERR_NOMEM);
	}
	(*footer)->logger = logger;
	read_footer(source, *footer);

	log_footer(*footer);

	return NO_ERROR;
}

/*
 * Writes the footer to the destination buffer.
 */
LDI_ERROR
vhd_footer_write(struct vhd_footer *footer, void *dest)
{
	uint8_t *bytes = (uint8_t *)dest;

	write_chars(footer->cookie, bytes + COOKIE_OFFSET, 8);
	write_uint32(footer->features, bytes + FEATURES_OFFSET);
	write_version(&footer->file_format_version, bytes + FILE_FORMAT_VERSION_OFFSET);
	write_uint64(footer->data_offset, bytes + DATA_OFFSET_OFFSET);
	write_int32(footer->time_stamp, bytes + TIME_STAMP_OFFSET);
	write_chars(&footer->creator_application, bytes + CREATOR_APPLICATION_OFFSET, 4);
	write_version(&footer->creator_version, bytes + CREATOR_VERSION_OFFSET);
	write_uint32(footer->creator_host_os, bytes + CREATOR_HOST_OS_OFFSET);
	write_uint64(footer->original_size, bytes + ORIGINAL_SIZE_OFFSET);
	write_uint64(footer->current_size, bytes + CURRENT_SIZE_OFFSET);
	write_disk_geometry(&footer->disk_geometry, bytes + DISK_GEOMETRY_OFFSET);
	write_uint32(footer->disk_type, bytes + DISK_TYPE_OFFSET);
	write_uint32(footer->calculated_checksum, bytes + CHECKSUM_OFFSET);
	write_uuid(&footer->unique_id, bytes + UNIQUE_ID_OFFSET);
	write_bool(footer->saved_state, bytes + SAVED_STATE_OFFSET);

	return NO_ERROR;
}

/*
 * Prints all the values in the footer, for debug purposes.
 */
void
log_footer(struct vhd_footer *footer)
{
	char *uuid_str;

	LOG_VERBOSE(footer->logger, "Cookie:\t\t\t%s\n", footer->cookie);
	LOG_VERBOSE(footer->logger, "Features:\t\t%d\n", footer->features);
	LOG_VERBOSE(footer->logger, "File format version:\t%d.%d\n",
	    footer->file_format_version.major,
	    footer->file_format_version.minor);
	LOG_VERBOSE(footer->logger, "Data offset:\t\t%lu\n", footer->data_offset);
	LOG_VERBOSE(footer->logger, "Time stamp:\t\t%d\n", footer->time_stamp);
	LOG_VERBOSE(footer->logger, "Creator application:\t%s\n", footer->creator_application);
	LOG_VERBOSE(footer->logger, "Creator version:\t%d.%d\n",
	    footer->creator_version.major, footer->creator_version.minor);
	LOG_VERBOSE(footer->logger, "Creator host os:\t%x\n", footer->creator_host_os);
	LOG_VERBOSE(footer->logger, "Original size:\t\t%lu\n", footer->original_size);
	LOG_VERBOSE(footer->logger, "Current size:\t\t%lu\n", footer->current_size);
	LOG_VERBOSE(footer->logger, "Disk geometry (c:h:s):\t%d:%d:%d\n",
	    footer->disk_geometry.cylinder,
	    footer->disk_geometry.heads,
	    footer->disk_geometry.sectors_per_track);
	LOG_VERBOSE(footer->logger, "Disk type:\t\t%d\n", footer->disk_type);
	LOG_VERBOSE(footer->logger, "Checksum:\t\t%u (", footer->checksum);
	if (vhd_footer_isvalid(footer)) {
		LOG_VERBOSE(footer->logger, "valid)\n");
	} else {
		LOG_VERBOSE(footer->logger, "invalid real cheksum: %u)\n",
		    footer->calculated_checksum);
	}

	uuid_str = get_uuid_string(footer->unique_id);
	LOG_VERBOSE(footer->logger, "Unique id:\t\t%s", uuid_str);
	free(uuid_str);

	LOG_VERBOSE(footer->logger, "\n");
	LOG_VERBOSE(footer->logger, "Saved state:\t\t%d\n", footer->saved_state);
}

/*
 * Deallocates the memory for the footer and sets the pointer to NULL.
 */
void
vhd_footer_destroy(struct vhd_footer **footer)
{
	free(*footer);
	*footer = NULL;
}

/*
 * Returns true if the checksum for the footer is valid.
 */
bool
vhd_footer_isvalid(struct vhd_footer *footer)
{
	return footer->checksum == footer->calculated_checksum;
}

/*
 * Returns the disk type that this footer represents.
 */
enum disk_type
vhd_footer_disk_type(struct vhd_footer *footer)
{
	switch (footer->disk_type) {
	case VHD_TYPE_FIXED:
		return DISK_TYPE_FIXED;
	case VHD_TYPE_DYNAMIC:
		return DISK_TYPE_DYNAMIC;
	case VHD_TYPE_DIFFERENCING:
		return DISK_TYPE_DIFFERENCING;
	default:
		return DISK_TYPE_UNKNOWN;
	}
}

/*
 * Returns the current size of the disk.
 */
long
vhd_footer_disksize(struct vhd_footer *footer)
{
	return footer->current_size;
}

/*
 * Returns the offset to the beginning of the data.
 */
off_t
vhd_footer_offset(struct vhd_footer *footer)
{
	return footer->data_offset;
}

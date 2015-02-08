
#include <sys/types.h>
#include <stdlib.h>
#include <uuid.h>
#include <errno.h>

#include "diskimage.h"
#include "log.h"
#include "vhdchecksum.h"
#include "vhdserialization.h"
#include "vhdtypes.h"

#include "vhdheader.h"

#define COOKIE_OFFSET 0
#define DATA_OFFSET_OFFSET 8
#define TABLE_OFFSET_OFFSET 16
#define HEADER_VERSION_OFFSET 24
#define MAX_TABLE_ENTRIES_OFFSET 28
#define BLOCK_SIZE_OFFSET 32
#define CHECKSUM_OFFSET 36
#define PARENT_UNIQUE_ID_OFFSET 40
#define PARENT_TIME_STAMP_OFFSET 56
#define PARENT_UNICODE_NAME_OFFSET 64
/* There are 8 parent locator entries each 24 bytes long. */
#define PARENT_LOCATOR_ENTRIES_OFFSET 576

struct parent_locator_entry {
	/*
	 * Since we don't support differencing disks yet, just read data
	 * into a dummy string.
	 */
	char	dummy[24];
};

struct vhd_header {
	/* The actual data read from the file. */
	char	cookie[9];
	uint64_t data_offset;
	uint64_t table_offset;
	struct vhd_version header_version;
	uint32_t max_table_entries;
	uint32_t block_size;
	uint32_t checksum;
	uuid_t	parent_unique_id;
	int32_t	parent_time_stamp;
	char	parent_unicode_name[513];
	struct parent_locator_entry parent_locator_entries[8];

	/* Other helper objects. */
	uint32_t calculated_checksum;
	struct logger logger;
};

uint32_t
checksum_parent_locator_entry(struct parent_locator_entry *parent_locator_entry)
{
	return checksum_chars(parent_locator_entry->dummy, 28);
};

void
read_parent_locator_entry(uint8_t *source, struct parent_locator_entry *parent_locator_entry)
{
	read_chars(source, &parent_locator_entry->dummy, 28);
}

/*
 * Calculate the checksum for the entire footer.
 */
static uint32_t
calculate_checksum(struct vhd_header *header)
{
	int i;
	uint32_t result = 0;

	result += checksum_chars(header->cookie, 8);
	result += checksum_uint64(header->data_offset);
	result += checksum_uint64(header->table_offset);
	result += checksum_version(header->header_version);
	result += checksum_uint32(header->max_table_entries);
	result += checksum_uint32(header->block_size);
	result += checksum_uuid(&header->parent_unique_id);
	result += checksum_int32(header->parent_time_stamp);
	result += checksum_chars(header->parent_unicode_name, 512);

	for (i = 0; i < 8; i++) {
		result += checksum_parent_locator_entry(&header->parent_locator_entries[i]);
	}

	/* The checksum is the 1's complement of the sum of bytes. */
	return ~result;
}

/*
 * Reads the entire header structure from the source.
 */
static void
read_header(void *source, struct vhd_header *header)
{
	int i;
	uint8_t *bytes = (uint8_t *)source;

	read_chars(bytes + COOKIE_OFFSET, &header->cookie, 8);
	header->data_offset = read_uint64(bytes + DATA_OFFSET_OFFSET);
	header->table_offset = read_uint64(bytes + TABLE_OFFSET_OFFSET);
	read_version(bytes + HEADER_VERSION_OFFSET, &header->header_version);
	header->max_table_entries = read_uint32(bytes + MAX_TABLE_ENTRIES_OFFSET);
	header->block_size = read_uint32(bytes + BLOCK_SIZE_OFFSET);
	header->checksum = read_uint32(bytes + CHECKSUM_OFFSET);
	read_uuid(bytes + PARENT_UNIQUE_ID_OFFSET, &header->parent_unique_id);
	header->parent_time_stamp = read_int32(bytes + PARENT_TIME_STAMP_OFFSET);
	read_chars(bytes + PARENT_UNICODE_NAME_OFFSET, &header->parent_unicode_name, 512);

	for (i = 0; i < 8; i++) {
		read_parent_locator_entry(
		    bytes + PARENT_LOCATOR_ENTRIES_OFFSET + i * 24,
		    &header->parent_locator_entries[i]);
	}

	header->calculated_checksum = calculate_checksum(header);
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
 * Creates a new vhd_header object by reading the values from source.
 */
LDI_ERROR
vhd_header_new(void *source, struct vhd_header **header, struct logger logger)
{
	errno = 0;
	*header = malloc((unsigned int)sizeof(struct vhd_header));
	if (header == NULL) {
		return LDI_ERR_NOMEM;
	}
	(*header)->logger = logger;
	read_header(source, *header);

	log_header(*header);

	return LDI_ERR_NOERROR;
}

/*
 * Writes all the values in the header to the log, for debug purposes.
 */
void
log_header(struct vhd_header *header)
{
	char *uuid_str;

	LOG_VERBOSE(header->logger, "Header:\n");
	LOG_VERBOSE(header->logger, "Cookie:\t\t\t%s\n", header->cookie);
	LOG_VERBOSE(header->logger, "Data offset:\t\t%lu\n", header->data_offset);
	LOG_VERBOSE(header->logger, "Table offset:\t\t%lu\n", header->table_offset);
	LOG_VERBOSE(header->logger, "Header version:\t\t%d.%d\n",
	    header->header_version.major, header->header_version.minor);
	LOG_VERBOSE(header->logger, "Max table entries:\t%d\n", header->max_table_entries);
	LOG_VERBOSE(header->logger, "Block size:\t\t%d\n", header->block_size);
	LOG_VERBOSE(header->logger, "Checksum:\t\t%u (", header->checksum);
	if (vhd_header_isvalid(header)) {
		LOG_VERBOSE(header->logger, "valid)\n");
	} else {
		LOG_VERBOSE(header->logger, "invalid real cheksum: %u)\n",
		    header->calculated_checksum);
	}

	uuid_str = get_uuid_string(header->parent_unique_id);
	LOG_VERBOSE(header->logger, "Parent unique id:\t%s\n", uuid_str);
	free(uuid_str);

	LOG_VERBOSE(header->logger, "Parent time stamp:\t%d\n", header->parent_time_stamp);
	LOG_VERBOSE(header->logger, "Parent unicode name:\t%s\n", header->parent_unicode_name);
}

/*
 * Returns true if the checksum for the header is valid.
 */
bool
vhd_header_isvalid(struct vhd_header *header)
{
	return header->checksum == header->calculated_checksum;
}

/*
 * Returns the absolute file offset to the block allocation table.
 */
uint64_t
vhd_header_table_offset(struct vhd_header *header)
{
	return header->table_offset;
}

/*
 * Returns the size of the block allocation table.
 */
uint32_t
vhd_header_max_table_entries(struct vhd_header *header)
{
	return header->max_table_entries;
}

/*
 * Returns the number of sectors (each 512 bytes) in a block.
 */
uint32_t
vhd_header_block_size(struct vhd_header *header)
{
	return header->block_size;
}

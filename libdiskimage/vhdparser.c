
#include "log.h"
#include "vhdbat.h"
#include "vhdfooter.h"
#include "vhdheader.h"
#include "parser.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>

#include <unistd.h>

#include "fileinterface.h"
#include "internal.h"

/* The internal parser state. */
struct vhd_parser {
	/* The file descriptor of the opened file. */
	struct file *file;
	/* The type of disk. */
	enum disk_type disk_type;
	/* The structure read from the footer. */
	struct vhd_footer *footer;
	/*
	 * The structure read from the header (only for dynamic/differencing
	 * disks).
	 */
	struct vhd_header *header;
	/* The block allocation table. */
	struct vhd_bat *bat;
	/* Information about the opened file. */
	size_t	filesize;
	/* Used for logging. */
	struct logger logger;
};

void	vhd_parser_destroy(void **parser);

const int SECTOR_SIZE = 512;

/*
 * Reads the dynamic header data from disk.
 */
LDI_ERROR
read_dynamic_header(struct vhd_parser *parser)
{
	struct filemap *map;
	off_t header_offset;
	size_t bat_size;
	LDI_ERROR result;

	header_offset = vhd_footer_offset(parser->footer);

	file_getmap(parser->file, header_offset, header_offset + 1024, &map, parser->logger);
	result = vhd_header_new(map->pointer, &parser->header, parser->logger);
	filemap_destroy(&map);

	return result;
}

/*
 * Read the block allocation table from disk.
 */
LDI_ERROR
read_bat_data(struct vhd_parser *parser)
{
	struct filemap *map;
	off_t bat_offset;
	size_t bat_size;
	LDI_ERROR result;

	bat_offset = vhd_header_table_offset(parser->header);
	bat_size = vhd_header_max_table_entries(parser->header);

	file_getmap(parser->file, bat_offset, bat_offset + bat_size, &map, parser->logger);
	result = vhd_bat_new(map->pointer, &parser->bat, bat_size, parser->logger);
	filemap_destroy(&map);

	return NO_ERROR;
}

/*
 * Reads all the extra data needed for dynamic disks.
 */
LDI_ERROR
read_dynamic_data(struct vhd_parser *parser)
{
	LDI_ERROR result;

	result = read_dynamic_header(parser);

	if (IS_ERROR(result)) {
		/*
		 * Something went wrong when reading the dynamic header,
		 * don't attempt to read the BAT, since it depends on the
		 * header having been read correctly.
		 */
		return result;
	}
	/* Read BAT and return result. */
	return read_bat_data(parser);
}

/*
 * Reads the footer that is available in all VHD types.
 */
LDI_ERROR
read_footer(struct vhd_parser *parser)
{
	struct filemap *map;
	LDI_ERROR result;

	file_getmap(parser->file, parser->filesize - 512LL, 512, &map, parser->logger);
	result = vhd_footer_new(map->pointer, &parser->footer, parser->logger);
	filemap_destroy(&map);

	return result;
}

/*
 * Reads any format specific data from the file.
 */
LDI_ERROR
read_format_specific_data(struct vhd_parser *parser)
{
	switch (parser->disk_type) {
	case DISK_TYPE_FIXED:
		/* We're done. Just return. */
		return NO_ERROR;
	case DISK_TYPE_DYNAMIC:
		return read_dynamic_data(parser);
	default:
		/* Only fixed and dynamic VHDs are supported. */
		return ERROR(LDI_ERR_FILENOTSUP);
	}
}

/*
 * Creates the parser state.
 */
LDI_ERROR
vhd_parser_new(struct fileinterface *fi, char *path, void **parser, struct logger logger)
{
	LDI_ERROR result;
	struct vhd_parser *vhd_parser;
	struct file *file;

	/* Open the backing file. */
	result = file_open(fi, path, &file);
	if (IS_ERROR(result)) {
		return result;
	}

	errno = 0;
	*parser = malloc((unsigned int)sizeof(struct vhd_parser));
	if (parser == NULL) {
		return ERROR(LDI_ERR_NOMEM);
	}
	vhd_parser = (struct vhd_parser *)(*parser);
	vhd_parser->file = file;

	result = file_getsize(file, &vhd_parser->filesize);
	if (IS_ERROR(result)) {
		vhd_parser_destroy(parser);
		return result;
	}

	vhd_parser->logger = logger;
	/* Set pointers to NULL as default. */
	vhd_parser->footer = NULL;
	vhd_parser->header = NULL;
	vhd_parser->bat = NULL;

	/* Read the footer */
	result = read_footer(vhd_parser);

	if (IS_ERROR(result)) {
		/* We couldn't read the footer. Clean up and return. */
		vhd_parser_destroy(parser);
		return result;
	}
	vhd_parser->disk_type = vhd_footer_disk_type(vhd_parser->footer);

	result = read_format_specific_data(vhd_parser);
	if (IS_ERROR(result)) {
		/*
		 * Something went wrong when reading the format specific
		 * data.
		 */
		vhd_parser_destroy(parser);
	}
	return result;
}

/*
 * Deallocates the parser state and sets the pointer to NULL.
 */
void
vhd_parser_destroy(void **parser)
{
	struct vhd_parser *vhd_parser;

	vhd_parser = (struct vhd_parser *)(*parser);

	/* Close the file. */
	file_close(&(vhd_parser)->file);

	if (vhd_parser->footer) {
		vhd_footer_destroy(&vhd_parser->footer);
	}
	free(*parser);
	*parser = NULL;
}

/*
 * Returns a diskinfo struct with properties for the disk.
 */
struct diskinfo
vhd_parser_diskinfo(void *parser)
{
	struct vhd_parser *vhd_parser;
	struct diskinfo result;

	vhd_parser = (struct vhd_parser *)parser;
	result.disksize = vhd_footer_disksize(vhd_parser->footer);

	return result;
}

/*
 * Reads data from a raw file (not disk) offset.
 */
LDI_ERROR
read_from_raw_offset(struct file *file, char *buf, size_t nbytes, off_t offset, struct logger logger)
{
	struct filemap *map;

	file_getmap(file, offset, nbytes, &map, logger);
	memcpy(buf, map->pointer, nbytes);
	filemap_destroy(&map);
	return NO_ERROR;
}

/*
 * Reads data from a fixed disk.
 */
LDI_ERROR
read_fixed(struct vhd_parser *parser, char *buf, size_t nbytes, off_t offset)
{
	return read_from_raw_offset(parser->file, buf, nbytes, offset, parser->logger);
}

/*
 * Returns the block size (in number of sectors) for the VHD file.
 */
uint32_t
get_block_size(struct vhd_parser *parser)
{
	return vhd_header_block_size(parser->header);
}

/*
 * Returns the size (in bytes) of the sector bitmap that exists
 * at the beginning of each block.
 */
uint32_t
get_block_bitmap_size(struct vhd_parser *parser)
{
	uint32_t block_bitmap_size, sectors_per_block;

	sectors_per_block = get_block_size(parser) / SECTOR_SIZE;

	/* One bit is used for each sector. */
	block_bitmap_size = sectors_per_block / 8;

	/* The bitmap is padded to fill the last sector. */
	block_bitmap_size = block_bitmap_size +
	    (SECTOR_SIZE - block_bitmap_size % SECTOR_SIZE) % SECTOR_SIZE;

	return block_bitmap_size;
}


/*
 * Reads data from a dynamic VHD.
 */
LDI_ERROR
read_dynamic(struct vhd_parser *parser, char *buf, size_t nbytes, off_t offset)
{
	int block, bytes_to_read, bytes_left_in_block;
	uint32_t block_offset, block_size, block_bitmap_size;
	uint64_t file_offset;
	LDI_ERROR result;

	/*
	 * The dynamic VHD is split into blocks. Each block can be mapped to
	 * an offset in the file, or be set to -1 to indicate that it is
	 * unused. In that case, we treat it as filled with zeros.
	 */
	block_size = get_block_size(parser);
	block_bitmap_size = get_block_bitmap_size(parser);

	/* Loop until there is nothing more to read. */
	while (nbytes > 0) {
		/* Calculate the block number. */
		block = offset / block_size;

		/* Get the block offset in the file (in number of sectors). */
		block_offset = vhd_bat_get_block_offset(parser->bat, block);

		/* Calculate the number of bytes remaining in the block. */
		bytes_left_in_block = block_size - offset % block_size;
		/*
		 * If nbytes is larger than bytes_left_in_block, the read will
		 * continue in the next loop iteration with the next block.
		 */
		bytes_to_read = MIN(bytes_left_in_block, nbytes);

		if (block_offset == -1) {
			/*
			 * This block is not yet allocated, which means that
			 * it is all zeros.
			 */
			bzero(buf, bytes_to_read);
		} else {
			/* Calculate the actual file offset to read at. */
			file_offset = block_offset * SECTOR_SIZE + block_bitmap_size + offset % block_size;

			/* Do the actual read. */
			result = read_from_raw_offset(parser->file, buf, bytes_to_read, file_offset, parser->logger);
			if (IS_ERROR(result)) {
				return result;
			}
		}

		/* update offset, buf and nbytes */
		buf += bytes_to_read;
		nbytes -= bytes_to_read;
		offset += bytes_to_read;
	}

	return NO_ERROR;
}

/*
 * Reads nbytes at offset into the buffer.
 */
LDI_ERROR
vhd_parser_read(void *parser, char *buf, size_t nbytes, off_t offset)
{
	struct vhd_parser *vhd_parser = (struct vhd_parser *)parser;

	switch (vhd_parser->disk_type) {
	case DISK_TYPE_FIXED:
		return read_fixed(vhd_parser, buf, nbytes, offset);
	case DISK_TYPE_DYNAMIC:
		return read_dynamic(vhd_parser, buf, nbytes, offset);
	default:
		/* Should not happen. */
		return ERROR(LDI_ERR_FILENOTSUP);
	}
}

/*
 * Writes nbytes of data at offset from the buffer to the VHD.
 */
LDI_ERROR
write_fixed(struct vhd_parser *parser, char *buf, size_t nbytes, off_t offset)
{
	struct filemap *map;

	file_getmap(parser->file, offset, nbytes, &map, parser->logger);
	memcpy(map->pointer, buf, nbytes);
	filemap_destroy(&map);

	return NO_ERROR;
}


/*
 * Extends a dynamic VHD with space for an extra block.
 * Does not update the block allocation table.
 */
LDI_ERROR
extend_file(struct vhd_parser *parser)
{
	size_t old_size, extension_size, new_size;
	struct filemap *map;
	LDI_ERROR res;

	old_size = parser->filesize;

	/* Write blocksize + sector_bitmap_size zeros. */
	extension_size = get_block_size(parser) + get_block_bitmap_size(parser);
	new_size = old_size + extension_size;

	res = file_setsize(parser->file, new_size);
	if (IS_ERROR(res)) {
		return res;
	}

	/* Write a new footer at the end. */
	file_getmap(parser->file, new_size - 512, 512, &map, parser->logger);
	vhd_footer_write(parser->footer, map->pointer);
	filemap_destroy(&map);

	/* Zero out the old footer. */
	file_getmap(parser->file, old_size - 512, 512, &map, parser->logger);;
	bzero(map->pointer, 512);
	filemap_destroy(&map);

	/* Update parser->filesize now that the size is updated. */
	res = file_getsize(parser->file, &parser->filesize);
	if (IS_ERROR(res)) {
		return res;
	}

	return NO_ERROR;
}

/*
 * Updates the sector bitmap for a block
 */
void
update_block_bitmap(void *destination, int sectors_in_block)
{
	char *bytes = destination;

	while (sectors_in_block > 0) {
		*bytes = 0xF;

		bytes++;
		sectors_in_block -= 8;
	}
}

/*
 * Writes nbytes of data at offset from the buffer to a dynamic VHD.
 */
LDI_ERROR
write_dynamic(struct vhd_parser *parser, char *buf, size_t nbytes, off_t offset)
{
	struct filemap *map;
	const int sector_size = 512;
	int block, bytes_to_write, bytes_left_in_block, sectors_per_block;
	uint32_t block_offset, block_size, block_bitmap_size, offset_in_block;
	uint64_t file_offset;
	size_t original_file_size, bat_size;
	off_t bat_offset;
	LDI_ERROR result;

	/*
	 * The dynamic VHD is split into blocks. Each block can be mapped to
	 * an offset in the file, or be set to -1 to indicate that it is
	 * unused. In that case, we treat it as filled with zeros.
	 */
	block_size = get_block_size(parser);
	block_bitmap_size = get_block_bitmap_size(parser);

	while (nbytes > 0) {

		/* Calculate the block number. */
		block = offset / block_size;

		/* Get the block offset in the file (in number of sectors). */
		block_offset = vhd_bat_get_block_offset(parser->bat, block);

		/* Calculate the number of bytes remaining in the block. */
		bytes_left_in_block = block_size - offset % block_size;
		/*
		 * If nbytes is larger than bytes_left_in_block, the read will
		 * continue in the next loop iteration with the next block.
		 */
		bytes_to_write = MIN(bytes_left_in_block, nbytes);

		if (block_offset == -1) {
			/* This block is not yet allocated. */

			original_file_size = parser->filesize;

			/*
			 * Make room for the new block by extending the
			 * file.
			 */
			extend_file(parser);

			/*
			 * Update the BAT. The footer has been moved to the
			 * end of the extended file. The new block starts
			 * where the old footer started.
			 */
			block_offset = original_file_size / 512 - 1;
			vhd_bat_add_block(parser->bat, block, block_offset);

			bat_offset = vhd_header_table_offset(parser->header);
			bat_size = vhd_header_max_table_entries(parser->header);

			file_getmap(parser->file, bat_offset, bat_offset + bat_size, &map, parser->logger);
			vhd_bat_write(parser->bat, map->pointer);
			filemap_destroy(&map);

		}
		offset_in_block = offset % block_size;

		/* Do the actual write. */
		file_getmap(parser->file, block_offset * SECTOR_SIZE + block_bitmap_size + offset_in_block, bytes_to_write, &map, parser->logger);
		memcpy(map->pointer, buf, bytes_to_write);
		filemap_destroy(&map);


		/*
		 * Update the sector bitmap. This indicates which sectors in
		 * the block have data in them. Ideally this can be used to
		 * do some optimization during read. This is not what we do
		 * below. Instead, we indicate that all sectors in the block
		 * have data in them if one of them have. This is probably
		 * not ideal, but seems to be what VirtualBox does.
		 */
		file_getmap(parser->file, block_offset * SECTOR_SIZE, block_bitmap_size, &map, parser->logger);
		sectors_per_block = get_block_size(parser) / SECTOR_SIZE;
		update_block_bitmap(map->pointer, sectors_per_block);
		filemap_destroy(&map);

		/* update offset, buf and nbytes */
		buf += bytes_to_write;
		nbytes -= bytes_to_write;
		offset += bytes_to_write;
	}

	return NO_ERROR;

}

/*
 * Writes nbytes from the buffer into the diskimage at the specified offset.
 */
LDI_ERROR
vhd_parser_write(void *parser, char *buf, size_t nbytes, off_t offset)
{
	struct vhd_parser *vhd_parser = (struct vhd_parser *)parser;

	switch (vhd_parser->disk_type) {
	case DISK_TYPE_FIXED:
		return write_fixed(vhd_parser, buf, nbytes, offset);
	case DISK_TYPE_DYNAMIC:
		return write_dynamic(vhd_parser, buf, nbytes, offset);
	default:
		/* Should not happen. */
		return ERROR(LDI_ERR_FILENOTSUP);
	}
}

/*
 * Define an ldi_parser struct for the VHD parser and define the parser
 * using the PARSER_DEFINE macro. This will make the parser discoverable
 * in diskimage.c.
 */
static struct ldi_parser vhd_parser_format = {
	.name = "vhd",
	.construct = vhd_parser_new,
	.destructor = vhd_parser_destroy,
	.diskinfo = vhd_parser_diskinfo,
	.read = vhd_parser_read,
	.write = vhd_parser_write
};

PARSER_DEFINE(vhd_parser_format);

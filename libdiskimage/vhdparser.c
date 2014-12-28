
#include "log.h"
#include "vhdbat.h"
#include "vhdfooter.h"
#include "vhdheader.h"
#include "filemap.h"
#include "parser.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>

#include <unistd.h>

/* The internal parser state. */
struct vhd_parser {
	/* The file descriptor of the opened file. */
	int fd;
    /* The type of disk. */
    enum disk_type disk_type;
	/* The structure read from the footer. */
	struct vhd_footer *footer;
    /* The structure read from the header (only for dynamic/differencing disks). */
    struct vhd_header *header;
    /* The block allocation table. */
    struct vhd_bat *bat;
	/* Information about the opened file. */
	struct stat sb;
	/* Used for logging. */
	struct logger logger;
};

void vhd_parser_destroy(void **parser);

/*
 * Reads the dynamic header data from disk.
 */
LDI_ERROR
read_dynamic_header(struct vhd_parser *parser)
{
	struct filemap *map;
    off_t header_offset;
	char	*source;
    size_t bat_size;
	LDI_ERROR result;
    header_offset = vhd_footer_offset(parser->footer);
    filemap_create(parser->fd, header_offset, header_offset+1024, &map, parser->logger);
    source = filemap_pointer(map);

    result = vhd_header_new(source, &parser->header, parser->logger);

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
	char	*source;
    size_t bat_size;
	LDI_ERROR result;

    bat_offset = vhd_header_table_offset(parser->header);
    bat_size = vhd_header_max_table_entries(parser->header);

    filemap_create(parser->fd, bat_offset, bat_offset + bat_size, &map, parser->logger);
    source = filemap_pointer(map);

    result = vhd_bat_new(source, &parser->bat, bat_size, parser->logger);

    filemap_destroy(&map);

    return LDI_ERR_NOERROR;
}

/*
 * Reads all the extra data needed for dynamic disks.
 */
LDI_ERROR
read_dynamic_data(struct vhd_parser *parser)
{
	LDI_ERROR result;

    result = read_dynamic_header(parser);

    if (result != LDI_ERR_NOERROR) {
        /*
         * Something went wront when reading the dynamic header,
         * don't attempt to read the BAT, since it depends on the
         * header having been read correctly.
         */
        return result;
    }

    /*
     * Read the BAT and return the result.
     */
    return read_bat_data(parser);
}

/*
 * Reads the footer that is available in all VHD types.
 */
LDI_ERROR
read_footer(struct vhd_parser *parser)
{
	struct filemap *map;
	char	*source;
	LDI_ERROR result;
	filemap_create(parser->fd, parser->sb.st_size-512LL, 512, &map, parser->logger);

	source = filemap_pointer(map);

	result = vhd_footer_new(source, &parser->footer, parser->logger);
	filemap_destroy(&map);

    return result;
}

/*
 * Reads any format specific data from the file.
 */
LDI_ERROR
read_format_specific_data(struct vhd_parser *parser) {
    switch (parser->disk_type) {
        case DISK_TYPE_FIXED:
            /* We're done. Just return. */
            return LDI_ERR_NOERROR;
        case DISK_TYPE_DYNAMIC:
            return read_dynamic_data(parser);
        default:
            /* Only fixed and dynamic VHDs are supported. */
            return LDI_ERR_FILENOTSUP;
    }
}

/*
 * Creates the parser state.
 */
LDI_ERROR
vhd_parser_new(int fd, void **parser, struct logger logger)
{
	LDI_ERROR result;
	struct vhd_parser *vhd_parser; 
    
	errno = 0;
	*parser = malloc((unsigned int)sizeof(struct vhd_parser));
	if (parser == NULL) {
		return LDI_ERR_NOMEM;
	}

	vhd_parser = (struct vhd_parser*)(*parser);


	vhd_parser->fd = fd;
	fstat(fd, &vhd_parser->sb);
	vhd_parser->logger = logger;
    /* Set pointers to NULL as default. */
    vhd_parser->footer = NULL;
    vhd_parser->header = NULL;
    vhd_parser->bat = NULL;

	/* Read the footer */
	result = read_footer(vhd_parser);

	if (result != LDI_ERR_NOERROR) {
		/* We couldn't read the footer. Clean up and return. */
        vhd_parser_destroy(parser);
        return result;
	}

    vhd_parser->disk_type = vhd_footer_disk_type(vhd_parser->footer);

    result = read_format_specific_data(vhd_parser);
    if (result != LDI_ERR_NOERROR) {
        /* Something went wrong when reading the format specific data. */
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
	vhd_parser = (struct vhd_parser*)(*parser);
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

	vhd_parser = (struct vhd_parser*)parser;
	result.disksize = vhd_footer_disksize(vhd_parser->footer);

	return result;
}

/*
 * Reads data from a raw file (not disk) offset.
 */
LDI_ERROR
read_from_raw_offset(int fd, char *buf, size_t nbytes, off_t offset, struct logger logger)
{
	char *source;
	struct filemap *map;
    filemap_create(fd, offset, nbytes, &map, logger);
    source = filemap_pointer(map);
    memcpy(buf, source, nbytes);

    filemap_destroy(&map);
    return LDI_ERR_NOERROR;
}

/*
 * Reads data from a fixed disk.
 */
LDI_ERROR
read_fixed(struct vhd_parser *parser, char *buf, size_t nbytes, off_t offset)
{
    return read_from_raw_offset(parser->fd, buf, nbytes, offset, parser->logger);
}

/*
 * Reads data from a dynamic VHD.
 */
LDI_ERROR
read_dynamic(struct vhd_parser *parser, char *buf, size_t nbytes, off_t offset)
{
    const int sector_size = 512;
    int block, bytes_to_read, bytes_left_in_block;
    uint32_t block_offset, block_size, sectors_per_block, block_bitmap_size;
    uint64_t file_offset;
    LDI_ERROR result;

    /* 
     * The dynamic VHD is split into blocks. Each block can be mapped to 
     * an offset in the file, or be set to -1 to indicate that it is 
     * unused. In that case, we treat it as filled with zeros.
     */
    block_size = vhd_header_block_size(parser->header);
    sectors_per_block = block_size / sector_size;

    /* One bit is used for each sector. */
    block_bitmap_size = sectors_per_block / 8;
    /* The bitmap is padded to fill the last sector. */
    block_bitmap_size = block_bitmap_size + 
        (sector_size - block_bitmap_size % sector_size) % sector_size;
    /* Loop until there is nothing more to read. */
    while (nbytes > 0) {
        /* Calculate the block number. */
        block = offset/block_size;

        /* Get the block offset in the file (in number of sectors). */
        block_offset = vhd_bat_get_block_offset(parser->bat, block);

        /* Calculate the number of bytes remaining in the block. */
        bytes_left_in_block = block_size - offset%block_size;
        /*
         * If nbytes is larger than bytes_left_in_block, the read will
         * continue in the next loop iteration with the next block.
         */
        bytes_to_read = MIN(bytes_left_in_block, nbytes);

        if (block_offset == -1) {
            /* 
             * This block is not yet allocated, which means that it is
             * all zeros.
             */
            bzero(buf, bytes_to_read);
        } else {
            /* Calculate the actual position in the file to read at. */
            file_offset = block_offset * sector_size + block_bitmap_size + offset%block_size;

            /* Do the actual read. */
            result = read_from_raw_offset(parser->fd, buf, bytes_to_read, file_offset, parser->logger);
            if (result != LDI_ERR_NOERROR) {
                return result;
            }
        }
        
        /* update offset, buf and nbytes */
        buf += bytes_to_read;
        nbytes -= bytes_to_read;
        offset += bytes_to_read;
    }

    return LDI_ERR_NOERROR;
}

/*
 * Reads nbytes at offset into the buffer.
 */
LDI_ERROR 
vhd_parser_read(void *parser, char *buf, size_t nbytes, off_t offset)
{
	struct vhd_parser *vhd_parser = (struct vhd_parser*)parser;

    switch (vhd_parser->disk_type) {
        case DISK_TYPE_FIXED:
            return read_fixed(vhd_parser, buf, nbytes, offset);
        case DISK_TYPE_DYNAMIC:
            return read_dynamic(vhd_parser, buf, nbytes, offset);
        default:
            /* Should not happen. */
            return LDI_ERR_FILENOTSUP;
    }
}

/*
 * Writes nbytes from the buffer into the diskimage at the specified offset.
 */
LDI_ERROR 
vhd_parser_write(void *parser, char *buf, size_t nbytes, off_t offset)
{
	char *destination;
	struct filemap *map;
	struct vhd_parser *vhd_parser;

	vhd_parser = (struct vhd_parser*)parser;

    if (vhd_parser->disk_type != DISK_TYPE_FIXED) {
        /* We don't support writing to dynamic disks yet. */
        return LDI_ERR_NOERROR;
    }

	filemap_create(vhd_parser->fd, offset, nbytes, &map, vhd_parser->logger);
	destination = filemap_pointer(map);
	memcpy(destination, buf, nbytes);

	filemap_destroy(&map);

	fsync(vhd_parser->fd);

	return LDI_ERR_NOERROR;
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

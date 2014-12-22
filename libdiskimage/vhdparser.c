
#include "vhdfooter.h"
#include "filemap.h"
#include "parser.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

/* The internal parser state. */
struct vhd_parser {
	/* The file descriptor of the opened file. */
	int fd;
	/* The structure read from the footer. */
	struct vhd_footer *footer;
	/* Information about the opened file. */
	struct stat sb;
};

/*
 * Creates the parser state.
 */
LDI_ERROR
vhd_parser_new(int fd, void **parser)
{
	struct filemap *map;
	char	*source;
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

	/* Read the footer */

	map = filemap_create(vhd_parser->fd, vhd_parser->sb.st_size-512LL, 512);

	source = filemap_pointer(map);

	result = vhd_footer_new(source, &vhd_parser->footer);
	if (result != LDI_ERR_NOERROR) {
		/* We couldn't allocate the footer. Clean up and return. */
		free(*parser);
		*parser = NULL;
		return result;
	}

	filemap_destroy(&map);

	vhd_footer_printf(vhd_parser->footer);

	return LDI_ERR_NOERROR;
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
 * Reads nbytes at offset into the buffer.
 */
LDI_ERROR 
vhd_parser_read(void *parser, char *buf, size_t nbytes, off_t offset)
{
	off_t data_offset;
	char *source;
	struct filemap *map;
	struct vhd_parser *vhd_parser;

	vhd_parser = (struct vhd_parser*)parser;

	map = filemap_create(vhd_parser->fd, offset, nbytes);
	source = filemap_pointer(map);
	memcpy(buf, source, nbytes);

	filemap_destroy(&map);

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
	.read = vhd_parser_read
};
PARSER_DEFINE(vhd_parser_format);

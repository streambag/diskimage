
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

struct vhd_parser {
	int fd;
	struct vhd_footer *footer;
	struct stat sb;
};

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

struct diskinfo
vhd_parser_diskinfo(void *parser)
{
	struct vhd_parser *vhd_parser;
	struct diskinfo result;

	vhd_parser = (struct vhd_parser*)parser;
	result.disksize = vhd_footer_disksize(vhd_parser->footer);

	return result;
}

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

static struct ldi_parser vhd_parser_format = {
	.name = "vhd",
	.construct = vhd_parser_new,
	.destructor = vhd_parser_destroy,
	.diskinfo = vhd_parser_diskinfo,
	.read = vhd_parser_read
};
PARSER_DEFINE(vhd_parser_format);


#include "log.h"
#include "parser.h"
#include "vhdinstance.h"

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
	/* Used for logging. */
	struct logger logger;
	/* File instance - only one so far */
	struct vhdinstance *instance;
};

void	vhd_parser_destroy(void **parser);


/*
 * Creates the parser state.
 */
LDI_ERROR
vhd_parser_new(struct fileinterface *fi, char *path, void **parser, struct logger logger)
{
	LDI_ERROR result;
	struct vhd_parser *vhd_parser;


	errno = 0;
	*parser = malloc((unsigned int)sizeof(struct vhd_parser));
	if (parser == NULL) {
		return ERROR(LDI_ERR_NOMEM);
	}
	vhd_parser = (struct vhd_parser *)(*parser);

	vhd_parser->logger = logger;

	vhd_parser->instance = NULL;
	result = vhdinstance_new(fi, path, &(vhd_parser->instance), logger);
	if (IS_ERROR(result)) {
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


	if (vhd_parser->instance) {
		vhdinstance_destroy(&(vhd_parser->instance));
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

	vhd_parser = (struct vhd_parser *)parser;
	return vhdinstance_diskinfo(vhd_parser->instance);
}


/*
 * Reads nbytes at offset into the buffer.
 */
LDI_ERROR
vhd_parser_read(void *parser, char *buf, size_t nbytes, off_t offset)
{
	struct vhd_parser *vhd_parser = (struct vhd_parser *)parser;

	return vhdinstance_read(vhd_parser->instance, buf, nbytes, offset);
}

/*
 * Writes nbytes from the buffer into the diskimage at the specified offset.
 */
LDI_ERROR
vhd_parser_write(void *parser, char *buf, size_t nbytes, off_t offset)
{
	struct vhd_parser *vhd_parser = (struct vhd_parser *)parser;

	return vhdinstance_write(vhd_parser->instance, buf, nbytes, offset);
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

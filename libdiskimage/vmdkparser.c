#include <stdlib.h>
#include <string.h>

#include "diskimage.h"
#include "fileinterface.h"
#include "parser.h"
#include "vmdkdescriptorfile.h"

/* The internal parser state. */
struct vmdkparser {
	/* The injected logger object which is used for all logging. */
	struct logger logger;

	/* A pointer to the file containing the descriptor. */
	struct file *descriptor;

	/* The length of the file containing the descriptor. */
	size_t	descriptorlength;

	/* The struct representing the contents of the descriptorfile. */
	struct vmdkdescriptorfile *descriptorfile;

	/* A pointer to the file containing the data. */
	struct file *datafile;
};

/*
 * Creates the parser state.
 */
LDI_ERROR
vmdkparser_new(struct fileinterface *fi, char *path, void **parser, struct logger logger)
{
	struct vmdkparser *vmdkparser;
	struct filemap *map;
	char *dir;
	char *datapath;

	*parser = malloc(sizeof(struct vmdkparser));
	vmdkparser = (struct vmdkparser *)(*parser);
	vmdkparser->logger = logger;

	/* Open the descriptor file. */
	file_open(fi, path, &vmdkparser->descriptor);
	vmdkparser->descriptorlength = file_getsize(vmdkparser->descriptor);

	file_getmap(vmdkparser->descriptor, 0, vmdkparser->descriptorlength, &map, logger);
	vmdkdescriptorfile_new(map->pointer, &vmdkparser->descriptorfile, vmdkparser->descriptorlength, logger);
	filemap_destroy(&map);

	/* Get the path to the data file. */
	dir = file_getdirectory(vmdkparser->descriptor);
	datapath = fileinterface_getpath(fi, dir, vmdkparser->descriptorfile->extents[0]->filename);

	/* Open the data file. */
	file_open(fi, datapath, &vmdkparser->datafile);

	/* Clean up temporary strings. */
	free(dir);
	free(datapath);

	return LDI_ERR_NOERROR;
}

/*
 * Deallocates the parser state and sets the pointer to NULL.
 */
void
vmdkparser_destroy(void **parser)
{
	struct vmdkparser *vmdkparser = *parser;

	vmdkdescriptorfile_destroy(&vmdkparser->descriptorfile);
	file_close(&vmdkparser->descriptor);
	file_close(&vmdkparser->datafile);
	free(*parser);
}

/*
 * Returns a diskinfo struct with properties for the disk.
 */
struct diskinfo
vmdkparser_diskinfo(void *parser)
{
	struct diskinfo result;
	struct vmdkparser *vmdkparser = parser;

	/* Calculate the disk size. */
	result.disksize = vmdkparser->descriptorfile->extents[0]->sectors * 512;

	return result;
}

/*
 * Reads nbytes at offset into the buffer.
 */
LDI_ERROR
vmdkparser_read(void *parser, char *buf, size_t nbytes, off_t offset)
{
	struct filemap *map;
	struct vmdkparser *vmdkparser = parser;

	file_getmap(vmdkparser->datafile, offset, nbytes, &map, vmdkparser->logger);
	memcpy(buf, map->pointer, nbytes);
	filemap_destroy(&map);

	return LDI_ERR_NOERROR;
}

/*
 * Writes nbytes from the buffer into the diskimage at the specified offset.
 */
LDI_ERROR
vmdkparser_write(void *parser, char *buf, size_t nbytes, off_t offset)
{
	/* Not yet supported, ignore the call. */
	return LDI_ERR_NOERROR;
}

/*
 * Define an ldi_parser struct for the VMDK parser and define the parser
 * using the PARSER_DEFINE macro. This will make the parser discoverable
 * in diskimage.c.
 */
static struct ldi_parser vmdkparser_format = {
	.name = "vmdk",
	.construct = vmdkparser_new,
	.destructor = vmdkparser_destroy,
	.diskinfo = vmdkparser_diskinfo,
	.read = vmdkparser_read,
	.write = vmdkparser_write
};

PARSER_DEFINE(vmdkparser_format);

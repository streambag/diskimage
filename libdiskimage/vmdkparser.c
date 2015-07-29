#include <stdlib.h>
#include <string.h>

#include "diskimage.h"
#include "fileinterface.h"
#include "internal.h"
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

void	vmdkparser_destroy(void **parser);

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
	LDI_ERROR res;

	/* Allocate the parser structure. */
	*parser = malloc(sizeof(struct vmdkparser));
	if (!*parser) {
		/* Failed to allocate the parser. */
		return ERROR(LDI_ERR_NOMEM);
	}
	vmdkparser = (struct vmdkparser *)(*parser);
	vmdkparser->logger = logger;

	/* Intialize pointers to null to simplify cleanup on error. */
	vmdkparser->descriptor = NULL;
	vmdkparser->descriptorfile = NULL;
	vmdkparser->datafile = NULL;

	/* Open the descriptor file. */
	res = file_open(fi, path, &vmdkparser->descriptor);
	if (IS_ERROR(res)) {
		/* Something went wrong opening the descriptor file. */
		vmdkparser_destroy(parser);
		return res;
	}

	/* Get the size of the descriptor file. */
	res = file_getsize(vmdkparser->descriptor, &vmdkparser->descriptorlength);
	if (IS_ERROR(res)) {
		/* Failed to get the size of the file. */
		vmdkparser_destroy(parser);
		return res;
	}

	/* Map the entire descriptorfile into memory. */
	res = file_getmap(vmdkparser->descriptor, 0, vmdkparser->descriptorlength, &map, logger);
	if (IS_ERROR(res)) {
		/* Couldn't map memory. */
		vmdkparser_destroy(parser);
		return res;
	}

	/* Create a descriptorfile struct from the mapped file. */
	res = vmdkdescriptorfile_new(
	    map->pointer,
	    &vmdkparser->descriptorfile,
	    vmdkparser->descriptorlength,
	    logger);
	filemap_destroy(&map);
	if (IS_ERROR(res)) {
		/* Couldn't read the descriptorfile. */
		vmdkparser_destroy(parser);
		return res;
	}

	/* Get the path to the data file. */
	dir = file_getdirectory(vmdkparser->descriptor);
	res = fileinterface_getpath(fi, dir, vmdkparser->descriptorfile->extents[0]->filename, &datapath);
	if (IS_ERROR(res)) {
		/* Couldn't get file path. */
		vmdkparser_destroy(parser);
		return res;
	}

	/* Open the data file. */
	res = file_open(fi, datapath, &vmdkparser->datafile);

	/* Clean up temporary strings. */
	free(dir);
	free(datapath);

	/* Check if the file was opened correctly. */
	if (IS_ERROR(res)) {
		/* Failed to open the data file. */
		vmdkparser_destroy(parser);
	}

	return NO_ERROR;
}

/*
 * Deallocates the parser state and sets the pointer to NULL.
 */
void
vmdkparser_destroy(void **parser)
{
	struct vmdkparser *vmdkparser = *parser;

	if (vmdkparser->descriptorfile) {
		vmdkdescriptorfile_destroy(&vmdkparser->descriptorfile);
	}

	if (vmdkparser->descriptor) {
		file_close(&vmdkparser->descriptor);
	}

	if (vmdkparser->datafile) {
		file_close(&vmdkparser->datafile);
	}
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
	LDI_ERROR res;

	res = file_getmap(vmdkparser->datafile, offset, nbytes, &map, vmdkparser->logger);
	if (IS_ERROR(res)) {
		return res;
	}
	memcpy(buf, map->pointer, nbytes);
	filemap_destroy(&map);

	return NO_ERROR;
}

/*
 * Writes nbytes from the buffer into the diskimage at the specified offset.
 */
LDI_ERROR
vmdkparser_write(void *parser, char *buf, size_t nbytes, off_t offset)
{
	/* Not yet supported, ignore the call. */
	return NO_ERROR;
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

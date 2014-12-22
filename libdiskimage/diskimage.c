
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>

#include "diskimage.h"
#include "parser.h"

/* Keeps track of all state between calls. */
struct diskimage {
	/* The file descriptor of the opened file. */
	int fd;
	/* Information about the disk. */
	struct diskinfo diskinfo;
	/* The definition of the parser for the file. */
	struct ldi_parser *parser;
	/* The internal state of the parser. */
	void *parser_state;
};

/*
 * Opens the disk image at the supplied path with the given format.
 * Allocates the diskimage structure that is passed to all successive calls.
 * The diskimage structure must be deallocated using diskimage_destroy.
 */
LDI_ERROR
diskimage_open(char *path, char *format, struct diskimage **di)
{
	int fd;
	struct ldi_parser *parser = NULL, **iter;

	/* Go through all formats and find the one that matches the format name. */
	SET_FOREACH(iter, parsers) {
		if (strcasecmp(format, (*iter)->name) == 0)
			parser = *iter;
	}

	/* Check that we actually found a parser matching the name */
	if (parser == NULL)
		return LDI_ERR_FORMATUNKNOWN;

	/* Open the backing file. */
	fd = open(path, O_RDONLY | O_DIRECT | O_FSYNC);

	/* All state must be saved in the diskimage struct. */
	*di = malloc(sizeof(struct diskimage));
	(*di)->fd = fd;
	(*di)->parser = parser;

	/* Let the parser create its own format specific parser state. */
	(*di)->parser->construct(fd, &(*di)->parser_state);

	/* Get the disk info from the parser so that we know the disk size */
	(*di)->diskinfo = (*di)->parser->diskinfo((*di)->parser_state);

	return LDI_ERR_NOERROR;
}

/*
 * Deallocates and sets the diskimage pointer ot zero.
 */
void
diskimage_destroy(struct diskimage **di)
{
	/* We don't know what the parser state is. Let the parser destroy it. */
	(*di)->parser->destructor(&((*di)->parser_state));

	/* Close the file. */
	close((*di)->fd);

	/* Free the memory allocated for the diskimage struct. */
	free(*di);
	*di = NULL;
}

/*
 * Resturns a structure containing disk information.
 */
struct diskinfo
diskimage_diskinfo(struct diskimage *di)
{
	/* Hand over to the file format aware parser. */
	return di->parser->diskinfo(di->parser_state);
}


/*
 * Reads nbytes of data at offset into the supplied buffer.
 */
LDI_ERROR 
diskimage_read(struct diskimage *di, char *buf, size_t nbytes, off_t offset)
{
	/* Check that the whole range is within the range of the disk. */
	if (offset < 0 || offset+nbytes > di->diskinfo.disksize)
		return LDI_ERR_OUTOFRANGE;

	/* Hand over to the file format aware parser. */
	return di->parser->read(di->parser_state, buf, nbytes, offset);
}



#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>

#include "diskimage.h"
#include "fileinterface.h"
#include "internal.h"
#include "log.h"
#include "parser.h"

/* Keeps track of all state between calls. */
struct diskimage {
	struct fileinterface *fileinterface;
	/* Information about the disk. */
	struct diskinfo diskinfo;
	/* The definition of the parser for the file. */
	struct ldi_parser *parser;
	/* The internal state of the parser. */
	void   *parserstate;
	/* Object used for logging. */
	struct logger logger;
};

void
empty_log_write(int level, void *privarg, char *fmt,...)
{
}

/*
 * Opens the disk image at the supplied path with the given format.
 * Allocates the diskimage structure that is passed to all successive calls.
 * The diskimage structure must be deallocated using diskimage_destroy.
 */
LDI_ERROR
diskimage_open(char *path, char *format, struct logger logger, struct diskimage **di)
{
	struct fileinterface *fileinterface;
	struct ldi_parser *parser = NULL, **iter;
	LDI_ERROR res;

	/* Go through all formats and find one that matches the format name. */
	SET_FOREACH(iter, parsers) {
		if (strcasecmp(format, (*iter)->name) == 0)
			parser = *iter;
	}

	/* Check that we actually found a parser matching the name */
	if (parser == NULL)
		return ERROR(LDI_ERR_FORMATUNKNOWN);

	res = fileinterface_create(&fileinterface);
	if (IS_ERROR(res)) {
		return res;
	}

	/* All state must be saved in the diskimage struct. */
	*di = malloc(sizeof(struct diskimage));
	if (!di) {
		fileinterface_destroy(&fileinterface);
		return ERROR(LDI_ERR_NOMEM);
	}

	(*di)->parser = parser;
	(*di)->fileinterface = fileinterface;

	if (logger.write == NULL) {
		logger.write = empty_log_write;
	}
	(*di)->logger = logger;

	/* Let the parser create its own format specific parser state. */
	res = (*di)->parser->construct(fileinterface, path, &(*di)->parserstate, logger);
	if (IS_ERROR(res)) {
		fileinterface_destroy(&fileinterface);
		free(*di);
		*di = NULL;
		return res;
	}
	/* Get the disk info from the parser so that we know the disk size */
	(*di)->diskinfo = (*di)->parser->diskinfo((*di)->parserstate);

	return NO_ERROR;
}

/*
 * Deallocates and sets the diskimage pointer to zero.
 */
void
diskimage_destroy(struct diskimage **di)
{
	/* Let the parser destroy the parser state. */
	(*di)->parser->destructor(&((*di)->parserstate));

	fileinterface_destroy(&(*di)->fileinterface);

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
	return di->parser->diskinfo(di->parserstate);
}

/*
 * Reads nbytes of data at offset into the supplied buffer.
 */
LDI_ERROR
diskimage_read(struct diskimage *di, char *buf, size_t nbytes, off_t offset)
{
	LDI_ERROR result;

	/* Check that the whole range is within the range of the disk. */
	if (offset < 0 || offset + nbytes > di->diskinfo.disksize)
		return ERROR(LDI_ERR_OUTOFRANGE);

	LOG_VERBOSE(di->logger, "Reading %d bytes at %d\n", nbytes, offset);

	/* Hand over to the file format aware parser. */
	result = di->parser->read(di->parserstate, buf, nbytes, offset);
	LOG_VERBOSE(di->logger, "Result: %d\n", result);
	return result;
}

/*
 * Writes nbytes from the buffer into the diskimage at the specified offset.
 */
LDI_ERROR
diskimage_write(struct diskimage *di, char *buf, size_t nbytes, off_t offset)
{
	LDI_ERROR result;

	/* Check that the whole range is within the range of the disk. */
	if (offset < 0 || offset + nbytes > di->diskinfo.disksize)
		return ERROR(LDI_ERR_OUTOFRANGE);

	LOG_VERBOSE(di->logger, "Writing %d bytes at %d\n", nbytes, offset);

	/* Hand over to the file format aware parser. */
	result = di->parser->write(di->parserstate, buf, nbytes, offset);
	LOG_VERBOSE(di->logger, "Result: %d\n", result);
	return result;
}

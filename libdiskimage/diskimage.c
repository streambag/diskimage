
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <fcntl.h>
#include "vhdparser.h"
#include "diskimage.h"

#include "parser.h"

enum disk_type {
	DISK_TYPE_VHD
};

struct diskimage {
	int fd;
	enum disk_type disk_type;
	struct diskinfo diskinfo;
	struct ldi_parser *parser;
	void *parser_state;
};

LDI_ERROR
diskimage_open(char *path, char *format, struct diskimage **di)
{
	int fd;
	struct ldi_parser *parser = NULL, **iter;

	SET_FOREACH(iter, parsers) {
		if (strcasecmp(format, (*iter)->name) == 0) {
			parser = *iter;
		}
	}

	if (parser == NULL)
		return LDI_ERR_FORMATUNKNOWN;

	fd = open(path, O_RDONLY | O_DIRECT | O_FSYNC);

	*di = malloc(sizeof(struct diskimage));
	(*di)->fd = fd;
	(*di)->disk_type = DISK_TYPE_VHD;
	(*di)->parser = parser;

	/*vhd_parser_new(fd, &(*di)->parser);*/
	(*di)->parser->construct(fd, &(*di)->parser_state);

	(*di)->diskinfo = (*di)->parser->diskinfo((*di)->parser_state);

	return LDI_ERR_NOERROR;
}

void
diskimage_destroy(struct diskimage **di)
{
	(*di)->parser->destructor(&((*di)->parser_state));
	close((*di)->fd);
	free(*di);
	*di = NULL;
}

struct diskinfo
diskimage_diskinfo(struct diskimage *di)
{
	return di->parser->diskinfo(di->parser_state);
}


LDI_ERROR 
diskimage_read(struct diskimage *di, char *buf, size_t nbytes, off_t offset)
{
	if (offset < 0 || offset+nbytes > di->diskinfo.disksize)
		return LDI_ERR_OUTOFRANGE;

	return di->parser->read(di->parser_state, buf, nbytes, offset);
}

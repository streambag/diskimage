
#ifndef _LDI_PARSER_H_
#define _LDI_PARSER_H_

#include <sys/linker_set.h>

/*
 * A common interface for all parsers.
 */
struct ldi_parser {
	/* The name of the parser */
	const char	*name;
	/* A constructor for the parser state. */
	LDI_ERROR	(*construct)(int fd, void **parser);
	/* A destructor for the parser state */
	void		(*destructor)(void **parser);
	/* Returns diskinfo with properties for the disk. */
	struct diskinfo	(*diskinfo)(void *parser);
	/* Reads data from the disk into the buffer. */
	LDI_ERROR	(*read)(void *parser, char *buf, size_t nbytes, off_t offset);
	/* Writes the data from the buffer to the diskimage at the given offset. */
	LDI_ERROR	(*write)(void *parser, char *buf, size_t nbytes, off_t offset);
};

/* Declare a linker set for all the parsers. */
SET_DECLARE(parsers, struct ldi_parser);

/* Define a preprocessor macro that is used to define each parser. */
#define PARSER_DEFINE(definition)	DATA_SET(parsers, definition)

#endif /* _LDI_PARSER_H_ */

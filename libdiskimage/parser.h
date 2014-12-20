
#ifndef _LDI_PARSER_H_
#define _LDI_PARSER_H_

#include <sys/linker_set.h>

struct ldi_parser {
	const char	*name;
	LDI_ERROR	(*construct)(int fd, void **parser);
	void		(*destructor)(void **parser);
	struct diskinfo	(*diskinfo)(void *parser);
	LDI_ERROR	(*read)(void *parser, char *buf, size_t nbytes, off_t offset);
};

SET_DECLARE(parsers, struct ldi_parser);
#define PARSER_DEFINE(definition)	DATA_SET(parsers, definition)

#endif /* _LDI_PARSER_H_ */

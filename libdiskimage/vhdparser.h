
#ifndef VHDPARSER_H
#define VHDPARSER_H

#include "diskimage.h"
#include <sys/types.h>

struct vhd_parser;

/*
LDI_ERROR
vhd_parser_new(int fd, struct vhd_parser **parser);

void
vhd_parser_destroy(struct vhd_parser **parser);

struct diskinfo
vhd_parser_diskinfo(struct vhd_parser *parser);

LDI_ERROR 
vhd_parser_read(struct vhd_parser *parser, char *buf, size_t nbytes, off_t offset);
*/
#endif /* VHDPARSER_H */


#ifndef FILEMAP_H
#define FILEMAP_H

struct filemap;

struct filemap *filemap_create(int fd, size_t offset, size_t length);
void filemap_destroy(struct filemap **map);

void * filemap_pointer(struct filemap *map);

#endif /* FILEMAP_H */

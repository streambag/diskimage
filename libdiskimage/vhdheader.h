
#ifndef _VHDHEADER_H_
#define _VHDHEADER_H_

struct vhd_header;

/*
 * Creates a new vhd_header object by reading the values from source.
 */
LDI_ERROR vhd_header_new(void *source, struct vhd_header **header, struct logger logger);

/*
 * Returns true if the checksum for the header is valid.
 */
bool vhd_header_isvalid(struct vhd_header *header);

/*
 * Writes all the values in the header to the log, for debug purposes.
 */
void log_header(struct vhd_header *header);
    
/*
 * Returns the absolute file offset to the block allocation table.
 */
uint64_t vhd_header_table_offset(struct vhd_header *header);

/*
 * Returns the size of the block allocation table.
 */
uint32_t vhd_header_max_table_entries(struct vhd_header *header);

/*
 * Returns the number of sectors (each 512 bytes) in a block.
 */
uint32_t vhd_header_block_size(struct vhd_header *header);

#endif /* _VHDHEADER_H_ */

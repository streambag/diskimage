
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

#include "diskimage.h"
#include "log.h"
#include "vhdserialization.h"


struct vhd_bat {
    uint32_t numblocks;
    uint32_t *block_offsets;

    struct logger logger;
};

void log_bat(struct vhd_bat *bat);

/*
 * Reads the block allocation table from disk.
 */
static void
read_bat(void *source, struct vhd_bat *bat)
{
    int i;
    bat->block_offsets = malloc(bat->numblocks * sizeof(uint32_t));

    for (i = 0; i < bat->numblocks; i++) {
        bat->block_offsets[i] = read_uint32(source + i*4);
    }
}


/*
 * Creates a new block allocation table by reading it from disk.
 */
LDI_ERROR
vhd_bat_new(void *source, struct vhd_bat **bat, int numblocks, struct logger logger)
{
    errno = 0;
    *bat = malloc((unsigned int)sizeof(struct vhd_bat));
    if (bat == NULL) {
        return LDI_ERR_NOMEM;
    }

    (*bat)->numblocks = numblocks;
    (*bat)->logger = logger;

    read_bat(source, *bat);
    
    log_bat(*bat);

    return LDI_ERR_NOERROR;
}

/*
 * Writes the block allocation table to the specified destination.
 */
LDI_ERROR
vhd_bat_write(struct vhd_bat *bat, void *destination)
{
    int i;

    for (i = 0; i < bat->numblocks; i++) {
        write_uint32(bat->block_offsets[i], destination + i*4);
    }

    return LDI_ERR_NOERROR;
}

/*
 * Logs the contents of the bat for debug purposes.
 */
void
log_bat(struct vhd_bat *bat)
{
    int i;
    LOG_VERBOSE(bat->logger, "Block allocation table:\n");
    for (i = 0; i < bat->numblocks; i++) {
        LOG_VERBOSE(bat->logger, "Block %d: %d\n", i, bat->block_offsets[i]);
    }
}


/*
 * Returns the block offset for the given block.
 */
uint32_t
vhd_bat_get_block_offset(struct vhd_bat *bat, int block)
{
    return bat->block_offsets[block];
}

/*
 * Add a new block to the block allocation table.
 */
void
vhd_bat_add_block(struct vhd_bat *bat, int block, off_t offset)
{
    bat->block_offsets[block] = offset;
}

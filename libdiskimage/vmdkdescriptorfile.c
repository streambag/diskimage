
#include <sys/queue.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "vmdkdescriptorfile.h"
#include "vmdkextentdescriptor.h"

/*
 * Holds pointers to keys and values and their respective lengths.
 * This is used to avoid having to copy all strings just to have them
 * null terminated.
 */
struct key_value {
    char *key;
    size_t key_length;
    char *value;
    size_t value_length;
};

/*
 * Used internally in the get_key_value function.
 */
enum parser_state {
    BEFORE_KEY,
    KEY,
    BEFORE_VALUE,
    VALUE,
    DONE
};

/*
 * Parses the input up to the next newline and finds a '=' separated key 
 * and value. If no '=' is found, considers the entire line a value
 * with an empty key.
 */
size_t
get_key_value(char *input, size_t length, struct key_value *result)
{
    char *cur;
    char *last_nonwhite;
    char *end = input + length;
    enum parser_state state = BEFORE_KEY;

    result->key = NULL;
    result->key_length = 0;
    result->value = NULL;
    result->value_length = 0;

    cur = input;

    while (cur != end && (state == BEFORE_KEY || *cur != '\n')) {
        switch (*cur) {
            case ' ':
            case '\t':
            case '\n':
                /* Whitespace */
                break;
            case '=':
                if (state == KEY) {
                    /* Save the key length */
                    result->key_length = last_nonwhite - result->key + 1;
                    state = BEFORE_VALUE;
                }
                break;
            default:
                /* Regular character */
                if (state == BEFORE_KEY) {
                    state = KEY;
                    result->key = cur;
                } else if (state == BEFORE_VALUE) {
                    state = VALUE;
                    result->value = cur;
                }
                last_nonwhite = cur;
        }

        cur++;
    }

    if (state == KEY) {
        /* No = sign in line. Treat key as empty. */
        result->value = result->key;
        result->key = NULL;
    }

    result->value_length = last_nonwhite - result->value + 1;

    /* Strip quotes from the value. */
    if (result->value_length > 1 && result->value[0] == '"' && result->value[result->value_length-1] == '"') {
        result->value++;
        result->value_length -= 2;
    }

    return cur-input;
}

/*
 * Stores the version in the descriptorfile.
 */
int
handle_version(struct vmdkdescriptorfile *descriptorfile, char *value)
{
    long version;
    char *endptr = NULL;
    version = strtol(value, &endptr, 10);
    if (endptr != value + strlen(value)) {
        /* Failed to intepret the entire value as a long. */
        return -1;
    }
    descriptorfile->version = version;
    return 0;
}

/*
 * A mapping from type name to enum value.
 */
struct type_name {
    char *name;
    enum vmdktype type;
};

/*
 * Maps type names to enum values.
 */
struct type_name types[] = {
    { "monolithicSparse", MONOLITHIC_SPARSE },
    { "vmfsSparse", VMFS_SPARSE },
    { "monolithicFlat", MONOLITHIC_FLAT },
    { "vmfs", VMFS },
    { "twoGbMaxExtentSparse", TWO_GB_MAX_EXTENT_SPARSE },
    { "twoGbMaxExtentFlat", TWO_GB_MAX_EXTENT_FLAT },
    { "fullDevice", FULL_DEVICE },
    { "vmfsRaw", VMFS_RAW },
    { "partitionedDevice", PARTITIONED_DEVICE },
    { "vmfsRawDeviceMap", VMFS_RAW_DEVICE_MAP },
    { "vmfsPassthroughRawDeviceMap", VMFS_PASSTHROUGH_RAW_DEVICE_MAP },
    { "streamOptimized", STREAM_OPTIMIZED },
    { NULL, 0 }
};

/*
 * Stores the file type (called createtype in the spec) in the descriptorfile.
 */
int
handle_createtype(struct vmdkdescriptorfile *descriptorfile, char *value)
{
    int i;
    for (i = 0; types[i].name; i++) {
        if (strcmp(value, types[i].name) == 0) {
            descriptorfile->filetype = types[i].type;
            return 0;
        }
    }
    /* This is not a createtype we know. */
    return -1;
}

/*
 * Stores the extent description in the descriptorfile.
 */
int
handle_extent(struct vmdkdescriptorfile *descriptorfile, char *value)
{
    int res;
    struct vmdkextentdescriptor *extentdescriptor;
    res = vmdkextentdescriptor_new(value, &extentdescriptor);
    if (res != 0) {
        /* We couldn't parse the extent. */
        return -1;
    }

    /* 
     * Increment numextents and reallocate the extents array to make sure that
     * the new entry fits.
     */
    descriptorfile->numextents++;
    descriptorfile->extents = realloc(descriptorfile->extents, descriptorfile->numextents*sizeof(struct vmdkextentdescriptor *));
    descriptorfile->extents[descriptorfile->numextents-1] = extentdescriptor;

    return 0;
}

/*
 * Stores the creator id in the descriptorfile.
 */
int
handle_cid(struct vmdkdescriptorfile *descriptorfile, char *value)
{
    long cid;
    char *endptr = NULL;
    cid = strtol(value, &endptr, 16);
    if (endptr != value + strlen(value)) {
        /* Failed to intepret the entire value as a long. */
        return -1;
    }
    descriptorfile->cid = cid;
    return 0;
}

/*
 * Stores the parent creator id in the descriptorfile.
 */
int
handle_parentcid(struct vmdkdescriptorfile *descriptorfile, char *value)
{
    long parentcid;
    char *endptr = NULL;
    parentcid = strtol(value, &endptr, 16);
    if (endptr != value + strlen(value)) {
        /* Failed to intepret the entire value as a long. */
        return -1;
    }
    descriptorfile->parentcid = parentcid;
    return 0;
}

/*
 * Maps a key to a specific handler.
 */
struct handler {
    char *key;
    int (*handler)(struct vmdkdescriptorfile *, char *);
};

/*
 * Maps keys to handler functions.
 */
struct handler handlers[] = {
    { "", handle_extent },
    { "version", handle_version },
    { "createType", handle_createtype },
    { "CID", handle_cid },
    { "parentCID", handle_parentcid },
    { NULL, NULL }
};

/*
 * Calls the handler for the key, if any.
 */
int
handle_argument(struct vmdkdescriptorfile *descriptorfile, char *key, size_t key_length, char *value, size_t value_length)
{
    char *value_copy;
    int i, res;
    int (*handler)(struct vmdkdescriptorfile *, char *) = NULL;
    for (i = 0; handlers[i].key; i++) {
        if (strlen(handlers[i].key) == key_length
                && strncmp(key, handlers[i].key, key_length) == 0) {
            handler = handlers[i].handler;
            break;
        }
    }

    if (handler) {
        value_copy = strndup(value, value_length);
        res = handler(descriptorfile, value_copy);
        free(value_copy);
        if (res != 0) {
            /* Something went wrong. */
            return -1;
        }
    }

    return 0;
}

/*
 * Creates a new descriptorfile given the description in source.
 */
LDI_ERROR
vmdkdescriptorfile_new(void *source, struct vmdkdescriptorfile **descriptorfile, size_t length, struct logger logger)
{
    char *next_newline, *data;
    size_t line_length;
    size_t buffer_size = 10;
    struct key_value key_value;
    int res;
    int bytes_left = length;
    data = source;

    (*descriptorfile) = malloc((unsigned int)sizeof(struct vmdkdescriptorfile));
    (*descriptorfile)->numextents = 0;
    (*descriptorfile)->extents = NULL;
    while (bytes_left > 0) {
        line_length = get_key_value(data, length, &key_value);
        data += line_length + 1;
        bytes_left -= line_length + 1;

        if (line_length == 1 || (key_value.key_length == 0 && *key_value.value == '#')) {
            /* Empty or commented line. Ignore. */
            continue;
        }
        

        res = handle_argument(*descriptorfile, key_value.key, key_value.key_length, key_value.value, key_value.value_length);

        if (res != 0) {
            /* Something went wrong handling this key value pair. */
            vmdkdescriptorfile_destroy(descriptorfile);
            return LDI_ERR_PARSEERROR;
        }
    }

    return LDI_ERR_NOERROR;
}

/*
 * Frees mamory and zeros the descriptorfile.
 */
void
vmdkdescriptorfile_destroy(struct vmdkdescriptorfile **descriptorfile)
{
    int i;
    if ((*descriptorfile)->extents) {
        for (i = 0; i < (*descriptorfile)->numextents; i++) {
            vmdkextentdescriptor_destroy(&(*descriptorfile)->extents[i]);
        }
        free((*descriptorfile)->extents);
    }
    free(*descriptorfile);
    *descriptorfile = NULL;
}

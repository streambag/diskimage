#include <stdlib.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>

#include "vmdkextentdescriptor.h"

/*
 * Maps an access description string to an enum value.
 */
struct string_to_access {
	char   *string;
	enum vmdk_access value;
};

/*
 * Array mapping string to access enums.
 */
struct string_to_access access_strings[] = {
	{"RW", VMDK_ACCESS_RW},
	{"RDONLY", VMDK_ACCESS_RDONLY},
	{"NOACCESS", VMDK_ACCESS_NOACCESS},
	{NULL, 0}
};

/*
 * Handles the access part of the extent description.
 */
static int
handle_access(char *value, struct vmdkextentdescriptor *descriptor)
{
	int i;

	for (i = 0; access_strings[i].string; i++) {
		if (strcmp(access_strings[i].string, value) == 0) {
			descriptor->access = access_strings[i].value;
			return 0;
		}
	}

	return -1;
}

/*
 * Maps an extent type string to an enum value.
 */
struct string_to_extenttype {
	char   *string;
	enum vmdk_extent_type value;
};

/*
 * Array mapping string to extent type enums.
 */
struct string_to_extenttype extenttype_strings[] = {
	{"FLAT", VMDK_EXTENT_FLAT},
	{"SPARSE", VMDK_EXTENT_SPARSE},
	{"ZERO", VMDK_EXTENT_ZERO},
	{"VMFS", VMDK_EXTENT_VMFS},
	{"VMFSSPARSE", VMDK_EXTENT_VMFSSPARSE},
	{"VMFSRDM", VMDK_EXTENT_VMFSRDM},
	{"VMFSRAW", VMDK_EXTENT_VMFSRAW},
	{NULL, 0}
};

/*
 * Handles the extent type part of the extent description.
 */
static int
handle_extenttype(char *value, struct vmdkextentdescriptor *descriptor)
{
	int i;

	for (i = 0; extenttype_strings[i].string; i++) {
		if (strcmp(extenttype_strings[i].string, value) == 0) {
			descriptor->type = extenttype_strings[i].value;
			return 0;
		}
	}

	return -1;
}

/*
 * Handles the size part of the extent description.
 */
static int
handle_size(char *value, struct vmdkextentdescriptor *descriptor)
{
	char *end;
	size_t size = strtol(value, &end, 10);

	if (end != value + strlen(value)) {
		return -1;
	}
	descriptor->sectors = size;
	return 0;
}

/*
 * Handles the filename part of the extent description.
 */
static int
handle_filename(char *value, struct vmdkextentdescriptor *descriptor)
{
	descriptor->filename = strdup(value);;
	return 0;
}

/*
 * Handles the offset part of the extent description.
 */
static int
handle_offset(char *value, struct vmdkextentdescriptor *descriptor)
{
	char *end;
	off_t offset = strtol(value, &end, 10);

	if (end != value + strlen(value)) {
		return -1;
	}
	descriptor->offset = offset;
	return 0;
}


/*
 * Duplicates the string in the match and calls the handler.
 */
static int
dup_call(char *source, regmatch_t match, struct vmdkextentdescriptor *descriptor, int (handler) (char *value, struct vmdkextentdescriptor *descriptor))
{
	int result;
	char *match_string;
	size_t match_length;

	match_length = match.rm_eo - match.rm_so;
	match_string = strndup(source + match.rm_so, match_length);

	result = handler(match_string, descriptor);

	free(match_string);
	return result;
}

/*
 * Initializes the descriptor given the match results.
 */
static int
initialize(char *source, regmatch_t matches[], struct vmdkextentdescriptor *descriptor)
{
	int res;

	/* Handle the access restriction. */
	res = dup_call(source, matches[1], descriptor, handle_access);
	if (res != 0) {
		/* Failed to parse the access. */
		return -1;
	}
	/* Handle the size. */
	res = dup_call(source, matches[2], descriptor, handle_size);
	if (res != 0) {
		/* Failed to parse the size. */
		return -1;
	}
	/* Handle the extent type. */
	res = dup_call(source, matches[3], descriptor, handle_extenttype);
	if (res != 0) {
		/* Failed to parse the extent type. */
		return -1;
	}
	/* Handle the filename. */
	res = dup_call(source, matches[4], descriptor, handle_filename);
	if (res != 0) {
		/* Failed to parse the filename. */
		return -1;
	}
	/* The offset part is optional. */
	descriptor->offset = 0;
	if (matches[6].rm_so >= 0) {
		/* Handle the offset. */
		res = dup_call(source, matches[6], descriptor, handle_offset);
		if (res != 0) {
			/* Failed to parse the offset. */
			return -1;
		}
	}
	/* Everything went well. */
	return 0;
}

/*
 * Creates a new extentdescriptor given the description in source.
 */
int
vmdkextentdescriptor_new(char *source, struct vmdkextentdescriptor **descriptor)
{
	regex_t regex;
	regmatch_t matches[7];
	int res;

	/* Start by allocating the descriptor. */
	(*descriptor) = malloc(sizeof(struct vmdkextentdescriptor));
	(*descriptor)->filename = NULL;

	/*
	 * A vmdk extent descriptor looks like this:
	 * access size_in_sectors type "filename" [offset]
	 *
	 * Use a regex to parse the different parts.
	 */
	res = regcomp(&regex, "([^ ]+) ([0-9]+) ([^ ]+) \"([^\"]*)\"( ([0-9]*))?", REG_EXTENDED);
	if (res != 0) {
		/* Failed to compile the regex. */
		vmdkextentdescriptor_destroy(descriptor);
		return -1;
	}
	/* Execute the regex. */
	res = regexec(&regex, source, 7, matches, 0);

	/* Free the regex. */
	regfree(&regex);

	if (res != 0) {
		/* Failed to match the input. */
		vmdkextentdescriptor_destroy(descriptor);
		return -1;
	}
	res = initialize(source, matches, *descriptor);
	if (res != 0) {
		/* Failed to parse. */
		vmdkextentdescriptor_destroy(descriptor);
		return -1;
	}
	/* Everything went well. */
	return 0;
}

/*
 * Frees memory and zeros the descriptor.
 */
void
vmdkextentdescriptor_destroy(struct vmdkextentdescriptor **descriptor)
{
	/* If a filename has been set, we need to free the string. */
	if ((*descriptor)->filename) {
		free((*descriptor)->filename);
	}
	free(*descriptor);
	*descriptor = NULL;
}

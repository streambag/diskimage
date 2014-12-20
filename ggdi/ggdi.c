
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <err.h>
#include <sys/errno.h>
#include <sys/bio.h>

#include <geom/gate/g_gate.h>
#include "ggate.h"

#include "diskimage.h"

enum action { UNSET, CREATE, DESTROY, LIST, RESCUE };


static void
ggdi_serve(int unit, struct diskimage *di)
{
	struct g_gate_ctl_io ggio;
	size_t bsize;

	ggio.gctl_version = G_GATE_VERSION;
	ggio.gctl_unit = unit;
	bsize = 512;
	ggio.gctl_data = malloc(bsize);
	for (;;) {
		int error;
		ggio.gctl_length = bsize;
		ggio.gctl_error = 0;
		g_gate_ioctl(G_GATE_CMD_START, &ggio);
		error = ggio.gctl_error;
		switch (error) {
		case 0:
			break;
		case ECANCELED:
			/* Exit gracefully. */
			free(ggio.gctl_data);
			g_gate_close_device();
			exit(EXIT_SUCCESS);
		}

		error = 0;
		switch (ggio.gctl_cmd) {
		case BIO_READ:
			if ((size_t)ggio.gctl_length > bsize) {
				ggio.gctl_data = realloc(ggio.gctl_data,
				    ggio.gctl_length);
				if (ggio.gctl_data != NULL)
					bsize = ggio.gctl_length;
				else
					error = ENOMEM;
			}
			if (error == 0) {
				printf("Before diskimage_read\n");
				printf("offset: %d\n", ggio.gctl_offset);
				if (diskimage_read(di, ggio.gctl_data, 
				    ggio.gctl_length, 
				    ggio.gctl_offset) != LDI_ERR_NOERROR) 
					error = errno;
			}
			break;

		case BIO_DELETE:
		case BIO_WRITE:
			printf("Delete/write call\n");
			break;
		default:
			error = EOPNOTSUPP;
		}

		ggio.gctl_error = error;
		g_gate_ioctl(G_GATE_CMD_DONE, &ggio);
	}
}


static void
ggdi_create(char *path)
{

	struct g_gate_ctl_create ggioc;
	struct diskimage *di;
	struct diskinfo diskinfo;
	int unit;
	diskimage_open(path, "vhd", &di);

	diskinfo = diskimage_diskinfo(di);

	ggioc.gctl_version = G_GATE_VERSION;
	ggioc.gctl_unit = G_GATE_UNIT_AUTO;
	printf("Setting disksize: %ld\n", diskinfo.disksize);
	ggioc.gctl_mediasize = diskinfo.disksize;
	ggioc.gctl_sectorsize = 512;//diskinfo.sectorsize;
	ggioc.gctl_timeout = G_GATE_TIMEOUT;
	ggioc.gctl_flags = 0;
	ggioc.gctl_maxcount = 0;
	strlcpy(ggioc.gctl_info, path, sizeof(ggioc.gctl_info));
	g_gate_ioctl(G_GATE_CMD_CREATE, &ggioc);
	printf("%s%u\n", G_GATE_PROVIDER_NAME, ggioc.gctl_unit);
	unit = ggioc.gctl_unit;
	ggdi_serve(unit, di);

	diskimage_destroy(&di);
}

static void
usage()
{
	fprintf(stderr, "usage: %s create <path>\n", getprogname());
	exit(EXIT_FAILURE);
}


int
main(int argc, char *argv[])
{
	enum action action = UNSET;
	char *path;
	if (argc < 2)
		usage();
	if (strcasecmp(argv[1], "create") == 0)
		action = CREATE;
	else
		usage();

	argc -= 2;
	argv += 2;

	if (action == CREATE) {
		if (argc != 1)
			usage();
		g_gate_load_module();
		g_gate_open_device();
		path = argv[0];
		ggdi_create(path);
	}


	exit(EXIT_SUCCESS);
}


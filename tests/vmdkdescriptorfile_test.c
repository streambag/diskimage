
#include <atf-c.h>

/* Include the source file to test. */
#include "vmdkdescriptorfile.c"

LDI_ERROR vmdkextentdescriptor_new(char *source, struct vmdkextentdescriptor **descriptor) {
	*descriptor = malloc(sizeof(struct vmdkextentdescriptor));
	(*descriptor)->sectors = 44042240;
	return NO_ERROR;
}

void vmdkextentdescriptor_destroy(struct vmdkextentdescriptor **descriptor) {
	free(*descriptor);
	*descriptor = NULL;
}

char *testdata = 
    "# Disk DescriptorFile\n"
    "version=1\n"
    "CID=00000000\n"
    "parentCID=ffffffff\n"
    "createType=\"monolithicSparse\"\n"
    "# Extent description\n"
    "RW 44042240 SPARSE \"\"\n"
    "# The Disk Data Base\n"
    "#DDB\n"
    "ddb.adapterType = \"ide\"\n"
    "ddb.geometry.cylinders = \"44042240\"\n"
    "ddb.geometry.heads = \"1\"\n"
    "ddb.geometry.sectors = \"1\"\n";


void empty_log_write(int level, void *privarg, char *fmt, ...) { }

struct logger empty_logger = {
    .write = empty_log_write
};


ATF_TC_WITHOUT_HEAD(vmdkdescriptorfile_new__correctly_reads_data);
ATF_TC_BODY(vmdkdescriptorfile_new__correctly_reads_data, tc)
{
    struct vmdkdescriptorfile *descriptorfile;

    vmdkdescriptorfile_new(testdata, &descriptorfile, strlen(testdata), empty_logger);

    ATF_CHECK_EQ(1, descriptorfile->version);
    ATF_CHECK_EQ(0, descriptorfile->cid);
    ATF_CHECK_EQ(0xFFFFFFFF, descriptorfile->parentcid);
    ATF_CHECK_EQ(MONOLITHIC_SPARSE, descriptorfile->filetype);
    ATF_CHECK_EQ(1, descriptorfile->numextents);
    ATF_CHECK_EQ(44042240, descriptorfile->extents[0]->sectors);

    vmdkdescriptorfile_destroy(&descriptorfile);
}

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, vmdkdescriptorfile_new__correctly_reads_data);
    return 0;
}

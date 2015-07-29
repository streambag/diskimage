
#include <atf-c.h>

#include "diskimage.h"
#include "vmdkextentdescriptor.h"

ATF_TC_WITHOUT_HEAD(vmdkextentdescriptor_new__correctly_reads_data);
ATF_TC_BODY(vmdkextentdescriptor_new__correctly_reads_data, tc)
{
    LDI_ERROR res;
    struct vmdkextentdescriptor *extentdescriptor;

    res = vmdkextentdescriptor_new("RW 44042240 SPARSE \"\" 1", &extentdescriptor);

    // Check the return value.
    ATF_CHECK_EQ(LDI_ERR_NOERROR, res.code);

    // Check the members of extentdescriptor.
    ATF_CHECK_EQ(VMDK_ACCESS_RW, extentdescriptor->access);
    ATF_CHECK_EQ(44042240, extentdescriptor->sectors);
    ATF_CHECK_EQ(VMDK_EXTENT_SPARSE, extentdescriptor->type);
    ATF_CHECK_STREQ("", extentdescriptor->filename);
    ATF_CHECK_EQ(1, extentdescriptor->offset);

    vmdkextentdescriptor_destroy(&extentdescriptor);
}

ATF_TC_WITHOUT_HEAD(vmdkextentdescriptor_new__does_not_read_invalid_data);
ATF_TC_BODY(vmdkextentdescriptor_new__does_not_read_invalid_data, tc)
{
    struct vmdkextentdescriptor *extentdescriptor;
    LDI_ERROR res;

    res = vmdkextentdescriptor_new("RW 4404X2240 SPARSE \"\" 1", &extentdescriptor);

    // Check the return value.
    ATF_CHECK_EQ(LDI_ERR_PARSEERROR, res.code);

    ATF_CHECK_EQ(NULL, extentdescriptor);
}

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, vmdkextentdescriptor_new__correctly_reads_data);
    ATF_TP_ADD_TC(tp, vmdkextentdescriptor_new__does_not_read_invalid_data);
    return 0;
}

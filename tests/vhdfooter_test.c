
#include <atf-c.h>

#include <string.h>

#include "vhdfooter.h"
#include "diskimage.h"

void empty_log_write(int level, void *privarg, char *fmt, ...) { }

char valid_footer[512] = {
    0x63, 0x6F, 0x6E, 0x65, 0x63, 0x74, 0x69, 0x78,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x1C, 0x27, 0xFE, 0x22, 0x76, 0x62, 0x6F, 0x78,
    0x00, 0x04, 0x00, 0x03, 0x57, 0x69, 0x32, 0x6B,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00,
    0x01, 0x2D, 0x04, 0x11, 0x00, 0x00, 0x00, 0x02,
    0xFF, 0xFF, 0xE7, 0xC2, 0x35, 0x56, 0xC9, 0x1E,
    0x50, 0x11, 0x9D, 0x4D, 0x84, 0x11, 0xE9, 0x5E,
    0xCA, 0xE3, 0x5F, 0x35,
};

char invalid_footer[512] = {
    0x63, 0x6F, 0x6E, 0x65, 0x63, 0x74, 0x69, 0x78,
    0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x1C, 0x27, 0xFE, 0x22, 0x76, 0x62, 0x6F, 0x78,
    0x00, 0x04, 0x00, 0x03, 0x57, 0x69, 0x32, 0x6B,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00,
    0x01, 0x2D, 0x04, 0x11, 0x00, 0x00, 0x00, 0x02,
    0xFF, 0xFF, 0xFF, 0xFF, 0x35, 0x56, 0xC9, 0x1E,
    0x50, 0x11, 0x9D, 0x4D, 0x84, 0x11, 0xE9, 0x5E,
    0xCA, 0xE3, 0x5F, 0x35,
};

struct logger empty_logger = {
    .write = empty_log_write
};

ATF_TC_WITHOUT_HEAD(vhd_footer_new__correctly_reads_valid_data);
ATF_TC_BODY(vhd_footer_new__correctly_reads_valid_data, tc)
{
    struct vhd_footer *footer;

    vhd_footer_new(valid_footer, &footer, empty_logger);

    /* We expect the checksum to be valid for the footer. */
    ATF_CHECK(vhd_footer_isvalid(footer));
    /* This is a fixed size 10M disk. */
    ATF_CHECK_EQ(DISK_TYPE_FIXED, vhd_footer_disk_type(footer));
    ATF_CHECK_EQ(10*1024*1024, vhd_footer_disksize(footer));

    /* The offset is -1 for fixed disks. */
    ATF_CHECK_EQ(-1, vhd_footer_offset(footer));

    vhd_footer_destroy(&footer);
}

ATF_TC_WITHOUT_HEAD(vhd_footer_new__correctly_reads_invalid_data);
ATF_TC_BODY(vhd_footer_new__correctly_reads_invalid_data, tc)
{
    struct vhd_footer *footer;

    vhd_footer_new(invalid_footer, &footer, empty_logger);

    /* We expect the checksum to be invalid for the footer. */
    ATF_CHECK(!vhd_footer_isvalid(footer));

    vhd_footer_destroy(&footer);
}

ATF_TC_WITHOUT_HEAD(vhd_footer_write__correctly_writes_data);
ATF_TC_BODY(vhd_footer_write__correctly_writes_data, tc)
{
    struct vhd_footer *footer;
    char output[512];

    vhd_footer_new(valid_footer, &footer, empty_logger);

    vhd_footer_write(footer, output);

    /* Check that the output matches the input. */
    ATF_CHECK(memcmp(valid_footer, output, 512));

    vhd_footer_destroy(&footer);
}

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, vhd_footer_new__correctly_reads_valid_data);
    ATF_TP_ADD_TC(tp, vhd_footer_new__correctly_reads_invalid_data);
    ATF_TP_ADD_TC(tp, vhd_footer_write__correctly_writes_data);
    return 0;
}

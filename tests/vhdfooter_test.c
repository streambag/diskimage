
#include <atf-c.h>
#include <stdio.h>
#include <uuid.h>

#include <string.h>

#include "memorytest.h"

#define TESTSEAM(functionname) functionname ## _fake

void uuid_to_string_fake(const uuid_t *uuid, char **str, uint32_t *status);

/* Include the source file to test. */
#include "vhdfooter.c"

/* The following are dependencies of vhdfooter that we don't want to stub. */
#include "vhdserialization.c"
#include "vhdchecksum.c"


char valid_footer[512] = {
    0x63, 0x6F, 0x6E, 0x65, 0x63, 0x74, 0x69, 0x78, /* Cookie */
    0x00, 0x00, 0x00, 0x02,			    /* Features */
    0x00, 0x01, 0x00, 0x00,			    /* Version */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Data offset */
    0x1C, 0x27, 0xFE, 0x22,			    /* Time stamp */
    0x76, 0x62, 0x6F, 0x78,			    /* Creator application */
    0x00, 0x04, 0x00, 0x03,			    /* Creator version */
    0x57, 0x69, 0x32, 0x6B,			    /* Creator host os */
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, /* Original size */
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, /* Current size */
    0x01, 0x2D, 0x04, 0x11,			    /* Disk geometry */
    0x00, 0x00, 0x00, 0x02,			    /* Disk type */
    0xFF, 0xFF, 0xE7, 0xC2,			    /* Checksum */
    0x35, 0x56, 0xC9, 0x1E, 0x50, 0x11, 0x9D, 0x4D, /* UUID */
    0x84, 0x11, 0xE9, 0x5E, 0xCA, 0xE3, 0x5F, 0x35,
    0x00					    /* Saved state */
};

char invalid_footer[512] = {
    0x63, 0x6F, 0x6E, 0x65, 0x63, 0x74, 0x69, 0x78, /* Cookie */
    0x00, 0x00, 0x00, 0x02,                         /* Features */
    0x00, 0x01, 0x00, 0x00,                         /* Version */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Data offset */
    0x1C, 0x27, 0xFE, 0x22,                         /* Time stamp */
    0x76, 0x62, 0x6F, 0x78,                         /* Creator application */
    0x00, 0x04, 0x00, 0x03,                         /* Creator version */
    0x57, 0x69, 0x32, 0x6B,                         /* Creator host os */
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, /* Original size */
    0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, /* Current size */
    0x01, 0x2D, 0x04, 0x11,                         /* Disk geometry */
    0x00, 0x00, 0x00, 0x02,                         /* Disk type */
    0xFF, 0xFF, 0xFF, 0xFF,                         /* Checksum */
    0x35, 0x56, 0xC9, 0x1E, 0x50, 0x11, 0x9D, 0x4D, /* UUID */
    0x84, 0x11, 0xE9, 0x5E,                                                   
    0xCA, 0xE3, 0x5F, 0x35,                         /* Saved state */
    0x00
};

void empty_log_write(int level, void *privarg, char *fmt, ...) { }

struct logger empty_logger = {
    .write = empty_log_write
};

/*
 * A test implementation of uuid_to_string which keeps track of the allocated
 * memory and can simulate allocation failure.
 */
void uuid_to_string_fake(const uuid_t *uuid, char **str, uint32_t *status)
{
	if (should_simulate_allocation_failure()) {
		*status = uuid_s_no_memory;
		return;
	}

	uuid_to_string(uuid, str, status);
	register_allocation(*str);
}

uint32_t
checksum_array(uint8_t *source, uint32_t count)
{
	int i;
	uint32_t result = 0;

	for (i = 0; i < count; i++) {
		result += *(source + i);
	}

	return ~result;
}

ATF_TC_WITHOUT_HEAD(vhdfooter_new__correctly_reads_valid_data);
ATF_TC_BODY(vhdfooter_new__correctly_reads_valid_data, tc)
{
    struct vhdfooter *footer;
    reset_memory();

    vhdfooter_new(valid_footer, &footer, empty_logger);

    /* We expect the checksum to be valid for the footer. */
    ATF_CHECK_EQ(VHDFOOTER_OK, vhdfooter_getstatus(footer));
    /* This is a fixed size 10M disk. */
    ATF_CHECK_EQ(DISK_TYPE_FIXED, vhdfooter_getdisktype(footer));
    ATF_CHECK_EQ(10*1024*1024, vhdfooter_disksize(footer));

    /* The offset is -1 for fixed disks. */
    ATF_CHECK_EQ(-1, vhdfooter_offset(footer));

    vhdfooter_destroy(&footer);

    /* Check that all allocated memory has been freed. */
    check_memory();
}

ATF_TC_WITHOUT_HEAD(vhdfooter_new__correctly_reads_invalid_data);
ATF_TC_BODY(vhdfooter_new__correctly_reads_invalid_data, tc)
{
    struct vhdfooter *footer;
    reset_memory();

    vhdfooter_new(invalid_footer, &footer, empty_logger);

    /* We expect the checksum to be invalid for the footer. */
    ATF_CHECK_EQ(VHDFOOTER_BADCHECKSUM, vhdfooter_getstatus(footer));

    vhdfooter_destroy(&footer);

    /* Check that all allocated memory has been freed. */
    check_memory();
}

ATF_TC_WITHOUT_HEAD(vhdfooter_write__correctly_writes_data);
ATF_TC_BODY(vhdfooter_write__correctly_writes_data, tc)
{
    struct vhdfooter *footer;
    char output[512];
    reset_memory();

    vhdfooter_new(valid_footer, &footer, empty_logger);

    vhdfooter_write(footer, output);

    /* Check that the output matches the input. */
    ATF_CHECK(memcmp(valid_footer, output, 512));

    vhdfooter_destroy(&footer);

    /* Check that all allocated memory has been freed. */
    check_memory();
}

ATF_TC_WITHOUT_HEAD(vhdfooter_new__returns_error_on_allocation_failure);
ATF_TC_BODY(vhdfooter_new__returns_error_on_allocation_failure, tc)
{
	struct vhdfooter *footer;
	LDI_ERROR res;
	int allocations, i;
	/* Count the number of allocations */
	reset_memory();
	vhdfooter_new(valid_footer, &footer, empty_logger);
	allocations = get_allocation_count();
	vhdfooter_destroy(&footer);

	for (i = 0; i < allocations; i++) {
		/* Simulate that the i+1:th allocation fails. */
		reset_memory();
		simulate_allocation_failure(i);

		res = vhdfooter_new(valid_footer, &footer, empty_logger);
		/* Check that the return value is set correctly and that the
		 * pointer is set to null. */
		ATF_CHECK_EQ(LDI_ERR_NOMEM, res.code);
		ATF_CHECK_EQ(NULL, footer);

		/* Check that all allocated memory has been freed. */
		check_memory();
	}
}

ATF_TC_WITHOUT_HEAD(vhdfooter_getstatus__handles_invalid_cookie);
ATF_TC_BODY(vhdfooter_getstatus__handles_invalid_cookie, tc)
{
    uint8_t invalid_cookie[512] = {
        [0] = 0x63, [1] = 0x6F, [2] = 0x6E, [3] = 0x64,    /* Cookie */
	[4] = 0x63, [5] = 0x74, [6] = 0x69, [7] = 0x78,	    
    };

    uint32_t checksum = checksum_array(invalid_cookie, 512);
    invalid_cookie[67] = checksum & 0xFF;
    invalid_cookie[66] = (checksum >> 8) & 0xFF;
    invalid_cookie[65] = (checksum >> 16) & 0xFF;
    invalid_cookie[64] = (checksum >> 24) & 0xFF;

    struct vhdfooter *footer;
    reset_memory();

    vhdfooter_new(invalid_cookie, &footer, empty_logger);

    ATF_CHECK_EQ(VHDFOOTER_BADCOOKIE, vhdfooter_getstatus(footer));

    vhdfooter_destroy(&footer);

    /* Check that all allocated memory has been freed. */
    check_memory();
}

ATF_TP_ADD_TCS(tp)
{
    ATF_TP_ADD_TC(tp, vhdfooter_new__correctly_reads_valid_data);
    ATF_TP_ADD_TC(tp, vhdfooter_new__correctly_reads_invalid_data);
    ATF_TP_ADD_TC(tp, vhdfooter_write__correctly_writes_data);
    ATF_TP_ADD_TC(tp, vhdfooter_new__returns_error_on_allocation_failure);
    ATF_TP_ADD_TC(tp, vhdfooter_getstatus__handles_invalid_cookie);
    return 0;
}

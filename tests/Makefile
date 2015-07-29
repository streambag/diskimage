
.include <bsd.own.mk>

CSTD?=	c99

TESTDIR=	${TESTBASE}/lib/diskimage/

CFLAGS+=	-I ../libdiskimage/
CFLAGS+= -g

DPADD=	${LIBGEOM} ${LIBUTIL} ${LIBDISKIMAGE}
LDADD=	-lgeom -lutil

ATF_TESTS_C=	vhdfooter_test vmdkdescriptorfile_test vmdkextentdescriptor_test

CLEANFILES+= ${ATF_TESTS_C} ${ATF_TESTS_C:T:S/$/.o/}

.include <bsd.test.mk>

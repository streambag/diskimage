
.include <bsd.own.mk>

TESTDIR=	${TESTBASE}/lib/diskimage/

CFLAGS+=	-I ../libdiskimage/
CFLAGS+= -L ../libdiskimage
CFLAGS+= -g

DPADD=	${LIBGEOM} ${LIBUTIL} ${LIBDISKIMAGE}
LDADD=	-lgeom -lutil -ldiskimage

ATF_TESTS_C=	vhdfooter_test

.include <bsd.test.mk>

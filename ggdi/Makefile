# $FreeBSD$

.PATH: /usr/src/sbin/ggate/shared

PROG=   ggdi
CSTD?=	c99
MAN=	ggdi.8
SRCS=	ggdi.c ggate.c

CFLAGS+= -DLIBGEOM
CFLAGS+= -Wall
CFLAGS+= -I /usr/src/sbin/ggate/shared
CFLAGS+= -I /usr/local/include
CFLAGS+= -I ../libdiskimage
CFLAGS+= -L /usr/local/lib
CFLAGS+= -L ../libdiskimage

CFLAGS+= -g

DPADD=	${LIBGEOM} ${LIBUTIL} ${LIBDISKIMAGE}
LDADD=	-lgeom -lutil -ldiskimage

.include <bsd.prog.mk>

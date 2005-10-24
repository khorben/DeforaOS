PACKAGE	= utils
VERSION	= 0.0.0
SUBDIRS	= src
TAR	= tar cfzv


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd $$i && $(MAKE) distclean) || exit; done

dist: distclean
	@$(TAR) $(PACKAGE)-$(VERSION).tar.gz \
		src/basename.c \
		src/cat.c \
		src/chgrp.c \
		src/chmod.c \
		src/chown.c \
		src/cksum.c \
		src/cmp.c \
		src/cp.c \
		src/dirname.c \
		src/du.c \
		src/echo.c \
		src/false.c \
		src/file.c \
		src/head.c \
		src/id.c \
		src/kill.c \
		src/link.c \
		src/ln.c \
		src/locale.c \
		src/logname.c \
		src/ls.c \
		src/mkdir.c \
		src/mkfifo.c \
		src/nice.c \
		src/pwd.c \
		src/renice.c \
		src/rm.c \
		src/rmdir.c \
		src/sleep.c \
		src/strings.c \
		src/test.c \
		src/time.c \
		src/touch.c \
		src/true.c \
		src/tty.c \
		src/uname.c \
		src/uniq.c \
		src/unlink.c \
		src/wc.c \
		src/common.c \
		src/project.conf \
		src/Makefile \
		project.conf \
		Makefile

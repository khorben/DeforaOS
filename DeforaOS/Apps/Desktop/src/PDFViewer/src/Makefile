TARGETS	= pdfviewer
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
CC	?= cc
CPPFLAGSF= -I $(PREFIX)/include
CPPFLAGS?=
CFLAGSF	= -W `pkg-config --cflags libDesktop poppler-glib`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libDesktop poppler-glib` -lm
LDFLAGS	= 
RM	?= rm -f
LN	?= ln -f
MKDIR	?= mkdir -p
INSTALL	?= install


all: $(TARGETS)

pdfviewer_OBJS = callbacks.o pdfviewer.o main.o
pdfviewer_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
pdfviewer_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

pdfviewer: $(pdfviewer_OBJS)
	$(CC) -o pdfviewer $(pdfviewer_OBJS) $(pdfviewer_LDFLAGS)

callbacks.o: callbacks.c callbacks.h
	$(CC) $(pdfviewer_CFLAGS) -c callbacks.c

pdfviewer.o: pdfviewer.c callbacks.h pdfviewer.h ../config.h
	$(CC) $(pdfviewer_CFLAGS) -c pdfviewer.c

main.o: main.c
	$(CC) $(pdfviewer_CFLAGS) -c main.c

clean:
	$(RM) -- $(pdfviewer_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 -- pdfviewer $(DESTDIR)$(BINDIR)/pdfviewer

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/pdfviewer

.PHONY: all clean distclean install uninstall

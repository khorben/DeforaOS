SUBDIRS	= FSProxy


all clean distclean:
	@for i in $(SUBDIRS); do make -C $$i $@ || exit $$?; done

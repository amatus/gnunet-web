all: libextractor.so

libextractor.so: extractor.c extractor_metatypes.c extractor.h
	$(CC) -I. -shared -o $@ extractor.c extractor_metatypes.c

install:
	install -d $(DESTDIR)/include
	install extractor.h $(DESTDIR)/include
	install -d $(DESTDIR)/lib
	install libextractor.so $(DESTDIR)/lib

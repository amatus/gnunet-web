all: libextractor.so

libextractor.so: extractor.c extractor_metatypes.c extractor.h
	$(CC) -I. -shared -o $@ extractor.c extractor_metatypes.c

install: all
	install -d $(DESTDIR)/usr/include
	install extractor.h $(DESTDIR)/usr/include
	install -d $(DESTDIR)/usr/lib
	install libextractor.so $(DESTDIR)/usr/lib

LIBTOOL=libtool
MODPATH=/usr/local/lib
MODULES=ANSI_Z39.47.so

all:	$(MODULES)

%.so:	%.lo
	$(LIBTOOL) $(CC) -module -avoid-version -o $*.la $^ -rpath $(MODPATH)
	mv .libs/$@ $@

%.lo:	%.c
	$(LIBTOOL) $(CC) -c $^

clean:
	rm -rf .libs
	rm -f *.o *.lo *.la *.so

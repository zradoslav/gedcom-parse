# $Id$
# $Name$

LIBTOOL=libtool
MODPATH=/usr/local/lib
MODULES=ANSI_Z39.47.so
CFLAGS=-g

all:	$(MODULES)

%.so:	%.lo
	$(LIBTOOL) $(CC) -module -avoid-version $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $*.la -rpath $(MODPATH)
	mv .libs/$@ $@

%.lo:	%.c
	$(LIBTOOL) $(CC) -c $(CPPFLAGS) $(CFLAGS) $^

clean:
	rm -rf .libs
	rm -f *.o *.lo *.la *.so

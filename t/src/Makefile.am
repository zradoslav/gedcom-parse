## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" -I$(srcdir)/../../include \
           -I$(srcdir)/../../utf8 -I$(srcdir)/../../intl
CFLAGS   = -O2 @EXTRA_CFLAGS@

noinst_PROGRAMS = testgedcom pathtest gomtest updatetest testintl \
                  updategomtest writegomtest
noinst_HEADERS = output.h dump_gom.h portability.h

testgedcom_SOURCES = standalone.c output.c portability.c
testgedcom_LDFLAGS = -L../../gedcom/.libs -L../../utf8/.libs @ICONV_LIBPATH@
testgedcom_LDADD  = $(LIBICONV) -lgedcom -lutf8tools $(LIBICONV)

pathtest_SOURCES = pathtest.c
pathtest_LDFLAGS = -L../../gedcom/.libs -L../../utf8/.libs @ICONV_LIBPATH@
pathtest_LDADD = $(LIBICONV) -lgedcom -lutf8tools $(LIBICONV)

gomtest_SOURCES = gomtest.c output.c dump_gom.c portability.c
gomtest_LDFLAGS = -L../../gedcom/.libs -L../../gom/.libs -L../../utf8/.libs \
		  @ICONV_LIBPATH@
gomtest_LDADD = $(LIBICONV) -lgedcom_gom -lgedcom -lutf8tools $(LIBICONV)

updatetest_SOURCES = update.c output.c portability.c
updatetest_LDFLAGS = -L../../gedcom/.libs -L../../utf8/.libs @ICONV_LIBPATH@
updatetest_LDADD = $(LIBICONV) -lgedcom -lutf8tools $(LIBICONV)

updategomtest_SOURCES = update_gom.c output.c dump_gom.c portability.c
updategomtest_LDFLAGS = -L../../gedcom/.libs -L../../gom/.libs \
                        -L../../utf8/.libs @ICONV_LIBPATH@
updategomtest_LDADD = $(LIBICONV) -lgedcom_gom -lgedcom -lutf8tools $(LIBICONV)

writegomtest_SOURCES = gom_write.c output.c portability.c
writegomtest_LDFLAGS = -L../../gedcom/.libs -L../../gom/.libs \
                       -L../../utf8/.libs @ICONV_LIBPATH@
writegomtest_LDADD = $(LIBICONV) -lgedcom_gom -lgedcom -lutf8tools $(LIBICONV)

testintl_SOURCES = testintl.c output.c
testintl_LDFLAGS = -L../../gedcom/.libs -L../../utf8/.libs @ICONV_LIBPATH@
testintl_LDADD = $(LIBICONV) -lgedcom -lutf8tools @INTLLIBS@ $(LIBICONV)

TEST_SCRIPT=test_script test_gom test_update test_intl test_updategom test_writegom test_prologue.sh test_bulk.sh

EXTRA_DIST=$(TEST_SCRIPT)

## Process this file with automake to produce Makefile.in
# $Id$
# $Name$

INCLUDES = -DPKGDATADIR=\"$(pkgdatadir)\" -I $(srcdir)/../../include \
           -I $(srcdir)/../../utf8 -I $(srcdir)/../intl
CFLAGS   = -O2 @EXTRA_CFLAGS@

noinst_PROGRAMS = testgedcom pathtest gomtest updatetest testintl \
                  updategomtest writegomtest
noinst_HEADERS = output.h dump_gom.h portability.h

testgedcom_SOURCES = standalone.c output.c portability.c
testgedcom_LDFLAGS = -L../../gedcom/.libs -L../../utf8/.libs
testgedcom_LDADD  = -lgedcom -lutf8tools

pathtest_SOURCES = pathtest.c
pathtest_LDFLAGS = -L../../gedcom/.libs -L../../utf8/.libs
pathtest_LDADD = -lgedcom -lutf8tools

gomtest_SOURCES = gomtest.c output.c dump_gom.c portability.c
gomtest_LDFLAGS = -L../../gedcom/.libs -L../../gom/.libs -L../../utf8/.libs
gomtest_LDADD = -lgedcom_gom -lgedcom -lutf8tools

updatetest_SOURCES = update.c output.c portability.c
updatetest_LDFLAGS = -L../../gedcom/.libs -L../../utf8/.libs
updatetest_LDADD = -lgedcom -lutf8tools

updategomtest_SOURCES = update_gom.c output.c dump_gom.c portability.c
updategomtest_LDFLAGS = -L../../gedcom/.libs -L../../gom/.libs \
                        -L../../utf8/.libs
updategomtest_LDADD = -lgedcom_gom -lgedcom -lutf8tools

writegomtest_SOURCES = gom_write.c output.c portability.c
writegomtest_LDFLAGS = -L../../gedcom/.libs -L../../gom/.libs \
                       -L../../utf8/.libs
writegomtest_LDADD = -lgedcom_gom -lgedcom -lutf8tools

testintl_SOURCES = testintl.c output.c
testintl_LDFLAGS = -L../../gedcom/.libs -L../../utf8/.libs
testintl_LDADD = -lgedcom -lutf8tools @INTLLIBS@

TEST_SCRIPT=test_script test_gom test_update test_intl test_updategom test_writegom test_prologue.sh test_bulk.sh

EXTRA_DIST=$(TEST_SCRIPT)

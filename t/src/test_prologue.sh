# $Id$
# $Name$
# Common prologue for the test scripts

extra_options=

while [ $# -gt 0 ]
do
  case "$1" in
    -*) extra_options="$extra_options $1";;
    *)  break;;
  esac
  shift
done

test_name=`echo $1 | sed "s:.*/::" | sed "s:.test::"`
shift
expected_result=$1
shift
if [ -z "$test_name" ]
then
  test_name=check
fi
if [ -z "$expected_result" ]
then
  expected_result=0
fi
if [ -z "$GEDCOM_LANG" ]
then
  GEDCOM_LANG=C
fi

# For use outside Makefile
if [ -z "$srcdir" ]
then
  :
else
  options="-q $extra_options"
fi

outfile=$test_name.out
logfile=check.out
reffile=$srcdir/output/$test_name.ref
options="$options -o $outfile"

if [ "$gedcom_out" ]
then
  gedfile=$test_name.ged
  gedreffile=$srcdir/output/$test_name.ged
  options="$options -w $gedfile"
fi

GCONV_PATH=.:$GCONV_PATH
export GCONV_PATH
LC_ALL=$GEDCOM_LANG
export LC_ALL
ln -s $srcdir/../data/gedcom.enc .
ln -s $builddir/../data/new.ged .
ln -s $builddir/../iconv/glibc/.libs/ANSI_Z39.47.so .
ln -s $srcdir/../iconv/glibc/gconv-modules .
rm -f core

# $Id$
# $Name$
# Common test part for the test scripts

ltcmd="$builddir/../libtool --mode=execute"
for lib in $test_libs
do
  ltcmd="$ltcmd -dlopen $lib"
done

echo "======================================================">> $logfile
echo "Performing test '$test_name'" >> $logfile
$ltcmd $GEDCOM_TESTENV $builddir/src/$test_program $options $test_args
result=$?
rm gedcom.enc
rm new.ged
rm ANSI_Z39.47.so
rm gconv-modules

echo "Result is $result (expected: $expected_result)" >> $logfile
if [ "$result" -ne "$expected_result" ]
then
  echo "Not the expected return value!" >> $logfile
  exit 1
else
  if [ -r core ]
  then
    echo "Core file generated!" >> $logfile
    exit 1
  else
    if diff $outfile $reffile >/dev/null 2>>$logfile
    then
      echo "Output agrees with reference output" >> $logfile
      rm $outfile
      if [ "$gedcom_out" ]
      then
        if diff $gedfile $gedreffile >/dev/null 2>>$logfile
	then
	  echo "Gedcom output agrees with reference output" >> $logfile
	  rm $gedfile
	  exit 0
	else
	  echo "Differences with reference gedcom output detected!" >> $logfile
	  exit 1
	fi
      fi
      exit 0
    else
      echo "Differences with reference output detected!" >> $logfile
      exit 1
    fi
  fi
fi

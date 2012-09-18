#!/bin/sh

#Copy executables to current directory
cp ../../../../src/components/cpp/Playback/Windows/Playback.exe .
cp ../../../../src/components/cpp/Archiver/Default/Archiver.exe .
cp ../../../../src/components/cpp/ServiceDirectory/Default/ServiceDirectory.exe .

#Create the database
mysql test -uroot -pesmf < PlaybackDB.sql

#Run the Processes
ServiceDirectory &
SDPID=$!
Playback &
PLPID=$!
Archiver &
ARPID=$!

#Wait for the playback to finish.  
wait $PLPID
#Kill other processes.  
kill $ARPID
kill $SDPID

#Get the results
echo "Select HEX(message), DataproductID from test;" > tmpsql321.sql
mysql test -uroot -pesmf < tmpsql321.sql > tmpoutsql321.txt
rm tmpsql321.sql

#Compare the data to what was expected.  
if diff expected_sql_output.txt tmpoutsql321.txt ; then 
	echo Test Passed
else
	echo Test Failed
	exit -1
fi
rm tmpoutsql321.txt

exit 0

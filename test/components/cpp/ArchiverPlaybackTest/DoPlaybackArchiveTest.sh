#!/bin/sh
#** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
#**
#** Gravity is free software; you can redistribute it and/or modify
#** it under the terms of the GNU Lesser General Public License as published by
#** the Free Software Foundation; either version 3 of the License, or
#** (at your option) any later version.
#**
#** This program is distributed in the hope that it will be useful,
#** but WITHOUT ANY WARRANTY; without even the implied warranty of
#** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#** GNU Lesser General Public License for more details.
#**
#** You should have received a copy of the GNU Lesser General Public 
#** License along with this program;
#** If not, see <http://www.gnu.org/licenses/>.
#**



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
echo "Select HEX(message), DataproductID from archivetest;" > tmpsql321.sql
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

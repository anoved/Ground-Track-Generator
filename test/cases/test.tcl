#!/usr/bin/env tclsh
package require Tcl 8.5

# test number, start time, end time, and interval
# based on Appendix D - Test Case Listing - of "Revisiting Spacetrack Report 3"
set tests {
	{01 epoch epoch+4320m 360m}
	{02 epoch-5184m epoch-4896m 120m}
	{03 epoch epoch+2880m 120m}
	{04 epoch epoch+2880m 120m}
	{05 epoch epoch+2880m 120m}
	{06 epoch-1440m epoch-720m 60m}
	{07 epoch epoch+1440m 360m}
	{08 epoch epoch+2880m 120m}
	{09 epoch epoch+1440m 120m}
	{10 epoch+1440m epoch+4320m 120m}
	{11 epoch epoch+2880m 120m}
	{12 epoch+54.2028672m epoch+1440m 20m}
	{13 epoch epoch+2880m 120m}
	{14 epoch epoch+1440m 120m}
	{15 epoch epoch+1600m 120m}
	{16 epoch epoch+720m 20m}
	{17 epoch epoch+1440m 120m}
	{18 epoch-1440m epoch+1440m 120m}
	{19 epoch+9300m epoch+9400m 60m}
	{20 epoch epoch+2880m 120m}
	{21 epoch epoch+2880m 120m}
	{22 epoch epoch+1440m 120m}
	{23 epoch epoch+2880m 120m}
	{24 epoch epoch+1440m 120m}
	{25 epoch epoch+1440m 120m}
	{26 epoch epoch+60m 5m}
	{27 epoch epoch+440m 20m}
	{28 epoch epoch+1440m 120m}
	{29 epoch epoch+1440m 120m}
	{30 epoch epoch+150m 5m}
	{31 epoch epoch+1440m 1m}
	{32 epoch epoch+1440m 20m}
	{33 epoch+1844000m epoch+1845100m 5m}
}

# dumpfilePath - path to a text file created by dbfdump
# casefilePath - path for reformatted version of dumpfile
# testfile - file handle for cumulative test log
proc reformatresults {dumpfilePath casefilePath testfile} {
	
	# read contents of attribute table dump
	if {[catch {open $dumpfilePath} dumpfile]} {
		error $dumpfile
	}
	set fc [read -nonewline $dumpfile]
	close $dumpfile
	
	# convert file contents to list of lines, ignoring header line
	set lines [lreplace [split $fc \n] 0 0]
	
	
	if {[catch {open $casefilePath w} casefile]} {
		error $casefile
	}
	
	# rewrite the dumpfile contents to the casefile & cumulative testfile,
	foreach line $lines {
		foreach {fid date time utc mfe xpos ypos zpos xvel yvel zvel} $line {}
		set rec [list $mfe $xpos $ypos $zpos $xvel $yvel $zvel]
		puts $casefile $rec
		puts $testfile $rec
	}
	
	puts $testfile ""
	close $casefile
}

if {[catch {open test-gtg.txt w} testlog]} {
	puts stderr $testlog
	exit 1
}

set testdir [pwd]

foreach test $tests  {
	
	cd $testdir
	
	foreach {id start end interval} $test {}
	
	cd $id
	
	# clean up previous output
	if {[catch {exec rm -f $id.shp $id.shx $id.dbf $id.log $id.dbf.txt $id.txt} err]} {
		puts stderr "$id - cleanup error: $err"
	}
	
	# run this test case with gtg
	# status messages logged to .log file
	if {[catch {exec ../../../gtg \
		--input $id.tle \
		--output $id \
		--attributes mfe time xposition yposition zposition xvelocity yvelocity zvelocity \
		--start $start \
		--end $end \
		--forceend \
		--interval $interval \
		--verbose > $id.log} err]} {
		puts stderr "$id - gtg error: $err"
	}
	
	# convert the output .dbf attribute table to .dbf.txt text
	# dbfdump is a shapelib utility
	if {[catch {exec /usr/local/bin/dbfdump $id.dbf > $id.dbf.txt} err]} {
		puts stderr "$id - dbfdump error: error"
	}
	
	# tidy up the .dbf.txt attribute table text dump to match plain .txt key format
	# appends reformatted text table to cumulative test-gtg.txt as well
	puts $testlog $id
	if {[catch {reformatresults $id.dbf.txt $id.txt $testlog} err]} {
		puts stderr "$id - reformat error: $err"
	}
	
}

close $testlog
exit 0

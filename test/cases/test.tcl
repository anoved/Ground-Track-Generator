#!/usr/bin/env tclsh
package require Tcl 8.5

set testdir [pwd]

proc reformatresults {inpath outpath} {
	
	# read inpath file contents
	set f [open $inpath]
	set fc [read -nonewline $f]
	close $f
	
	# convert file contents to list of lines
	set lines [split $fc \n]
	
	# skip header line
	set lines [lreplace $lines 0 0]
	
	
	set o [open $outpath w]
	foreach line $lines {
		foreach {fid date time utc mfe xpos ypos zpos xvel yvel zvel} $line {}
		puts $o [list $mfe $xpos $ypos $zpos $xvel $yvel $zvel]
	}
	
	close $o
}

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

foreach test $tests  {
	
	cd $testdir
	
	foreach {id start end interval} $test {}
	
	cd $id
	
	if {[catch {exec ../../../gtg \
		--input $id.tle \
		--output $id \
		--attributes time mfe xposition yposition zposition xvelocity yvelocity zvelocity \
		--start $start \
		--end $end \
		--forceend \
		--interval $interval \
		--verbose > $id.log} err]} {
		puts "$id: $err"
	}
	
	if {[catch {exec /usr/local/bin/dbfdump $id.dbf > $id.dbf.txt} err]} {
		puts "$id: error"
	}
	
	if {[catch {reformatresults $id.dbf.txt $id.txt} err]} {
		puts "$id: error"
	}
	
}

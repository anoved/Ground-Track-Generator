#!/usr/bin/env tclsh

if {$argc != 2} {
	puts stderr "usage: difftable.tcl afile bfile"
	puts stderr "       difftable.tcl test-library.txt test-reference.txt"
	exit 1
}

if {[catch {open [lindex $argv 0]} afile]} {
	puts stderr $afile
	exit 1
}
set alines [split [read -nonewline $afile] "\n"]
close $afile

if {[catch {open [lindex $argv 1]} bfile]} {
	puts stderr $bfile
	exit 1
}
set blines [split [read -nonewline $bfile] "\n"]
close $bfile

foreach aline $alines bline $blines {

	set alen [llength $aline]
	set blen [llength $bline]
		
	if {$alen == $blen} {
		if {$alen > 1} {
			
			set line [list]
			
			foreach av $aline bv $bline {
				
				# count digits after . to determine precision
				# assume precision of both fields is the same
				set prec [string length [lindex [split $av .] 1]]
			
				set diff [expr {$av - $bv}]
				lappend line [format "%.*f" $prec $diff]
			}
		
			puts $line
		
		} else {
			# matching line size with just one field
			# assume to be the test case number
			puts $aline
		}
	} else {
		# mismatched line size; just print line from file a
		puts $aline
	}

}

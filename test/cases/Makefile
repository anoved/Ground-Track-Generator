# GTG/test/cases Makefile

# Tools

TCLSH = /usr/bin/tclsh
DIFF = /usr/bin/diff

# Scripts

TEST_SCRIPT = test.tcl

# Files

TESTLOG_GTG = test-gtg.txt
TESTLOG_LIB = test-library.txt
TESTLOG_REF = test-reference.txt



test:
	@echo "Running tests..."
	@$(TCLSH) $(TEST_SCRIPT) $(TESTLOG_GTG)
	@$(DIFF) -q $(TESTLOG_GTG) $(TESTLOG_LIB)
	@echo "... completed tests OK."

# GTG/test/cases Makefile

# Tools

TCLSH = /usr/bin/tclsh
DIFF = /usr/bin/diff

DBFDUMP_DIR = ../../Libraries/shapelib
DBFDUMP = $(DBFDUMP_DIR)/dbfdump

# Scripts

TEST_SCRIPT = test.tcl

# Files

TESTLOG_GTG = test-gtg.txt
TESTLOG_LIB = test-library.txt
TESTLOG_REF = test-reference.txt

.PHONY: test

test: $(DBFDUMP)
	@echo "Running tests..."
	@$(TCLSH) $(TEST_SCRIPT) $(DBFDUMP) $(TESTLOG_GTG)
	@$(DIFF) -q $(TESTLOG_GTG) $(TESTLOG_LIB)
	@echo "... completed tests OK."

$(DBFDUMP):
	@$(MAKE) --directory=$(DBFDUMP_DIR) dbfdump


# convenience Makefile for invoking common commands during Linux 64-bit porting.
SHELL=/bin/bash

# sane setting of env variables and cwd; use one shell to process all commands
.ONESHELL:

STARTDIR := $(shell pwd)
STARTTM  := $(shell date | sed 's/[ -]/_/g')

all:
	make -f lii64gc.gmk

clean:
	 rm -rf lii64gc

test:
	cd lii64gc/ci; ./amcss

trace:
	echo "generating etrace function call log (requires gcc), starting from $PWD"
	cd ${STARTDIR}/lii64gc/ci; if [[ -f TRACE ]]; then mv TRACE TRACE.old.before.${STARTTM}; fi ; touch TRACE; 
	cd ${STARTDIR}/lii64gc/ci; if [[ -f log ]]; then mv log log.old.before.${STARTTM}; fi ; 
	cd ${STARTDIR}/lii64gc/ci; ( etrace.pl ./amcss TRACE > log ) &  # start server, reads over named pipe in this directory call tracing info.
	cd ${STARTDIR}/lii64gc/ci; ./amcss  # start client, assuming it was compiled with ptrace.c in the makefile. See etrace pkg for details.

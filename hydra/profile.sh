#!/bin/sh
valgrind --instr-atstart=no --sigill-diagnostics=yes --dump-instr=yes --collect-jumps=yes --tool=callgrind build/TKPEmu
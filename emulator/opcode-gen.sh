#!/bin/bash

##generate opcode table
##opcode, mnemonic, func, addr_mode, cycle, len
##
awk '
###ignore the line start with #
$0!~/#/ {
#second column addressing mode is indexed from 0.
    if ($2 == 0)
        print "{ 0x"$1 ", \"\", NULL, " $3 - 1", " $4", " $5 ", " $6 " }, "
    else
        print "{ 0x"$1 ", \"" $2 "\", func_" $2 ", " $3 - 1", " $4", " $5 ", " $6 " }, "
}
' < opcode-6502 > opcode


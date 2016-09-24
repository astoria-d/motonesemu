#!/bin/bash

cd bin
test -e vgadisp.exe && test ! -e vgadisp && cp vgadisp.exe vgadisp

export DISPLAY=:0.0
./vgadisp &


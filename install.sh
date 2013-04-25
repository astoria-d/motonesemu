#!/bin/bash

test ! -e bin && mkdir bin
cp emulator/motonesemu bin/
cp display/vgadisp bin/
cp emulator/joypad/famicon-controller.jpg bin/


#!/bin/bash
# This script launches the headless version of the application by creating fake display so that wxwidgets is happy.
export DISPLAY=:1.0
Xvfb :1 -screen 0 1024x768x16 &> xvfb.log  &
./build/MoloVol $@

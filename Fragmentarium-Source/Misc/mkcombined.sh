#!/bin/bash
# usage: mkcombined.sh <basename> <resolution>
#    eg: ./sh mkcombined.sh myimgfiles 256x256x256
# creates a file named myimgfiles-combined-256x256x256.exr that contains all exr files with basename

../bin/exrmultipart -combine -i `ls $1.*.exr` -o $1-combined-$2.exr

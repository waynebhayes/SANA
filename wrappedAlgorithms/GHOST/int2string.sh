#!/bin/sh
module unload java
module load java/1.7.0_71
exec java -jar $HOME/bin/int2string.jar "$@"


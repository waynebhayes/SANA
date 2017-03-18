#!/bin/sh
module unload java
module load java/1.7.0_71
exec java -jar `dirname $0`/int2string.jar "$@"

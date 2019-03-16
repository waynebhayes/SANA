#!/bin/sh
for dir in regression-tests/*; do
   for r in $dir/*.sh; do
      nice -19 "$r"
   done
done

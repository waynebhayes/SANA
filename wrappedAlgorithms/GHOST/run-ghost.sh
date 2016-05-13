#!/bin/bash

./gw2gexf.sh

for f in configs/*.cfg; do
	echo "Processing $f file.."
	filename=$(basename "$f")
	echo "report/$filename.log"
	./GHOST -c $f | tee "report/$filename.log"
done

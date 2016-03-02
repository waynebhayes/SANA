#!/bin/bash

rm -f gexf/*
find . gw/ -name "*.gw" -exec ./bio-graph --convert {} --output {}.gexf \;
mv gw/*.gexf gexf/

#!/bin/bash

cmake -S . -B build -DENABLE_CPPCHECK:BOOL=ON

#!/bin/bash
# A clean up file - in case fuzzer crashes
# used to clean files
chmod -R 755 code_coverage.txt coverage.txt /tmp/data/ 
rm -rf code_coverage.txt coverage.txt /tmp/data/*
rmdir /tmp/data/* /tmp/data/

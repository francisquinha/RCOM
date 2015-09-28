#!/bin/bash

echo "started compiling shit..."
gcc $1.c -o $1 -D_REENTRANT -lrt -lpthread -Wall
#!/bin/bash

echo "started compiling shit..."
gcc $1.c -o $1 -D_REENTRANT -lpthread -Wall
#!/bin/sh

GDB=gdb-multiarch

$GDB "$1" -ex 'target remote localhost:3333'

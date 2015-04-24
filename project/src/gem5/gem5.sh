#!/bin/sh

#--debug-start=20000000 \
#--debug-flags=MemoryAccess,Cache,DRAM \

./build/X86/gem5.opt \
  --debug-flags=Exec,MemoryAccess \
  --debug-file=trace.out \
  ./configs/example/se.py \
  --cpu-type=timing \
  --caches --l2cache \
  -c hello -o "$*"

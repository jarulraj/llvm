#!/bin/sh

GEM5_PATH="."

#--debug-start=20000000 \

    $GEM5_PATH/build/X86/gem5.opt \
    --debug-flags=Exec,MemoryAccess,Cache,DRAM \
    --debug-file=trace.out \
    $GEM5_PATH/configs/example/se.py \
    --cpu-type=timing \
    --caches --l2cache \
    -c sample -o "$*"

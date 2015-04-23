#!/bin/sh

opt -load ./LoopMemoryAnalysis.so --loop-memory-analysis  tests/simple.bc -o out
#opt -load ./LoopMemoryAnalysis.so --loop-memory-analysis  tests/anuj_simple.bc -o out
